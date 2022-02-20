#include "mainwindow.h"
#include "piece.h"
#include "test.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>

static const QString programName { "学象棋" };
static const QStringList settingNames {
    "mainWindow", "geometry", "splitter", "navIndex",
    "subWindow", "source", "locate"
};

enum SettingNameIndex {
    MAINWINDOW,
    GEOMETRY,
    SPLITTER,
    NAVINDEX,
    SUBWINDOW,
    SOURCE,
    LOCATE
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->mdiArea, &QMdiArea::subWindowActivated,
        this, &MainWindow::updateActions);
    connect(ui->mdiArea, &QMdiArea::subWindowActivated,
        this, &MainWindow::updateWindowMenu);

    updateActions();
    updateWindowMenu();
    readSettings();

    setWindowTitle(programName);
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
        statusBar()->showMessage("文件加载完成.", 2000);
    return succeeded;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    ui->mdiArea->closeAllSubWindows();
    if (ui->mdiArea->currentSubWindow()) {
        event->ignore();
    } else {
        writeSettings();
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

    QMdiSubWindow* subWindow = ui->mdiArea->addSubWindow(chessForm);
    subWindow->setWindowIcon(QPixmap(":res/chess.png"));
    subWindow->setAttribute(Qt::WA_DeleteOnClose);
    subWindow->setSystemMenu(Q_NULLPTR);

    subWindow->show();
}

void MainWindow::on_actOpen_triggered()
{

    //    QString fileName = QFileDialog::getSaveFileName(this, "另存为",
    //        "./", QString("棋谱文件(%1)").arg(InstanceIO::getSuffixNames().join(' ')));

    const QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty())
        openFile(fileName);
}

void MainWindow::on_actSave_triggered()
{
    if (activeChessForm() && activeChessForm()->save())
        statusBar()->showMessage(tr("File saved"), 2000);
}

void MainWindow::on_actSaveAs_triggered()
{
    ChessForm* child = activeChessForm();
    if (child && child->saveAs()) {
        statusBar()->showMessage(tr("File saved"), 2000);
        MainWindow::prependToRecentFiles(child->getFileName());
    }
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
    QMessageBox::about(this, tr("About MDI"),
        tr("The <b>MDI</b> example demonstrates how to write multiple "
           "document interface applications using Qt."));
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

void MainWindow::updateActions()
{
    bool hasActivedSubWindow = bool(ui->mdiArea->activeSubWindow());
    ui->actSave->setEnabled(hasActivedSubWindow);
    ui->actSaveAs->setEnabled(hasActivedSubWindow);
    ui->actClose->setEnabled(hasActivedSubWindow);
    ui->actCloseAll->setEnabled(hasActivedSubWindow);
}

void MainWindow::updateWindowMenu()
{
}

ChessForm* MainWindow::createChessForm()
{
    ChessForm* chessForm = new ChessForm;
    ui->mdiArea->addSubWindow(chessForm);

    //#ifndef QT_NO_CLIPBOARD
    //    connect(child, &QTextEdit::copyAvailable, cutAct, &QAction::setEnabled);
    //    connect(child, &QTextEdit::copyAvailable, copyAct, &QAction::setEnabled);
    //#endif

    return chessForm;
}

void MainWindow::writeSettings()
{
    QSettings settings;
    settings.beginGroup(settingNames[SettingNameIndex::MAINWINDOW]);
    settings.setValue(settingNames[SettingNameIndex::GEOMETRY], saveGeometry());
    settings.setValue(settingNames[SettingNameIndex::SPLITTER], ui->splitter->saveState());
    settings.setValue(settingNames[SettingNameIndex::NAVINDEX], ui->navTabWidget->currentIndex());

    settings.beginWriteArray(settingNames[SettingNameIndex::SUBWINDOW]);
    QList<QMdiSubWindow*> subWindows = ui->mdiArea->subWindowList();
    for (int i = 0; i < subWindows.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue(settingNames[SettingNameIndex::GEOMETRY], subWindows.at(i)->saveGeometry());
        //        settings.setValue("source",subWindows.at(i).source());
        //        settings.setValue("locate",subWindows.at(i).locate());
    }

    settings.endArray();
    settings.endGroup();
}

void MainWindow::readSettings()
{
    QSettings settings;
    settings.beginGroup(settingNames[SettingNameIndex::MAINWINDOW]);
    restoreGeometry(settings.value(settingNames[SettingNameIndex::GEOMETRY]).toByteArray());
    ui->splitter->restoreState(settings.value(settingNames[SettingNameIndex::SPLITTER]).toByteArray());
    ui->navTabWidget->setCurrentIndex(settings.value(settingNames[SettingNameIndex::NAVINDEX]).toInt());

    int size = settings.beginReadArray(settingNames[SettingNameIndex::SUBWINDOW]);
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        //        Login login;
        //        login.userName = settings.value("userName").toString();
        //        login.password = settings.value("password").toString();
        //        logins.append(login);
    }

    settings.endArray();
    settings.endGroup();
}

bool MainWindow::loadFile(const QString& fileName)
{
    ChessForm* child = createChessForm();
    const bool succeeded = child->loadFile(fileName);
    if (succeeded)
        child->show();
    else
        child->close();
    MainWindow::prependToRecentFiles(fileName);
    return succeeded;
}

