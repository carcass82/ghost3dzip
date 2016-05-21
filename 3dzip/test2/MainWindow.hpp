#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include "ui_Main.h"

class Scene;

class MainWindow: public QWidget, private Ui::MainForm
{
    Q_OBJECT;
    Scene *scene;

    virtual void draw() const;

public:
    MainWindow(QWidget *parent = NULL);
    ~MainWindow();

private slots:
    void on_pb_load_pressed();
    void on_radio_orig_toggled(bool checked) {
	widget->showOrig(checked);
    }
    void on_part_select_valueChanged(const int val) {
	widget->selectPart(val);
	lbl_numtri->setText(QString("Polygons: %1").arg(widget->getNumTri()));
    }
};

#endif
