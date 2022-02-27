#include "mainwindow.h"
#include "instanceio.h"
#include "piece.h"
#include "test.h"
#include "tools.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>

static const QStringList stringLiterals {
    QStringLiteral("学象棋"),
    QStringLiteral("mainWindow"),
    QStringLiteral("geometry"),
    QStringLiteral("viewmode"),
    QStringLiteral("splitter"),
    QStringLiteral("navIndex"),
    QStringLiteral("subWindow"),
    QStringLiteral("source"),
    QStringLiteral("locate"),
    QStringLiteral("recentFileList"),
    QStringLiteral("file")
};

enum StringLiteralIndex {
    WINDOWTITLE,
    MAINWINDOW,
    GEOMETRY,
    VIEWMODE,
    SPLITTER,
    NAVINDEX,
    SUBWINDOW,
    SOURCE,
    LOCATE,
    RECENTFILELIST,
    FILEKEY
};

static const int actUsersTag { 1 };

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->mdiArea, &QMdiArea::subWindowActivated, this, &MainWindow::updateMainActions);
    connect(ui->menuFile, &QMenu::aboutToShow, this, &MainWindow::updateFileMenu);
    connect(ui->menuWindow, &QMenu::aboutToShow, this, &MainWindow::updateWindowMenu);
    connect(ui->menuRecent, &QMenu::aboutToShow, this, &MainWindow::updateRecentFileActions);
    QList<QAction*> actRecents = ui->menuRecent->actions();
    for (int i = 0; i < actRecents.size() - 2; ++i) // 排除最后两个按钮
        connect(actRecents.at(i), &QAction::triggered, this, &MainWindow::openRecentFile);

    windowMenuSeparatorAct = new QAction(this);
    windowMenuSeparatorAct->setSeparator(true);
    ui->menuWindow->addAction(windowMenuSeparatorAct);
    ui->actRecentFileClear->setData(actUsersTag);
    updateMainActions();

    readSettings();
    setWindowTitle(stringLiterals.at(StringLiteralIndex::WINDOWTITLE));
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::openFile(const QString& fileName)
{
    if (QMdiSubWindow* existing = findChessForm(fileName)) {
        ui->mdiArea->setActiveSubWindow(existing);
        return true;
    }

    const bool succeeded = loadFile(fileName);
    if (succeeded)
        statusBar()->showMessage(QString("文件加载完成: %1").arg(fileName), 2000);

    return succeeded;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    writeSettings();
    ui->mdiArea->closeAllSubWindows();
    if (ui->mdiArea->currentSubWindow()) {
        event->ignore();
    } else {
        event->accept();
    }
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
    ChessForm* chessForm = createChessForm();
    chessForm->newFile();
    chessForm->show();
}

void MainWindow::on_actOpen_triggered()
{
    const QString fileName = QFileDialog::getOpenFileName(this, "打开棋谱", "./", ChessForm::getFilter());
    if (!fileName.isEmpty())
        openFile(fileName);
}

void MainWindow::on_actSave_triggered()
{
    saveFile(false);
}

void MainWindow::on_actSaveAs_triggered()
{
    saveFile(true);
}

