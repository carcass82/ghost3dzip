#include "MainWindow.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow *mw = new MainWindow;
    mw->show();
    return app.exec();
}
