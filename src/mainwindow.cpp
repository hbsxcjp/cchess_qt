#include "mainwindow.h"
#include "common.h"
#include "database.h"
#include "manualIO.h"
#include "manualsubwindow.h"
#include "piece.h"
#include "test.h"
#include "tools.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>

enum {
    FileTree_Name,
    FileTree_Size,
    FileTree_Type,
    FileTree_Date,
};

static const int actUsersTag { 1 };

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , windowMenuSeparatorAct(Q_NULLPTR)
    , fileModel(Q_NULLPTR)
    , insItemSelModel(Q_NULLPTR)
    , dataBase(new DataBase)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initMenu();

    readSettings();
    updateMainActions();
    setWindowTitle(stringLiterals.at(StringIndex::WINDOWTITLE));
}

MainWindow::~MainWindow()
{
    delete dataBase;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    writeSettings();
    on_actCloseAll_triggered();
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

    TestManual tins;
    QTest::qExec(&tins);

    TestAspect tasp;
    QTest::qExec(&tasp);

    TestInitEcco tecco;
    QTest::qExec(&tecco); // , { "-o output/testOutput.txt txt" }
}

void MainWindow::on_actNew_triggered()
{
    ManualSubWindow* manualSubWindow = createManualSubWindow();
    manualSubWindow->newFile();
    manualSubWindow->show();
}

void MainWindow::on_actOpen_triggered()
{
    const QString fileName = QFileDialog::getOpenFileName(this, "打开棋谱", "./", ManualSubWindow::getFilter());
    if (!fileName.isEmpty())
        openTitleName(fileName);
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
    QMdiSubWindow* subWindow = ui->mdiArea->activeSubWindow();
    if (subWindow)
        subWindow->close();
}

void MainWindow::on_actCloseAll_triggered()
{
    for (auto subWindow : ui->mdiArea->subWindowList())
        subWindow->close();
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
    Tools::messageBox("关于",
        "一个象棋打谱的软件，\t\n含有大量实战棋谱，欢迎使用！\t\n\ncjp\n2022.12.31\n",
        "关闭");
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
    const int count = settings.beginReadArray(stringLiterals.at(StringIndex::RECENTFILELIST));
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
        ManualSubWindow* manualSubWindow = getManualSubWindow(subWindow);
        QAction* action = ui->menuWindow->addAction(manualSubWindow->getFriendlyFileName(), subWindow,
            [this, subWindow]() {
                ui->mdiArea->setActiveSubWindow(subWindow);
            });
        action->setCheckable(true);
        action->setChecked(subWindow == ui->mdiArea->activeSubWindow());
        action->setData(actUsersTag);
    }
}

void MainWindow::openChessFile(const QModelIndex& index)
{
    if (fileModel->isDir(index))
        return;

    // 已过滤棋谱文件
    openTitleName(fileModel->filePath(index));
}

void MainWindow::on_actOption_triggered()
{
    QSettings settings;
    // 清除全部设置内容
    settings.clear();
}

void MainWindow::on_navTabWidget_currentChanged(int index)
{
    if (index == 0) {
        if (!fileModel)
            initFileTree();
    } else if (index == 1) {
        if (!insItemSelModel)
            initInsTableModelView();

        on_actSearchData_triggered();
    }
}

void MainWindow::on_actClearFilter_triggered()
{
    ui->startDateEdit->setDate(QDate(1970, 1, 1));
    ui->endDateEdit->setDate(QDate(2030, 1, 1));
    ui->titleLineEdit->clear();
    ui->eventLineEdit->clear();
    ui->siteLineEdit->clear();
    ui->eccoSnLineEdit->clear();
    ui->eccoNameLineEdit->clear();
    ui->personLineEdit->clear();
    ui->colorComboBox->setCurrentIndex(0);
    ui->resultComboBox->setCurrentIndex(0);

    on_actSearchData_triggered();
}

void MainWindow::on_actSearchData_triggered()
{
    dataBase->updateInsTableModel(ui->startDateEdit->date(), ui->endDateEdit->date(),
        ui->titleLineEdit->text(), ui->eventLineEdit->text(), ui->siteLineEdit->text(),
        ui->eccoSnLineEdit->text(), ui->eccoNameLineEdit->text(), ui->resultComboBox->currentText(),
        ui->personLineEdit->text(), ui->colorComboBox->currentIndex());
}

void MainWindow::on_sourceComboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    on_actSearchData_triggered();
}

void MainWindow::on_titleLineEdit_textChanged(const QString& arg1)
{
    Q_UNUSED(arg1)
    on_actSearchData_triggered();
}

void MainWindow::on_eventLineEdit_textChanged(const QString& arg1)
{
    Q_UNUSED(arg1)
    on_actSearchData_triggered();
}

