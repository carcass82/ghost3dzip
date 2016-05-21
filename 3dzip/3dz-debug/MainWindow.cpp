#include <sstream>
#include <GL/glew.h>
#include <boost/bind.hpp>
#include <QFileDialog>
#include <hds/OFF.hh>
#include <hds/HDS.hh>
#include <3dzip/ManifoldCompressorFacade.hh>
#include <3dzip/Compressor.hh>
#include <3dzip/Symbol.hh>
#include <3dzip/ActiveStack.hh>
#include "MainWindow.hpp"
#include "ui_MainWindow.h"

struct Impl
{
    HDS::Mesh* mesh;
    VBE::ManifoldCompressorFacade* f;
    VBE::Compressor* compr;
    const VBE::ActiveStack* stack;
    Impl(): mesh(0), f(0), compr(0), stack(0) {}
};

MainWindow::MainWindow(QWidget* parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    count(0), p(new Impl)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    glDeleteBuffers(2, vbo);
    delete ui;
    delete p->compr;
    delete p->f;
    delete p->mesh;
    delete p;
}

void MainWindow::on_action_Open_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open 3D model", "", "3D Model (*.off)");
    if (filename.isEmpty() == true)
	return;
    std::vector<float> ver;
    std::vector<unsigned> tri;
    OFF::load(filename.toAscii().data(), tri, ver);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * tri.size(), &tri[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * ver.size(), &ver[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    count = tri.size();
    delete p->compr;
    delete p->f;
    delete p->mesh;
    p->mesh = new HDS::Mesh(&tri[0], &tri[0] + tri.size(), ver.size() / 3);
    p->f = new VBE::ManifoldCompressorFacade(*p->mesh);
    p->compr = new VBE::Compressor(*p->f, boost::bind(&MainWindow::output_symbol, this, _1));
    p->stack = &p->compr->getActiveStack();
    ui->output->clear();
}

void MainWindow::on_action_Next_triggered()
{
    p->compr->step();
    ui->activelists->clear();
    for (unsigned i = 0; i < p->stack->size(); ++i) {
	new QListWidgetItem(QString::number(i), ui->activelists);
    }
    ui->viewer->updateGL();
}

void MainWindow::on_action_Terminate_triggered()
{
    p->compr->run();
}

void MainWindow::on_viewer_viewerInitialized()
{
    glewInit();
    glGenBuffers(2, vbo);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH);
}

void MainWindow::on_viewer_drawNeeded()
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glLineWidth(1);
    glColor3f(1, 1, 1);
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glLineWidth(3);
    glColor3f(0, 1, 0);
    if (p->stack != 0 && p->stack->empty() == false) {
	glBegin(GL_LINE_LOOP);
	const VBE::ActiveList* l = p->stack->top();
	for (VBE::ActiveList::ConstIterator i = l->begin(); i != l->end(); ++i) {
	    HDS::Handle_t h = i->h;
	    if (p->mesh->isDummy(h) == true)
		continue;
	    glArrayElement(p->mesh->element(h));
	}
	glEnd();
	if (p->mesh->isDummy(l->focus()) == false) {
	    glPointSize(5);
	    glColor3f(0, 0, 1);
	    glBegin(GL_POINTS);
	    glArrayElement(p->mesh->element(l->focus()));
	    glEnd();
	}
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void MainWindow::output_symbol(const VBE::Symbol& s)
{
    std::ostringstream out;
    out << s;
    QString str(out.str().c_str());
    ui->statusbar->showMessage("Last symbol: " + str, 4000);
    new QListWidgetItem(str, ui->output);
}
