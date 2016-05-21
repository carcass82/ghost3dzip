#include "MainWindow.hpp"
#include "Scene.hpp"

MainWindow::MainWindow(const QString &filename, QWidget *parent) : GLViewer(parent), scene(NULL)
{
    this->setWindowTitle(filename);
}

MainWindow::~MainWindow()
{
    delete scene;
}

void MainWindow::initializeGL()
{
    GLViewer::initializeGL();
    scene = new Scene(windowTitle().toStdString());
}

void MainWindow::draw() const
{
    scene->draw();
}
