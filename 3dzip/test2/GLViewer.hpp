#ifndef GL_VIEWER_HPP
#define GL_VIEWER_HPP

#include <QGLWidget>

class GLViewer : public QGLWidget
{
    Q_OBJECT;
    QPoint p;
    float x, y, s;
    float obj[16];

protected:
    virtual void initializeGL();
    virtual void paintGL();
    virtual void resizeGL(int w, int h);

    virtual void keyPressEvent(QKeyEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void wheelEvent(QWheelEvent *);

    virtual void draw() const = 0;

public:
    GLViewer(QWidget *parent = NULL);
};

#endif
