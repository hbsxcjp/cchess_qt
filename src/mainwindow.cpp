#include "mainwindow.h"
#include "chessform.h"
#include "instanceio.h"
#include "piece.h"
#include "publicString.h"
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
    , instanceTableModel(Q_NULLPTR)
    , insItemSelModel(Q_NULLPTR)
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

void MainWindow::updateActiveSubWindowSize(int changeWidth, int changeHeight)
{
    QMdiSubWindow* subWindow = ui->mdiArea->activeSubWindow();
    if (!subWindow)
        return;

    subWindow->resize(subWindow->width() + changeWidth, subWindow->height() + changeHeight);
}

void MainWindow::openChessFile(const QModelIndex& index)
{
    if (fileModel->isDir(index))
        return;

    // 已过滤棋谱文件
    openFile(fileModel->filePath(index));
}

void MainWindow::writeSettings()
{
    QSettings settings;
    settings.beginGroup(stringLiterals[StringIndex::MAINWINDOW]);
    QStringList activeFileNames;
    for (auto subWindow : ui->mdiArea->subWindowList())
        activeFileNames.append(getChessForm(subWindow)->getFileName());
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
    //    on_navTabWidget_currentChanged(lastIndex);

    QVariant activeFileNames = settings.value(stringLiterals[StringIndex::ACTIVEFILENAMES]);
    for (QString& fileName : activeFileNames.value<QStringList>())
        openFile(fileName);
    //    on_actNextWindow_triggered();

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
    for (auto& suffix : InstanceIO::getSuffixNames()) {
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

void MainWindow::initDataTable()
{
    QString dbname { "data.db" };
    DB = QSqlDatabase::addDatabase("QSQLITE");
    DB.setDatabaseName(dbname);
    if (!QFileInfo::exists(dbname) || !DB.open()) {
        QMessageBox::warning(this, "打开文件错误", "没有找到文件：" + dbname, QMessageBox::Ok);
        return;
    }

    // 模型和视图
    instanceTableModel = new QSqlTableModel(this);
    insItemSelModel = new QItemSelectionModel(instanceTableModel);
    instanceTableModel->setTable("manual");
    ui->dataTableView->setModel(instanceTableModel);
    ui->dataTableView->setSelectionModel(insItemSelModel);
    QMap<InfoIndex, QPair<int, QString>> showFields {
        { InfoIndex::TITLE, { 300, "棋局标题" } },
        { InfoIndex::EVENT, { 200, "赛事名称" } },
        { InfoIndex::DATE, { 110, "日期" } },
        //        { InfoIndex::SITE, {150,"地点" }},
        { InfoIndex::BLACK, { 120, "黑方" } },
        { InfoIndex::RED, { 120, "红方" } },
        { InfoIndex::RESULT, { 60, "结果" } },
        { InfoIndex::ECCOSN, { 70, "开局编号" } },
        { InfoIndex::ECCONAME, { 300, "开局名称" } },
    };
    int fieldNum = InstanceIO::getAllInfoName().size() + 1;
    for (int fieldIndex = 0; fieldIndex < fieldNum; ++fieldIndex) {
        InfoIndex showField = InfoIndex(fieldIndex - 1);
        if (showFields.contains(showField)) {
            auto& widthTitle = showFields[showField];
            ui->dataTableView->setColumnWidth(fieldIndex, widthTitle.first);
            instanceTableModel->setHeaderData(fieldIndex, Qt::Horizontal, widthTitle.second);
        } else
            ui->dataTableView->hideColumn(fieldIndex);
    }
    instanceTableModel->setSort(0, Qt::SortOrder::AscendingOrder);
    instanceTableModel->setEditStrategy(QSqlTableModel::EditStrategy::OnFieldChange);

    ui->btnClearFilter->setDefaultAction(ui->actClearFilter);
    ui->btnSearchData->setDefaultAction(ui->actSearchData);
    //    ui->dataTableView->addAction(ui->actionCopy);
    //    connect(insItemSelModel, &QItemSelectionModel::selectionChanged,
    //        this, &MAINWINDOW::);
}

void MainWindow::updateDataTable()
{
    QStringList sql {
        //        "project_id " + Common::getSelectionIdFilter(proTableModel, proItemSelModel),
        //        Common::getKeysFilter(ui->empLineEdit->text(), "\\W+", "empName"),
        //        Common::getKeysFilter(ui->telLineEdit->text(), "\\D+", "telephone")
    };
    //    printf((sql + '\n').toUtf8());

    //    instanceTableModel->setFilter(sql.join("AND "));
    instanceTableModel->select();
    //    ui->empTableView->resizeColumnsToContents();
}

void MainWindow::handleRecentFiles(const QString& fileName)
{
    QSettings settings;
    const QStringList oldRecentFiles = Tools::readStringList(settings,
        stringLiterals[StringIndex::RECENTFILELIST],
        stringLiterals[StringIndex::FILEKEY]);
    QStringList recentFiles = oldRecentFiles;

    if (fileName.isEmpty()) {
        for (QString& recentFile : recentFiles)
            if (!findChessForm(recentFile))
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
    const QStringList recentFiles = Tools::readStringList(settings, stringLiterals[StringIndex::RECENTFILELIST],
        stringLiterals[StringIndex::FILEKEY]);
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
    QMdiSubWindow* subWindow = ui->mdiArea->addSubWindow(new ChessForm);
    ChessForm* chessForm = getChessForm(subWindow);

    subWindow->setAttribute(Qt::WA_DeleteOnClose);
    subWindow->setSystemMenu(Q_NULLPTR);
    subWindow->layout()->setSizeConstraint(QLayout::SetMinimumSize);
    //    subWindow->setWindowFlags(Qt::Dialog);

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
        if (!instanceTableModel)
            initDataTable();

        updateDataTable();
    }
}

void MainWindow::on_actClearFilter_triggered()
{
    ui->startDateEdit->setDate(QDate(1970, 1, 1));
    ui->endDateEdit->setDate(QDate(2030, 1, 1));
    ui->eventLineEdit->clear();
    ui->siteLineEdit->clear();
    ui->eccoSNLineEdit->clear();
    ui->eccoNameLineEdit->clear();
    ui->personLineEdit->clear();
    ui->colorComboBox->setCurrentIndex(0);
    ui->resultComboBox->setCurrentIndex(0);
}

void MainWindow::on_actSearchData_triggered()
{
    updateDataTable();
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
