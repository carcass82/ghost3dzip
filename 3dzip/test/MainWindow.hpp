#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include "GLViewer.hpp"

class Scene;

class MainWindow : public GLViewer
{
    Q_OBJECT;
    Scene* scene;

    virtual void initializeGL();
    virtual void draw() const;

public:
    MainWindow(const QString &filename, QWidget *parent = NULL);
    ~MainWindow();
};

#endif
