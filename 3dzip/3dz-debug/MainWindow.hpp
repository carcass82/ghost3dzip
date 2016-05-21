#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

struct Impl;

namespace VBE {
    class Symbol;
}

class MainWindow: public QMainWindow
{
    Q_OBJECT
    Ui::MainWindow* ui;
    unsigned vbo[2], count;
    Impl* p;
    void output_symbol(const VBE::Symbol& s);
public:
    MainWindow(QWidget* parent = 0);
    ~MainWindow();
private slots:
    void on_action_Open_triggered();
    void on_action_Next_triggered();
    void on_action_Terminate_triggered();
    void on_viewer_viewerInitialized();
    void on_viewer_drawNeeded();
};
