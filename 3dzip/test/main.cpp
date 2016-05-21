#include <QApplication>
#include "MainWindow.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow m(argc < 2 ? "" : argv[1]);
    m.show();
    return app.exec();
}