void MainWindow::on_actRecentFileClear_triggered()
{
    handleRecentFiles(QString());
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

void MainWindow::on_actNextWindow_triggered()
{
    ui->mdiArea->activateNextSubWindow();
}

void MainWindow::on_actPreWindow_triggered()
{
    ui->mdiArea->activatePreviousSubWindow();
}

void MainWindow::on_actTabShowWindow_triggered(bool checked)
{
    if (checked) {
        ui->mdiArea->setViewMode(QMdiArea::TabbedView);

        ui->actTileWindow->setEnabled(false);
        ui->actCascadeWindow->setEnabled(false);
    } else {
        ui->mdiArea->setViewMode(QMdiArea::SubWindowView);

        ui->actTileWindow->setEnabled(true);
        ui->actCascadeWindow->setEnabled(true);
    }
}

void MainWindow::on_actTileWindow_triggered()
{
    ui->mdiArea->tileSubWindows();
}

void MainWindow::on_actCascadeWindow_triggered()
{
    ui->mdiArea->cascadeSubWindows();
}

void MainWindow::on_actAbout_triggered()
{
    QMessageBox::about(this, "关于",
        "一个学习象棋、可以打谱的软件，\t\n内部含有大量实战棋谱，欢迎使用！\t\n\ncjp\n2022.12.31\n");
}

void MainWindow::updateMainActions()
{
    bool hasActivedSubWindow = bool(ui->mdiArea->activeSubWindow());
    ui->actSave->setEnabled(hasActivedSubWindow);
    ui->actSaveAs->setEnabled(hasActivedSubWindow);

    ui->actImportFile->setEnabled(hasActivedSubWindow);
    ui->actExportFile->setEnabled(hasActivedSubWindow);
    ui->actSaveDatabase->setEnabled(hasActivedSubWindow);

    ui->actClose->setEnabled(hasActivedSubWindow);
    ui->actCloseAll->setEnabled(hasActivedSubWindow);
}

void MainWindow::updateFileMenu()
{
    QSettings settings;
    const int count = settings.beginReadArray(stringLiterals.at(StringLiteralIndex::RECENTFILELIST));
    settings.endArray();
    ui->menuRecent->setEnabled(count > 0);
}

void MainWindow::updateWindowMenu()
{
    for (QAction* action : ui->menuWindow->actions())
        if (action->data().toInt() == actUsersTag)
            ui->menuWindow->removeAction(action);

    QList<QMdiSubWindow*> subWindowList = ui->mdiArea->subWindowList();
    windowMenuSeparatorAct->setVisible(!subWindowList.isEmpty());
    for (int i = 0; i < subWindowList.size(); ++i) {
        QMdiSubWindow* subWindow = subWindowList.at(i);
        ChessForm* chessForm = getChessForm(subWindow);
        QAction* action = ui->menuWindow->addAction(chessForm->getFriendlyFileName(), subWindow,
            [this, subWindow]() {
                ui->mdiArea->setActiveSubWindow(subWindow);
            });
        action->setCheckable(true);
        action->setChecked(subWindow == ui->mdiArea->activeSubWindow());
        action->setData(actUsersTag);
    }
}

void MainWindow::writeSettings()
{
    QSettings settings;
    settings.beginGroup(stringLiterals[StringLiteralIndex::MAINWINDOW]);

    settings.setValue(stringLiterals[StringLiteralIndex::GEOMETRY], saveGeometry());
    settings.setValue(stringLiterals[StringLiteralIndex::VIEWMODE], ui->actTabShowWindow->isChecked());
    settings.setValue(stringLiterals[StringLiteralIndex::SPLITTER], ui->splitter->saveState());
    settings.setValue(stringLiterals[StringLiteralIndex::NAVINDEX], ui->navTabWidget->currentIndex());

    settings.beginWriteArray(stringLiterals[StringLiteralIndex::SUBWINDOW]);
    QList<QMdiSubWindow*> subWindowList = ui->mdiArea->subWindowList();
    for (int i = 0; i < subWindowList.size(); ++i) {
        settings.setArrayIndex(i);
        QMdiSubWindow* subWindow = subWindowList.at(i);
        settings.setValue(stringLiterals[StringLiteralIndex::FILEKEY], getChessForm(subWindow)->getFileName());
        settings.setValue(stringLiterals[StringLiteralIndex::GEOMETRY], subWindow->saveGeometry());
    }
    settings.endArray();

    settings.endGroup();
}

void MainWindow::readSettings()
{
    QSettings settings;
    settings.beginGroup(stringLiterals[StringLiteralIndex::MAINWINDOW]);

    restoreGeometry(settings.value(stringLiterals[StringLiteralIndex::GEOMETRY]).toByteArray());
    bool tabShowIsChecked = settings.value(stringLiterals[StringLiteralIndex::VIEWMODE]).toBool();
    on_actTabShowWindow_triggered(tabShowIsChecked);
    ui->splitter->restoreState(settings.value(stringLiterals[StringLiteralIndex::SPLITTER]).toByteArray());
    ui->navTabWidget->setCurrentIndex(settings.value(stringLiterals[StringLiteralIndex::NAVINDEX]).toInt());

    int size = settings.beginReadArray(stringLiterals[StringLiteralIndex::SUBWINDOW]);
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString fileName = settings.value(stringLiterals[StringLiteralIndex::FILEKEY]).toString();
        if (openFile(fileName) && !tabShowIsChecked) {
            QMdiSubWindow* subWindow = ui->mdiArea->currentSubWindow();
            if (subWindow)
                subWindow->restoreGeometry(settings.value(stringLiterals[StringLiteralIndex::GEOMETRY]).toByteArray());
        }
    }
    settings.endArray();

    settings.endGroup();
}