static inline QString recentFilesKey() { return QStringLiteral("recentFileList"); }
static inline QString fileKey() { return QStringLiteral("file"); }

static QStringList readRecentFiles(QSettings& settings)
{
    QStringList result;
    const int count = settings.beginReadArray(recentFilesKey());
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        result.append(settings.value(fileKey()).toString());
    }
    settings.endArray();
    return result;
}

static void writeRecentFiles(const QStringList& files, QSettings& settings)
{
    const int count = files.size();
    settings.beginWriteArray(recentFilesKey());
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        settings.setValue(fileKey(), files.at(i));
    }
    settings.endArray();
}

bool MainWindow::hasRecentFiles()
{
    QSettings settings;
    const int count = settings.beginReadArray(recentFilesKey());
    settings.endArray();
    return count > 0;
}

void MainWindow::prependToRecentFiles(const QString& fileName)
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    const QStringList oldRecentFiles = readRecentFiles(settings);
    QStringList recentFiles = oldRecentFiles;
    recentFiles.removeAll(fileName);
    recentFiles.prepend(fileName);
    if (oldRecentFiles != recentFiles)
        writeRecentFiles(recentFiles, settings);

    setRecentFilesVisible(!recentFiles.isEmpty());
}

void MainWindow::setRecentFilesVisible(bool visible)
{
    //    recentFileSubMenuAct->setVisible(visible);
    //    recentFileSeparator->setVisible(visible);
}

void MainWindow::updateRecentFileActions()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    const QStringList recentFiles = readRecentFiles(settings);
    const int count = qMin(int(MaxRecentFiles), recentFiles.size());
    int i = 0;
    for (; i < count; ++i) {
        const QString fileName = QFileInfo(recentFiles.at(i)).fileName();
        //        recentFileActs[i]->setText(tr("&%1 %2").arg(i + 1).arg(fileName));
        //        recentFileActs[i]->setData(recentFiles.at(i));
        //        recentFileActs[i]->setVisible(true);
    }
    for (; i < MaxRecentFiles; ++i)
        ; //        recentFileActs[i]->setVisible(false);
}

void MainWindow::openRecentFile()
{
    if (const QAction* action = qobject_cast<const QAction*>(sender()))
        openFile(action->data().toString());
}

// void MainWindow::updateWindowMenu()
//{
//     windowMenu->clear();
//     windowMenu->addAction(closeAct);
//     windowMenu->addAction(closeAllAct);
//     windowMenu->addSeparator();
//     windowMenu->addAction(tileAct);
//     windowMenu->addAction(cascadeAct);
//     windowMenu->addSeparator();
//     windowMenu->addAction(nextAct);
//     windowMenu->addAction(previousAct);
//     windowMenu->addAction(windowMenuSeparatorAct);

//    QList<QMdiSubWindow*> windows = mdiArea->subWindowList();
//    windowMenuSeparatorAct->setVisible(!windows.isEmpty());

//    for (int i = 0; i < windows.size(); ++i) {
//        QMdiSubWindow* mdiSubWindow = windows.at(i);
//        ChessForm* child = qobject_cast<ChessForm*>(mdiSubWindow->widget());

//        QString text;
//        if (i < 9) {
//            text = tr("&%1 %2").arg(i + 1).arg(child->userFriendlyCurrentFile());
//        } else {
//            text = tr("%1 %2").arg(i + 1).arg(child->userFriendlyCurrentFile());
//        }
//        QAction* action = windowMenu->addAction(text, mdiSubWindow, [this, mdiSubWindow]() {
//            mdiArea->setActiveSubWindow(mdiSubWindow);
//        });
//        action->setCheckable(true);
//        action->setChecked(child == activeChessForm());
//    }
//}

// void MainWindow::createActions()
//{
//     QMenu* recentMenu = fileMenu->addMenu(tr("Recent..."));
//     connect(recentMenu, &QMenu::aboutToShow, this, &MainWindow::updateRecentFileActions);
//     recentFileSubMenuAct = recentMenu->menuAction();

//    for (int i = 0; i < MaxRecentFiles; ++i) {
//        recentFileActs[i] = recentMenu->addAction(QString(), this, &MainWindow::openRecentFile);
//        recentFileActs[i]->setVisible(false);
//    }

//    recentFileSeparator = fileMenu->addSeparator();

//    setRecentFilesVisible(MainWindow::hasRecentFiles());
//}

ChessForm* MainWindow::activeChessForm() const
{
    if (QMdiSubWindow* activeSubWindow = ui->mdiArea->activeSubWindow())
        return qobject_cast<ChessForm*>(activeSubWindow->widget());

    return Q_NULLPTR;
}

QMdiSubWindow* MainWindow::findChessForm(const QString& fileName) const
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();

    const QList<QMdiSubWindow*> subWindows = ui->mdiArea->subWindowList();
    for (QMdiSubWindow* window : subWindows) {
        ChessForm* chessForm = qobject_cast<ChessForm*>(window->widget());
        if (chessForm->getFileName() == canonicalFilePath)
            return window;
    }
    return nullptr;
}
