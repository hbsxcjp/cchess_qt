#include "mainwindow.h"
#include "piece.h"
#include "test.h"
#include "ui_mainwindow.h"

static const QString programName { "学象棋" };

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //    setWindowIcon(QPixmap(":res/chess.png"));
    setWindowTitle(programName);

    connect(ui->mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),
        this, SLOT(updateActions()));
    connect(ui->mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),
        this, SLOT(setCurrentChessForm(QMdiSubWindow*)));

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
    QMdiSubWindow* subWindow = ui->mdiArea->addSubWindow(new ChessForm);
    subWindow->setWindowIcon(QPixmap(":res/chess.png"));
    subWindow->setAttribute(Qt::WA_DeleteOnClose);
    subWindow->setSystemMenu(Q_NULLPTR);

    subWindow->show();
}

void MainWindow::on_actOpen_triggered()
{
}

void MainWindow::on_actSave_triggered()
{
    if (currentChessForm)
        currentChessForm->saveFile();
}

void MainWindow::on_actSaveAs_triggered()
{
}

void MainWindow::on_actClose_triggered()
{
    ui->mdiArea->closeActiveSubWindow();
}

void MainWindow::on_actCloseAll_triggered()
{
    ui->mdiArea->closeAllSubWindows();
}

void MainWindow::on_actExit_triggered()
{
    close();
}

void MainWindow::on_actAbout_triggered()
{
}

void MainWindow::setCurrentChessForm(QMdiSubWindow* subWindow)
{
    currentChessForm = subWindow ? qobject_cast<ChessForm*>(subWindow->widget()) : Q_NULLPTR;
}

void MainWindow::updateActions()
{
    bool hasActivedSubWindow = bool(ui->mdiArea->activeSubWindow());
    ui->actSave->setEnabled(hasActivedSubWindow);
    ui->actSaveAs->setEnabled(hasActivedSubWindow);
    ui->actClose->setEnabled(hasActivedSubWindow);
    ui->actCloseAll->setEnabled(hasActivedSubWindow);
}

void MainWindow::on_actNextWindow_triggered()
{
    ui->mdiArea->activateNextSubWindow();
}

void MainWindow::on_actPreWindow_triggered()
{
    ui->mdiArea->activatePreviousSubWindow();
}

void MainWindow::on_actTileWindow_triggered()
{
    ui->mdiArea->tileSubWindows();
}

void MainWindow::on_actCascadeWindow_triggered()
{
    ui->mdiArea->cascadeSubWindows();
}
