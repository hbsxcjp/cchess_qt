#include "ecco.h"
#include "mainwindow.h"
#include "piece.h"
#include "tools.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();
    return a.exec();
}
