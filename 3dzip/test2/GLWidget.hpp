#ifndef GL_WIDGET_HPP
#define GL_WIDGET_HPP

#include "GLViewer.hpp"

class Scene;

class GLWidget : public GLViewer
{
    Q_OBJECT;
    Scene *scene;
    bool show_orig;
    unsigned selected_part;

    int mouse_x, mouse_y;
    bool mouse_l, mouse_r;
public:
    GLWidget(QWidget *parent = NULL);
    void setScene(Scene *_scene);
    void showOrig(const bool b = true) {
	show_orig = b;
	updateGL();
    }
    void selectPart(const unsigned i);
    unsigned getNumTri() const;
    virtual void draw() const;
};

#endif
