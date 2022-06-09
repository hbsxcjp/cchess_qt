#include "mainwindow.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    //    QApplication::setAttribute(Qt::AA_Use96Dpi);
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "1");
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication a(argc, argv);
    a.setOrganizationName("person");
    a.setApplicationName("studyChess");

    MainWindow w;
    w.show();
    return a.exec();
}
