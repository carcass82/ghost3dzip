#include "gui.h"
#include "ui_gui.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    origViewable(false), compViewable(false),
    showWireframe(true),
    vertexCount(0), triangleCount(0)
{
    ui->setupUi(this);

    ui->spinQuant->setMaximum(sizeof(float) * CHAR_BIT - 1);

    setupActions();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupActions()
{
    // buttons actions
    connect(ui->btnLoadMesh, SIGNAL(clicked()), this, SLOT(loadMesh()));
    connect(ui->btnCompress, SIGNAL(clicked()), this, SLOT(compressMesh()));

    // settings panel actions
    connect(ui->spinCoeffs, SIGNAL(valueChanged(int)), ui->slideCoeffs, SLOT(setValue(int)));
    connect(ui->slideCoeffs, SIGNAL(valueChanged(int)), ui->spinCoeffs, SLOT(setValue(int)));

    // connect GL signals
    connect(ui->origViewer, SIGNAL(viewerInitialized()), this, SLOT(initGL()));
    connect(ui->origViewer, SIGNAL(drawNeeded()), this, SLOT(updateOrigGL()));
    connect(ui->compViewer, SIGNAL(viewerInitialized()), this, SLOT(initGL()));
    connect(ui->compViewer, SIGNAL(drawNeeded()), this, SLOT(updateCompGL()));
}

void MainWindow::loadMesh(const char *filename)
{
    QString objfile;
    if (filename != 0)
        objfile = QString(filename);

    else
        objfile = QFileDialog::getOpenFileName(this, tr("Load Mesh..."), QDir::currentPath(),
                                               "All supported files (*.off *.obj);;"
                                               "Object File Format (*.off);;"
                                               "Wavefront Obj (*.obj)");

    if (!objfile.isNull()) {

        // reset data and gui labels
        topology.clear();
        geometry.clear();
        origViewable = false;
        compViewable = false;
        vertexCount = 0;
        triangleCount = 0;

        ui->lblMeshLoadTime->setText("Loaded in <b>0 ms</b>");
        ui->lblMeshName->setText("<b>No mesh loaded</b>");
        ui->lblMeshSize->setText("filesize: <b>0 KiB</b>");
        ui->lblMeshCompTime->setText("encoded in <b>0 ms</b>");
        ui->lblMeshCompSize->setText("compressed: <b>0 KiB</b>");
        ui->lblCompRatio->setText("compression ratio: <b>0%</b>");
        ui->lblMeshDecompTime->setText("decoded in <b>0 ms</b>");
        ui->lblRmse->setText("RMSE: <b>0%</b>");

        QTime t;
        t.start();

        try {
            if (objfile.endsWith(".off", Qt::CaseInsensitive))
                OFF::load(objfile.toStdString().c_str(), topology, geometry);
            else
                OBJ::load(objfile.toStdString().c_str(), topology, geometry);
            triangleCount = topology.size() / 3;
            vertexCount = geometry.size() / 3;
            origViewable = true;

        } catch (std::exception &e) {
            QMessageBox::critical(this, "libHDS Error", e.what());
            return;
        }

        // update elapsed loading time
        ui->lblMeshLoadTime->setText("Loaded in <b>" + QString().number(t.elapsed()) + " ms</b>");
        ui->lblMeshLoadTime->adjustSize();

        // update mesh name and size
        ui->lblMeshName->setText("<b>" + QFileInfo(objfile).fileName() + "</b> " +
                                 "(" + QString().number(vertexCount) + " V / " +
                                 QString().number(triangleCount) + " F)");
        ui->lblMeshName->adjustSize();

        ui->lblMeshSize->setText("filesize: <b>" +
                                 QString().number((geometry.size() * sizeof(float) + topology.size() * sizeof(unsigned)) / 1024.0f, 'f', 1) +
                                 " KiB</b>");

        // finally enable the "compress!" button
        ui->btnCompress->setEnabled(true);
    }

    ui->origViewer->updateGL();
}

void MainWindow::initGL()
{
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);

    //glClearColor(1.0, 1.0, 1.0, 1.0);
    //glColor3f(0.0, 0.0, 0.0);

    glClearColor(.0, .0, .0, 1.);
    glColor3f(1., 1., 1.);
}

