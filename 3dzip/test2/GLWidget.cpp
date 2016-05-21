#include <climits>
#include "GLWidget.hpp"
#include "Scene.hpp"

GLWidget::GLWidget(QWidget *parent) : GLViewer(parent), scene(NULL), show_orig(false), selected_part(UINT_MAX), mouse_l(false), mouse_r(false)
{
}

void GLWidget::setScene(Scene *_scene)
{
    scene = _scene;
    selectPart(0);
    updateGL();
}

void GLWidget::selectPart(const unsigned i)
{
    if (i < scene->getNumMeshes())
	selected_part = i;
    updateGL();
}

unsigned GLWidget::getNumTri() const
{
    return scene->getNumTri(selected_part);
}

void GLWidget:: draw() const
{
    if (scene != NULL)
	scene->draw(selected_part, show_orig);
}
