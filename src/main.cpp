#include "ecco.h"
#include "mainwindow.h"
#include "piece.h"
#include "tools.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("person");
    a.setApplicationName("studyChess");

    MainWindow w;
    w.show();
    return a.exec();
}