void MainWindow::updateCompGL()
{
    if (!compViewable)
        return;

    if (showWireframe) {
        glDisable(GL_LIGHTING);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glEnable(GL_LIGHTING);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, &geometryRecon[0]);
    glDrawElements(GL_TRIANGLES, topologyRecon.size(), GL_UNSIGNED_INT, &topologyRecon[0]);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void MainWindow::updateOrigGL()
{
    if (!origViewable)
        return;

    if (showWireframe) {
        glDisable(GL_LIGHTING);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glEnable(GL_LIGHTING);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, &geometry[0]);
    glDrawElements(GL_TRIANGLES, topology.size(), GL_UNSIGNED_INT, &topology[0]);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void MainWindow::compressMesh()
{
    std::stringstream fakeFile(std::ios::in | std::ios::out | std::ios::binary);
    std::vector<unsigned int> topologyOrig(topology);
    std::vector<float>geometryOrig(geometry);

    QTime t;
    t.start();
    {
        VBE::Writer write(&topologyOrig[0], triangleCount, vertexCount, true);
        unsigned int *vo = write.getVertexOrdering();
        unsigned int *fo = write.getFaceOrdering();

        write(fakeFile);

        Utils::UpdateMesh(&topologyOrig[0], topologyOrig.size(), &geometryOrig[0], geometryOrig.size(), vo, fo);
        Utils::OrderIndices(&topologyOrig[0], &topologyOrig[0] + topologyOrig.size());

        delete[] vo;
        delete[] fo;

        Encoder enc(&geometryOrig[0], &topologyOrig[0], geometryOrig.size(), topologyOrig.size());

        int coeffRatio = ui->spinCoeffs->value();
        int patchThreshold = (ui->comboMetis->currentIndex() != 0)? atoi(ui->comboMetis->currentText().toStdString().c_str()) : 0;
        int q = ui->spinQuant->value();

        enc.Encode(fakeFile, coeffRatio, patchThreshold, q);
    }
    ui->lblMeshCompTime->setText("encoded in <b>" + QString().number(t.elapsed()) + " ms</b>");
    ui->lblMeshCompTime->adjustSize();

    ui->lblMeshCompSize->setText("compressed: <b>" + QString().number(fakeFile.str().size() / 1024.0f, 'f', 1) + " KiB</b>");
    ui->lblMeshCompSize->adjustSize();

    int compsize = fakeFile.str().size();
    int origsize = topology.size() * sizeof(unsigned int) + geometry.size() * sizeof(float);
    int compratio = ((1.0 - ((double)compsize / origsize)) * 100.0);
    ui->lblCompRatio->setText("compression ratio: <b>" + QString().number(compratio) + "%</b>");


    t.restart();
    {
        VBE::Reader read;
        read(fakeFile);

        unsigned *f = read.getTriangles();
        unsigned fsize = read.getNumTri() * 3;

        Utils::OrderIndices(f, f + fsize);

        Decoder dec(f, fsize, read.getNumVer());
        dec.Decode(fakeFile);

        topologyRecon.assign(f, f + fsize);
        geometryRecon = dec.getVertices();

        delete[] f;
    }
    ui->lblMeshDecompTime->setText("decoded in <b>" + QString().number(t.elapsed()) + " ms</b>");
    ui->lblMeshDecompTime->adjustSize();

    compViewable = true;
    ui->compViewer->updateGL();

    ui->lblRmse->setText("RMSE: <b>" + QString().number(Utils::ComputeRMSE(geometryOrig, geometryRecon) * 100.0, 'g', 3) + "%</b>");
    ui->lblRmse->adjustSize();
}




int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    if (argc > 1)
        w.loadMesh(argv[1]);
    w.show();

    return a.exec();
}

