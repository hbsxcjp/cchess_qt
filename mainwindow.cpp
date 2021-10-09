#include "mainwindow.h"
#include "piece.h"
#include "test.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    on_actTest_triggered();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actTest_triggered()
{
    TestPiece tpiece;
    QTest::qExec(&tpiece);

    TestBoard tboard;
    QTest::qExec(&tboard);
}