void MainWindow::on_siteLineEdit_textChanged(const QString& arg1)
{
    Q_UNUSED(arg1)
    on_actSearchData_triggered();
}

void MainWindow::on_startDateEdit_userDateChanged(const QDate& date)
{
    Q_UNUSED(date)
    on_actSearchData_triggered();
}

void MainWindow::on_endDateEdit_userDateChanged(const QDate& date)
{
    Q_UNUSED(date)
    on_actSearchData_triggered();
}

void MainWindow::on_eccoSnLineEdit_textChanged(const QString& arg1)
{
    Q_UNUSED(arg1)
    on_actSearchData_triggered();
}

void MainWindow::on_eccoNameLineEdit_textChanged(const QString& arg1)
{
    Q_UNUSED(arg1)
    on_actSearchData_triggered();
}

void MainWindow::on_personLineEdit_textChanged(const QString& arg1)
{
    Q_UNUSED(arg1)
    on_actSearchData_triggered();
}

void MainWindow::on_colorComboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    on_actSearchData_triggered();
}

void MainWindow::on_resultComboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    on_actSearchData_triggered();
}

void MainWindow::openSelectedItem()
{
    QString titleName { dataBase->getTitleName(insItemSelModel) };
    if (!titleName.isEmpty())
        openTitleName(titleName);
}

bool MainWindow::openTitleName(const QString& titleName)
{
    if (QMdiSubWindow* existing = findSubWindow(titleName)) {
        ui->mdiArea->setActiveSubWindow(existing);
        return true;
    }

    InfoMap infoMap;
    ManualSubWindow* manualSubWindow = createManualSubWindow();
    if (!QFileInfo::exists(titleName))
        infoMap = dataBase->getInfoMap(titleName);

    bool succeeded = manualSubWindow->loadTitleName(titleName, infoMap);

    if (succeeded) {
        manualSubWindow->show();
        handleRecentFiles(titleName);
        statusBar()->showMessage(QString("棋谱加载完成: %1").arg(titleName), 5000);
    } else
        manualSubWindow->parentWidget()->close();

    return succeeded;
}

void MainWindow::writeSettings()
{
    QSettings settings;
    settings.beginGroup(stringLiterals[StringIndex::MAINWINDOW]);
    QStringList activeFileNames;
    for (auto subWindow : ui->mdiArea->subWindowList())
        activeFileNames.append(getManualSubWindow(subWindow)->getTitleName());
    settings.setValue(stringLiterals[StringIndex::ACTIVEFILENAMES], activeFileNames);

    settings.setValue(stringLiterals[StringIndex::GEOMETRY], saveGeometry());
    settings.setValue(stringLiterals[StringIndex::VIEWMODE], ui->actTabShowWindow->isChecked());
    settings.setValue(stringLiterals[StringIndex::SPLITTER], ui->splitter->saveState());
    settings.setValue(stringLiterals[StringIndex::NAVINDEX], ui->navTabWidget->currentIndex());

    settings.endGroup();
}

void MainWindow::readSettings()
{
    QSettings settings;
    settings.beginGroup(stringLiterals[StringIndex::MAINWINDOW]);

    restoreGeometry(settings.value(stringLiterals[StringIndex::GEOMETRY]).toByteArray());
    on_actTabShowWindow_triggered(settings.value(stringLiterals[StringIndex::VIEWMODE]).toBool());
    ui->splitter->restoreState(settings.value(stringLiterals[StringIndex::SPLITTER]).toByteArray());
    int lastIndex = settings.value(stringLiterals[StringIndex::NAVINDEX]).toInt();
    ui->navTabWidget->setCurrentIndex(lastIndex);
    ui->navTabWidget->currentChanged(lastIndex);

    QVariant activeFileNames = settings.value(stringLiterals[StringIndex::ACTIVEFILENAMES]);
    for (QString& titleName : activeFileNames.value<QStringList>())
        openTitleName(titleName);
    //    on_actNextWindow_triggered();

    settings.endGroup();
}

void MainWindow::saveFile(bool isSaveAs)
{
    ManualSubWindow* manualSubWindow = activeManualSubWindow();
    if (!manualSubWindow)
        return;

    if (isSaveAs ? manualSubWindow->saveAs() : manualSubWindow->save()) {
        statusBar()->showMessage("文件已保存.", 2000);
        handleRecentFiles(manualSubWindow->getTitleName());
    }
}

void MainWindow::initMenu()
{
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
}

