#include <GL/glew.h>
#include <QApplication>
#include <QKeyEvent>
#include "GLViewer.hpp"

GLViewer::GLViewer(QWidget *parent) : QGLWidget(parent), x(0), y(0), s(1)
{
}

void GLViewer::initializeGL()
{
    glewInit();
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    //glFrontFace(GL_CW);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLoadIdentity();
    glGetFloatv(GL_MODELVIEW_MATRIX, obj);
    gluLookAt(0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
}

void GLViewer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(s, s, s);
    glMultMatrixf(obj);
    draw();
    glPopMatrix();
}

void GLViewer::resizeGL(const int w, const int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double ratio = (double)w / h;
    //gluPerspective(60.0, (double)w / h, 0.1, 100.0);
    glOrtho(-2, 2, -2 / ratio, 2 / ratio, 0.1, 1000);
    glMatrixMode(GL_MODELVIEW);
}

void GLViewer::keyPressEvent(QKeyEvent *ev)
{
    switch (ev->key()) {
	case Qt::Key_Escape:
	    qApp->quit();
	    break;
	default:
	    break;
    }
}

void GLViewer::mousePressEvent(QMouseEvent *e)
{
    p = e->pos();
}

void GLViewer::mouseMoveEvent(QMouseEvent *e)
{
    QPoint d = e->pos() - p;
    if (e->buttons() & Qt::RightButton) {
	glPushMatrix();
	glLoadIdentity();
	glRotatef(d.x(), 0, 1, 0);
	glRotatef(d.y(), 1, 0, 0);
	glMultMatrixf(obj);
	glGetFloatv(GL_MODELVIEW_MATRIX, obj);
	glPopMatrix();
    }
    if (e->buttons() & Qt::LeftButton) {
	x += 0.01f * d.x();
	y -= 0.01f * d.y();
    }
    p = e->pos();
    updateGL();
}

void GLViewer::wheelEvent(QWheelEvent *e)
{
    s *= 1.0f + ((float)e->delta()) / 120.0f * 0.1f;
    updateGL();
}
