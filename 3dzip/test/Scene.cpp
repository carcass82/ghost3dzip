#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <vector>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <QTime>
#include <QSettings>
#include <GL/glew.h>
#include "Scene.hpp"
#include "hds/OFF.hh"
#include "3dzip/Writer.hh"
#include "3dzip/Reader.hh"
#ifdef HAVE_PLY
#include "ply_reader.hpp"
#endif

static void build_scene(const std::string &filename, std::vector<unsigned>& tri, std::vector<float>& ver);
static void build_dummy(std::vector<unsigned>& tri, std::vector<float>& ver);

Scene::Scene(const std::string &filename)
{
    std::cout << "Model: " << filename << std::endl;
    std::vector<unsigned> tri;
    std::vector<float> ver;
    build_scene(filename, tri, ver);
    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    std::cout << "Vertices: " << ver.size() / 3
	<< "\nTriangles: " << tri.size() / 3 << std::endl;
    std::cout << "Encoding... " << std::flush;
    QSettings settings("3dzip.ini", QSettings::IniFormat);
    bool check_manifold = settings.value("check_manifold", true).toBool();
    QTime t;
    t.start();
    VBE::Writer write(&tri[0], tri.size() / 3, ver.size() / 3, check_manifold);
    write.addAttrib(&ver[0], ver.size() / 3, 3, "V");
    write(ss);
    size_t compressed_size = ss.str().size();
    float compression_ratio = double(compressed_size) / (tri.size() * sizeof(unsigned) + ver.size() * sizeof(float));
    std::cout << t.elapsed() << " ms" << "\nSize: " << compressed_size << " bytes" << "\nSpace Saving: " << (1.0 - compression_ratio) * 100.0 << "%\nCompression ratio: " << compression_ratio << "\nBit per vertex: " << double(compressed_size) * CHAR_BIT / (ver.size() / 3.0) << "\nDecoding..." << std::flush;
    GLfloat *ogeometry;
    GLuint *otri;
    t.start();
    VBE::Reader read;
    read(ss);
    onum_tri = read.getNumTri();
    onum_ver = read.getNumVer();
    otri = read.getTriangles();
    ogeometry = read.getAttrib("V");
    std::cout << t.elapsed() << " ms" << std::endl;

    glGenBuffers(N_BO, bo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bo[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, onum_tri * 3 * sizeof(GLuint), otri, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, bo[1]);
    glBufferData(GL_ARRAY_BUFFER, onum_ver * 3 * sizeof(GLfloat), ogeometry, GL_STATIC_DRAW);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glEnableClientState(GL_VERTEX_ARRAY);

    delete[] ogeometry;
    delete[] otri;

    glColor3f(1.0f, 0.0f, 0.0f);
}

Scene::~Scene()
{
    glDeleteBuffers(N_BO, bo);
}

void build_scene(const std::string &filename, std::vector<unsigned>& tri, std::vector<float>& ver)
{
    if (filename == "") {
	build_dummy(tri, ver);
	return;
    }
    std::cout << "Loading... " << std::flush;
    QTime t;
    t.start();
    if (filename.substr(filename.size() - 4) == ".off")
	OFF::load(filename.c_str(), tri, ver);
#ifdef HAVE_PLY
    else if (filename.substr(filename.size() - 4) == ".ply") {
	ply::reader read(ver, tri);
	read(filename);
    }
#endif
    else
	throw std::logic_error("Parser for file " + filename + " missing");
    std::cout << t.elapsed() << " ms" << std::endl;
}

void build_dummy(std::vector<unsigned>& tri, std::vector<float>& ver)
{
    float tmp_ver[] = {
	0.0f, 0.0f, 1.0f,
	-1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, -1.0f,
	-0.5f, 0.4f, 0.0f,
	0.5f, 0.4f, 0.0f
    };
    std::copy(tmp_ver, tmp_ver + sizeof(tmp_ver) / sizeof(float), ver.begin());

    unsigned tmp_tri[] = {
	0, 1, 2,
	0, 2, 3,
	1, 4, 2,
	2, 4, 3,
	1, 5, 2,
	2, 6, 3
    };
    std::copy(tmp_tri, tmp_tri + sizeof(tmp_tri) / sizeof(unsigned), tri.begin());
}

void Scene::draw() const
{
    glDrawElements(GL_TRIANGLES, onum_tri * 3, GL_UNSIGNED_INT, 0);
}
