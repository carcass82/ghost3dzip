#include <QFileDialog>
#include "MainWindow.hpp"
#include "Scene.hpp"

MainWindow::MainWindow(QWidget *parent) : QWidget(parent), scene(NULL)
{
    setupUi(this);
}

MainWindow::~MainWindow()
{
    delete scene;
}

void MainWindow::draw() const
{
    if (scene != NULL)
	scene->draw();
}

void MainWindow::on_pb_load_pressed()
{
    QString filename = QFileDialog::getOpenFileName(this, "Select model to compress", "../models", "Models (*.3d)");
    if (filename == "")
	return;
    this->setWindowTitle(filename);
    delete scene;
    scene = new Scene(windowTitle().toStdString());
    widget->setScene(scene);
    part_select->setMaximum(scene->getNumMeshes());
    on_part_select_valueChanged(0);
}