void MainWindow::initFileTree()
{
    fileModel = new MyFileSystemModel(this);
    fileModel->setRootPath(QDir::currentPath());
    QStringList nameFilter;
    for (auto& suffix : ManualIO::getSuffixNames()) {
        nameFilter.append("*." + suffix);
        nameFilter.append("*." + suffix.toUpper());
    }
    fileModel->setNameFilters(nameFilter);
    fileModel->setNameFilterDisables(false);

    ui->fileTreeView->setModel(fileModel);
    ui->fileTreeView->setRootIndex(fileModel->index(QDir::currentPath()));
    ui->fileTreeView->setColumnWidth(FileTree_Name, 300);
    ui->fileTreeView->setColumnWidth(FileTree_Size, 80);
    ui->fileTreeView->setColumnWidth(FileTree_Type, 100);
    ui->fileTreeView->setColumnWidth(FileTree_Date, 150);
    //        ui->fileTreeView->setColumnHidden(FileTree_Type, true);
    connect(ui->fileTreeView, &QTreeView::clicked, this, &MainWindow::openChessFile);
}

void MainWindow::initInsTableModelView()
{
    dataBase->initInsTableModelView(ui->dataTableView, insItemSelModel);
    connect(insItemSelModel, &QItemSelectionModel::selectionChanged,
        this, &MainWindow::openSelectedItem);
    connect(ui->dataTableView, &QTableView::entered,
        this, &MainWindow::openSelectedItem);

    ui->btnClearFilter->setDefaultAction(ui->actClearFilter);
}

void MainWindow::handleRecentFiles(const QString& fileName)
{
    QSettings settings;
    QStringList oldRecentFiles = Tools::readStringList(settings,
                    stringLiterals[StringIndex::RECENTFILELIST],
                    stringLiterals[StringIndex::FILEKEY]),
                recentFiles { oldRecentFiles };

    if (fileName.isEmpty()) {
        for (QString& recentFile : recentFiles)
            if (!findSubWindow(recentFile))
                recentFiles.removeAll(recentFile);
    } else {
        recentFiles.removeAll(fileName);
        recentFiles.prepend(fileName);
    }

    if (oldRecentFiles != recentFiles)
        Tools::writeStringList(recentFiles, settings,
            stringLiterals[StringIndex::RECENTFILELIST],
            stringLiterals[StringIndex::FILEKEY]);

    ui->menuRecent->setEnabled(!recentFiles.isEmpty());
}

void MainWindow::updateRecentFileActions()
{
    QSettings settings;
    const QStringList recentFiles = Tools::readStringList(settings,
        stringLiterals[StringIndex::RECENTFILELIST],
        stringLiterals[StringIndex::FILEKEY]);
    QList<QAction*> recentActs = ui->menuRecent->actions();
    const int actCount = recentActs.size() - 2, // 排除最后的分隔符和清除按钮
        count = qMin(actCount, recentFiles.size());
    int i = 0;
    for (; i < count; ++i) {
        const QString fileName = QFileInfo(recentFiles.at(i)).fileName();
        QAction* action = recentActs.at(i);
        action->setText(QString("&%1. %2").arg(i + 1).arg(fileName));
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
        openTitleName(action->data().toString());
}

ManualSubWindow* MainWindow::createManualSubWindow()
{
    QMdiSubWindow* subWindow = ui->mdiArea->addSubWindow(new ManualSubWindow);
    ManualSubWindow* manualSubWindow = getManualSubWindow(subWindow);

    subWindow->setAttribute(Qt::WA_DeleteOnClose);
    subWindow->setSystemMenu(Q_NULLPTR);
    subWindow->layout()->setSizeConstraint(QLayout::SetMinimumSize);
    //    subWindow->setWindowFlags(Qt::Dialog);

    return manualSubWindow;
}

ManualSubWindow* MainWindow::getManualSubWindow(QMdiSubWindow* subWindow) const
{
    return qobject_cast<ManualSubWindow*>(subWindow->widget());
}

ManualSubWindow* MainWindow::activeManualSubWindow() const
{
    if (QMdiSubWindow* subWindow = ui->mdiArea->activeSubWindow())
        return getManualSubWindow(subWindow);

    return Q_NULLPTR;
}

QMdiSubWindow* MainWindow::findSubWindow(const QString& titleName) const
{
    for (QMdiSubWindow* subWindow : ui->mdiArea->subWindowList()) {
        if (getManualSubWindow(subWindow)->getTitleName() == titleName)
            return subWindow;
    }

    return Q_NULLPTR;
}

QVariant MyFileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role) {
    case Qt::DecorationRole:
        break;
    case Qt::TextAlignmentRole:
        return Qt::AlignLeft;
    }

    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QAbstractItemModel::headerData(section, orientation, role);

    QString returnValue;
    switch (section) {
    case FileTree_Name:
        returnValue = "名称";
        break;
    case FileTree_Size:
        returnValue = "大小";
        break;
    case FileTree_Type:
        returnValue = "类型";
        break;
    case FileTree_Date:
        returnValue = "修改日期";
        break;
    default:
        return QVariant();
    }
    return returnValue;
}
