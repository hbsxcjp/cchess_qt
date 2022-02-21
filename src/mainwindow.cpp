#include "mainwindow.h"
#include "instanceio.h"
#include "piece.h"
#include "test.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>

static const QString programName { "学象棋" };
static const QStringList settingNames {
    "mainWindow", "geometry", "viewmode", "splitter", "navIndex",
    "subWindow", "source", "locate"
};

enum SettingNameIndex {
    MAINWINDOW,
    GEOMETRY,
    VIEWMODE,
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

    createActions();
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
    chessForm->show();
}

void MainWindow::on_actOpen_triggered()
{
    const QString fileName = QFileDialog::getOpenFileName(this, "打开棋谱",
        "./", QString("棋谱文件(%1)").arg(InstanceIO::getSuffixNames().join(' ')));
    if (!fileName.isEmpty())
        openFile(fileName);
}

void MainWindow::on_actSave_triggered()
{
    ChessForm* chessForm = activeChessForm();
    if (chessForm && chessForm->save())
        statusBar()->showMessage("文件已保存.", 2000);
}

void MainWindow::on_actSaveAs_triggered()
{
    ChessForm* chessForm = activeChessForm();
    if (chessForm && chessForm->saveAs()) {
        statusBar()->showMessage("文件已保存.", 2000);
        MainWindow::prependToRecentFiles(chessForm->getFileName());
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
    QMessageBox::about(this, "关于",
        "一个学习象棋、可以打谱，\n并且具有大量实战棋谱的软件.\n\ncjp 2022.12.31\n");
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

void MainWindow::updateActions()
{
    bool hasActivedSubWindow = bool(ui->mdiArea->activeSubWindow());
    ui->actSave->setEnabled(hasActivedSubWindow);
    ui->actSaveAs->setEnabled(hasActivedSubWindow);
    ui->actClose->setEnabled(hasActivedSubWindow);
    ui->actCloseAll->setEnabled(hasActivedSubWindow);
}

ChessForm* MainWindow::createChessForm()
{
    ChessForm* chessForm = new ChessForm;
    QMdiSubWindow* subWindow = ui->mdiArea->addSubWindow(chessForm);
    subWindow->setSystemMenu(Q_NULLPTR);

    //#ifndef QT_NO_CLIPBOARD
    //    connect(chessForm, &QTextEdit::copyAvailable, cutAct, &QAction::setEnabled);
    //    connect(chessForm, &QTextEdit::copyAvailable, copyAct, &QAction::setEnabled);
    //#endif

    return chessForm;
}

void MainWindow::writeSettings()
{
    QSettings settings;
    settings.beginGroup(settingNames[SettingNameIndex::MAINWINDOW]);
    settings.setValue(settingNames[SettingNameIndex::GEOMETRY], saveGeometry());
    settings.setValue(settingNames[SettingNameIndex::VIEWMODE], ui->actTabShowWindow->isChecked());
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
    bool tabShowIsChecked = settings.value(settingNames[SettingNameIndex::VIEWMODE]).toBool();
    on_actTabShowWindow_triggered(tabShowIsChecked);
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
    ChessForm* chessForm = createChessForm();
    const bool succeeded = chessForm->loadFile(fileName);
    if (succeeded)
        chessForm->show();
    else
        chessForm->close();

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
    QSettings settings;
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
    recentFileSubMenuAct->setVisible(visible);
    recentFileSeparator->setVisible(visible);
}

void MainWindow::updateRecentFileActions()
{
    QSettings settings;
    const QStringList recentFiles = readRecentFiles(settings);
    const int count = qMin(int(MaxRecentFiles), recentFiles.size());
    int i = 0;
    for (; i < count; ++i) {
        const QString fileName = QFileInfo(recentFiles.at(i)).fileName();
        recentFileActs[i]->setText(tr("&%1 %2").arg(i + 1).arg(fileName));
        recentFileActs[i]->setData(recentFiles.at(i));
        recentFileActs[i]->setVisible(true);
    }
    for (; i < MaxRecentFiles; ++i)
        recentFileActs[i]->setVisible(false);
}

void MainWindow::openRecentFile()
{
    if (const QAction* action = qobject_cast<const QAction*>(sender()))
        openFile(action->data().toString());
}

void MainWindow::updateWindowMenu()
{
    ui->menuWindow->addAction(windowMenuSeparatorAct);

    QList<QMdiSubWindow*> windows = ui->mdiArea->subWindowList();
    windowMenuSeparatorAct->setVisible(!windows.isEmpty());

    for (int i = 0; i < windows.size(); ++i) {
        QMdiSubWindow* mdiSubWindow = windows.at(i);
        ChessForm* chessForm = qobject_cast<ChessForm*>(mdiSubWindow->widget());

        QString text;
        if (i < 9) {
            text = tr("&%1 %2").arg(i + 1).arg(chessForm->userFriendlyCurrentFile());
        } else {
            text = tr("%1 %2").arg(i + 1).arg(chessForm->userFriendlyCurrentFile());
        }
        QAction* action = ui->menuWindow->addAction(text, mdiSubWindow, [this, mdiSubWindow]() {
            ui->mdiArea->setActiveSubWindow(mdiSubWindow);
        });
        action->setCheckable(true);
        action->setChecked(chessForm == activeChessForm());
    }
}

void MainWindow::createActions()
{
    QMenu* recentMenu = ui->menuFile->addMenu("最近...");
    connect(recentMenu, &QMenu::aboutToShow, this, &MainWindow::updateRecentFileActions);
    recentFileSubMenuAct = recentMenu->menuAction();

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActs[i] = recentMenu->addAction(QString(), this, &MainWindow::openRecentFile);
        recentFileActs[i]->setVisible(false);
    }

    windowMenuSeparatorAct = new QAction(this);
    windowMenuSeparatorAct->setSeparator(true);

    recentFileSeparator = ui->menuFile->addSeparator();

    setRecentFilesVisible(MainWindow::hasRecentFiles());
}

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

    return Q_NULLPTR;
}
