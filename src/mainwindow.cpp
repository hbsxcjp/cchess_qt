#include "mainwindow.h"
#include "piece.h"
#include "test.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    setWindowIcon(QPixmap("res/chess.png"));
    ui->setupUi(this);

    //    on_actTest_triggered();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actTest_triggered()
{
    TestPiece tpiece;
    QTest::qExec(&tpiece);

    TestSeat tseat;
    QTest::qExec(&tseat);

    TestBoard tboard;
    QTest::qExec(&tboard);

    TestInstance tins;
    QTest::qExec(&tins);

    TestAspect tasp;
    QTest::qExec(&tasp);

    TestInitEcco tecco;
    QTest::qExec(&tecco); // , { "-o output/testOutput.txt txt" }
}

void MainWindow::on_actNew_triggered()
{
    QMdiSubWindow* subWindow = new QMdiSubWindow;
    ChessForm* chessForm = new ChessForm(subWindow);
    subWindow->setWidget(chessForm);
    subWindow->setAttribute(Qt::WA_DeleteOnClose);
    subWindow->setSystemMenu(Q_NULLPTR);
    ui->mdiArea->addSubWindow(subWindow, Qt::Dialog);

    subWindow->show();
    //    subWindow->widget()->show();
}

void MainWindow::on_actClose_triggered()
{
    if (ui->mdiArea->activeSubWindow())
        ui->mdiArea->activeSubWindow()->close();
}
