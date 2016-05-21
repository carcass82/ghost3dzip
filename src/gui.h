#pragma once
#ifndef GUI_H
#define GUI_H

#include <QtGui/QApplication>
#include <QMainWindow>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>

#include "hds/OFF.hh"
#include "hds/OBJ.hh"

#include "3dzip/Writer.hh"
#include "3dzip/Reader.hh"

#include "encoder.h"
#include "decoder.h"


namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

public slots:
	void loadMesh(const char *filename = 0);

private slots:
	void initGL();
	void updateCompGL();
	void updateOrigGL();
	void compressMesh();

private:
	void setupActions();

	bool showWireframe;
	int vertexCount, triangleCount;
	std::vector<unsigned int> topology, topologyRecon;
	std::vector<float> geometry, geometryRecon;
	bool origViewable, compViewable;
	Ui::MainWindow *ui;
};

#endif // GUI_H