void MainWindow::saveFile(bool isSaveAs)
{
    ChessForm* chessForm = activeChessForm();
    if (!chessForm)
        return;

    if (isSaveAs ? chessForm->saveAs() : chessForm->save()) {
        statusBar()->showMessage("文件已保存.", 2000);
        handleRecentFiles(chessForm->getFileName());
    }
}

bool MainWindow::loadFile(const QString& fileName)
{
    ChessForm* chessForm = createChessForm();
    const bool succeeded = chessForm->loadFile(fileName);
    if (succeeded) {
        chessForm->show();
        handleRecentFiles(fileName);
    } else
        chessForm->close();

    return succeeded;
}

void MainWindow::handleRecentFiles(const QString& fileName)
{
    QSettings settings;
    const QStringList oldRecentFiles = Tools::readStringList(settings,
        stringLiterals[StringLiteralIndex::RECENTFILELIST],
        stringLiterals[StringLiteralIndex::FILEKEY]);
    QStringList recentFiles = oldRecentFiles;

    if (fileName.isEmpty()) {
        for (QString& file : recentFiles)
            if (!findChessForm(file))
                recentFiles.removeAll(file);
    } else {
        recentFiles.removeAll(fileName);
        recentFiles.prepend(fileName);
    }

    if (oldRecentFiles != recentFiles)
        Tools::writeStringList(recentFiles, settings,
            stringLiterals[StringLiteralIndex::RECENTFILELIST],
            stringLiterals[StringLiteralIndex::FILEKEY]);

    ui->menuRecent->setEnabled(!recentFiles.isEmpty());
}

void MainWindow::updateRecentFileActions()
{
    QSettings settings;
    const QStringList recentFiles = Tools::readStringList(settings, stringLiterals[StringLiteralIndex::RECENTFILELIST],
        stringLiterals[StringLiteralIndex::FILEKEY]);
    QList<QAction*> recentActs = ui->menuRecent->actions();
    const int actCount = recentActs.size() - 2, // 排除最后的分隔符和清除按钮
        count = qMin(actCount, recentFiles.size());
    int i = 0;
    for (; i < count; ++i) {
        const QString fileName = QFileInfo(recentFiles.at(i)).fileName();
        QAction* action = recentActs.at(i);
        action->setText(QString("&%1 %2").arg(i + 1).arg(fileName));
        action->setData(recentFiles.at(i));
        action->setVisible(true);
    }
    for (; i < actCount; ++i) {
        recentActs.at(i)->setVisible(false);
    }
}

void MainWindow::openRecentFile()
{
    if (const QAction* action = qobject_cast<const QAction*>(sender()))
        openFile(action->data().toString());
}

ChessForm* MainWindow::createChessForm()
{
    ChessForm* chessForm = new ChessForm;
    QMdiSubWindow* subWindow = ui->mdiArea->addSubWindow(chessForm);
    subWindow->setAttribute(Qt::WA_DeleteOnClose);
    //    subWindow->setWindowFlags(Qt::Dialog);
    subWindow->setSystemMenu(Q_NULLPTR);
    // 使在设置某些窗口组件可见性后，自动重新定义大小
    subWindow->layout()->setSizeConstraint(QLayout::SetMinimumSize);

    return chessForm;
}

ChessForm* MainWindow::getChessForm(QMdiSubWindow* subWindow) const
{
    return qobject_cast<ChessForm*>(subWindow->widget());
}

ChessForm* MainWindow::activeChessForm() const
{
    if (QMdiSubWindow* subWindow = ui->mdiArea->activeSubWindow())
        return getChessForm(subWindow);

    return Q_NULLPTR;
}

QMdiSubWindow* MainWindow::findChessForm(const QString& fileName) const
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    for (QMdiSubWindow* subWindow : ui->mdiArea->subWindowList()) {
        if (getChessForm(subWindow)->getFileName() == canonicalFilePath)
            return subWindow;
    }

    return Q_NULLPTR;
}
