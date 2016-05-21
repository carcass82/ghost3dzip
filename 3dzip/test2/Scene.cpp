#include <boost/smart_ptr.hpp>
#include <boost/ref.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cfloat>
#include <GL/glew.h>
#include <QFutureSynchronizer>
#include <QtConcurrentRun>
#include <QTime>
#include "Scene.hpp"
#include "3dzip/Writer.hh"
#include "3dzip/Reader.hh"

Scene::Scene(const std::string& filename)
{
    build_3d(filename);
    QTime t;
    t.start();
    int original_cum = 0, compressed_cum = 0, polygon_cum = 0, vertex_cum = 0, n = 0;
    std::vector<boost::shared_ptr<std::stringstream> > ss(num_meshes);

    QFutureSynchronizer<void> thr;
    for (unsigned i = 0; i < num_meshes; ++i) {
	ss[i] = boost::shared_ptr<std::stringstream>(new std::stringstream(std::ios::in | std::ios::out | std::ios::binary));
	thr.addFuture(QtConcurrent::run(this, &Scene::compress, i, boost::ref(*ss[i])));
    }
    thr.waitForFinished();
    for (unsigned i = 0; i < num_meshes; ++i) {
	const unsigned compressed_size = ss[i]->str().size(), original_size = 12 * (num_ver[i] + num_tri[i]);
	original_cum += original_size;
	compressed_cum += compressed_size;
	polygon_cum += num_tri[i];
	vertex_cum += num_ver[i];
	n++;
    }
    thr.clearFutures();
    std::cout << "Compression time: " << t.restart() << "ms" << std::endl;
    for (unsigned i = 0; i < num_meshes; ++i)
	thr.addFuture(QtConcurrent::run(this, &Scene::uncompress, i, boost::ref(*ss[i])));
    thr.waitForFinished();
    std::cout << "Decompression time: " << t.elapsed() << "ms" << std::endl;
    std::cout << "Vertex: " << vertex_cum << ", Triangles: " << polygon_cum
	<< "\nTotals: " << compressed_cum << " / " << original_cum << " = "
	<< (float)compressed_cum / (float)original_cum
	<< ", " << (float)compressed_cum * 8 / vertex_cum << " bpv" << std::endl;

    ss.clear();

    for (unsigned i = 0; i < 2 * num_meshes; ++i) {
	glBindVertexArray(vao[i]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2 * i]);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * num_ver[i], ver[i], GL_STATIC_DRAW);
	glVertexPointer(3, GL_FLOAT, 0, 0);
	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2 * i + 1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(unsigned) * num_tri[i], tri[i], GL_STATIC_DRAW);
	glBindVertexArray(0);
    }
}

Scene::~Scene()
{
    for (unsigned i = 0; i < 2 * num_meshes; i++) {
	delete[] ver[i];
	delete[] tri[i];
    }
    delete[] ver;
    delete[] tri;
    delete[] num_ver;
    delete[] num_tri;

    glDeleteVertexArrays(2 * num_meshes, vao);
    glDeleteBuffers(4 * num_meshes, vbo);
    delete[] vao;
    delete[] vbo;
}

void Scene::build_3d(const std::string& filename)
{
    std::ifstream in(filename.c_str(), std::ios::binary);
    build_3d(in);
    in.close();
}

void Scene::build_3d(std::istream &is)
{
    is.read((char*) &num_meshes, sizeof(unsigned));
    ver = new GLfloat*[2 * num_meshes];
    tri = new unsigned*[2 * num_meshes];
    num_ver = new unsigned[2 * num_meshes];
    num_tri = new unsigned[2 * num_meshes];
    over = ver + num_meshes;
    otri = tri + num_meshes;
    onum_ver = num_ver + num_meshes;
    onum_tri = num_tri + num_meshes;
    for (unsigned i = 0; i < num_meshes; i++) {
	is.read((char*) &num_ver[i], sizeof(unsigned));
	ver[i] = new GLfloat[num_ver[i] * 3];
	is.read((char*) ver[i], sizeof(GLfloat) * 3 * num_ver[i]);

	is.read((char*) &num_tri[i], sizeof(unsigned));
	tri[i] = new unsigned[num_tri[i] * 3];
	is.read((char*) tri[i], sizeof(unsigned) * 3 * num_tri[i]);

	normalize(i);
    }

    vao = new GLuint[2 * num_meshes];
    vbo = new GLuint[4 * num_meshes];
    glGenVertexArrays(2 * num_meshes, vao);
    glGenBuffers(4 * num_meshes, vbo);
}

void Scene::compress(unsigned i, std::stringstream& ss)
{
    try {
	VBE::Writer write(tri[i], num_tri[i], num_ver[i]);
	write.addAttrib(ver[i], num_ver[i], 3, "V");
	write(ss);
    } catch(const std::exception &e) {
	exception_handle(i, e);
    }
}

void Scene::uncompress(unsigned i, std::stringstream& ss)
{
    try {
	VBE::Reader read;
	read(ss);
	onum_tri[i] = read.getNumTri();
	onum_ver[i] = read.getNumVer();
	otri[i] = read.getTriangles();
	over[i] = read.getAttrib("V");
    } catch(const std::exception &e) {
	exception_handle(i, e);
    }
}

void Scene::exception_handle(unsigned i, const std::exception& e)
{
    std::cout << i << ") V: " << num_ver[i] << ", T: " << num_tri[i] << ", Error: " << e.what() << std::endl;
    over[i] = NULL;
    otri[i] = NULL;
    onum_ver[i] = 0;
    onum_tri[i] = 0;
}

void Scene::normalize(const int idx)
{
    double min[3] = {DBL_MAX, DBL_MAX, DBL_MAX}, max[3] = {-DBL_MAX, -DBL_MAX, -DBL_MAX};
    for (unsigned i = 0; i < num_ver[idx]; i++)
	for (int j = 0; j < 3; j++) {
	    if (ver[idx][3 * i + j] < min[j])
		min[j] = ver[idx][3 * i + j];
	    if (ver[idx][3 * i + j] > max[j])
		max[j] = ver[idx][3 * i + j];
	}

    double offset[3], scale[3], max_scale;
    for (int i = 0; i < 3; i++) {
	offset[i] = (max[i] + min[i]) / 2.0;
	scale[i] = max[i] - min[i];
    }
    max_scale = std::max(std::max(scale[0], scale[1]), scale[2]);
    for (unsigned i = 0; i < num_ver[idx]; i++)
	for (int j = 0; j < 3; j++)
	    ver[idx][3 * i + j] = (ver[idx][3 * i + j] - offset[j]) / max_scale;
}

void Scene::draw(unsigned i, const bool show_orig) const
{
    if (i >= num_meshes)
	return;
    if (show_orig == false)
	i += num_meshes;
    glColor3f(1.0f, 0.0f, 0.0f);
    glBindVertexArray(vao[i]);
    glDrawElements(GL_TRIANGLES, 3 * num_tri[i], GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
