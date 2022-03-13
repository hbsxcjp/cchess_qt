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
    FileTree_Type,
    FileTree_Date,
    FileTree_Size,
};

static const int actUsersTag { 1 };

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , windowMenuSeparatorAct(new QAction(this))
    , fileModel(new QFileSystemModel(this))
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

    windowMenuSeparatorAct->setSeparator(true);
    ui->menuWindow->addAction(windowMenuSeparatorAct);
    ui->actRecentFileClear->setData(actUsersTag);
    updateMainActions();

    setWindowTitle(stringLiterals.at(StringIndex::WINDOWTITLE));

    initFileTree();
    readSettings();
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
    ui->navTabWidget->setCurrentIndex(settings.value(stringLiterals[StringIndex::NAVINDEX]).toInt());

    QVariant activeFileNames = settings.value(stringLiterals[StringIndex::ACTIVEFILENAMES]);
    for (QString& fileName : activeFileNames.value<QStringList>())
        openFile(fileName);

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

void MainWindow::initFileTree()
{
    fileModel->setRootPath(QDir::currentPath());
    fileModel->setHeaderData(FileTree_Name, Qt::Horizontal, "名称");
    fileModel->setHeaderData(FileTree_Type, Qt::Horizontal, "类型");
    fileModel->setHeaderData(FileTree_Date, Qt::Horizontal, "日期");
    fileModel->setHeaderData(FileTree_Size, Qt::Horizontal, "大小");

    QStringList nameFilter;
    for (auto& suffix : InstanceIO::getSuffixNames()) {
        nameFilter.append("*." + suffix);
        nameFilter.append("*." + suffix.toUpper());
    }
    fileModel->setNameFilters(nameFilter);
    fileModel->setNameFilterDisables(false);

    ui->fileTreeView->setModel(fileModel);
    ui->fileTreeView->setRootIndex(fileModel->index(QDir::currentPath()));
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

    // 公司模型和视图
    comTableModel = new QSqlTableModel(this);
    comItemSelModel = new QItemSelectionModel(comTableModel);
    comTableModel->setTable("company");
    comTableModel->setFilter("end_date IS NULL");
    comTableModel->setSort(Company_Sort_Id, Qt::SortOrder::AscendingOrder);
    comTableModel->setHeaderData(Company_Name, Qt::Horizontal, "公司");
    comTableModel->setEditStrategy(QSqlTableModel::EditStrategy::OnFieldChange);
    ui->comTableView->setModel(comTableModel);
    ui->comTableView->setSelectionModel(comItemSelModel);
    ui->comTableView->hideColumn(Company_Id);
    ui->comTableView->hideColumn(Company_Sort_Id);
    ui->comTableView->hideColumn(Company_Start_Date);
    ui->comTableView->hideColumn(Company_End_Date);
    ui->comTableView->addAction(ui->actionCopy);
    connect(comItemSelModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this, SLOT(on_comItemSelectionChanged()));

    // 项目部模型和视图
    proTableModel = new QSqlRelationalTableModel(this);
    proItemSelModel = new QItemSelectionModel(proTableModel);
    proTableModel->setTable("project");
    proTableModel->setSort(Project_Sort_Id, Qt::SortOrder::AscendingOrder);
    proTableModel->setRelation(Project_Company_Id, QSqlRelation("company", "id", "comName"));
    proTableModel->setHeaderData(Project_Company_Id, Qt::Horizontal, "公司");
    proTableModel->setHeaderData(Project_Name, Qt::Horizontal, "项目部/机关");
    proTableModel->setEditStrategy(QSqlTableModel::EditStrategy::OnFieldChange);
    ui->proTableView->setModel(proTableModel);
    ui->proTableView->setSelectionModel(proItemSelModel);
    ui->proTableView->setItemDelegate(new QSqlRelationalDelegate(ui->proTableView));
    ui->proTableView->hideColumn(Project_Id);
    ui->proTableView->hideColumn(Project_Sort_Id);
    ui->proTableView->hideColumn(Project_Start_Date);
    ui->proTableView->hideColumn(Project_End_Date);
    ui->proTableView->hideColumn(Project_AtWork);
    ui->proTableView->addAction(ui->actionCopy);
    connect(proItemSelModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this, SLOT(on_proItemSelectionChanged()));

    // 人员模型和视图
    empTableModel = new QSqlRelationalTableModel(this);
    empItemSelModel = new QItemSelectionModel(empTableModel);
    empTableModel->setTable("employee");
    empTableModel->setSort(Employee_Sort_Id, Qt::SortOrder::AscendingOrder);
    empTableModel->setRelation(Employee_Project_Id, QSqlRelation("project", "id", "proName"));
    empTableModel->setRelation(Employee_Role_Id, QSqlRelation("role", "id", "rolName"));
    empTableModel->setHeaderData(Employee_Project_Id, Qt::Horizontal, "项目/机关");
    empTableModel->setHeaderData(Employee_Role_Id, Qt::Horizontal, "小组职务");
    empTableModel->setHeaderData(Employee_Name, Qt::Horizontal, "姓名");
    empTableModel->setHeaderData(Employee_Depart_Position, Qt::Horizontal, "部门/职务");
    empTableModel->setHeaderData(Employee_Telephone, Qt::Horizontal, "电话");
    empTableModel->setEditStrategy(QSqlTableModel::EditStrategy::OnFieldChange);
    ui->empTableView->setModel(empTableModel);
    ui->empTableView->setSelectionModel(empItemSelModel);
    ui->empTableView->setItemDelegate(new QSqlRelationalDelegate(ui->empTableView));
    ui->empTableView->hideColumn(Employee_Id);
    ui->empTableView->hideColumn(Employee_Sort_Id);
    ui->empTableView->hideColumn(Employee_Start_Date);
    ui->empTableView->addAction(ui->actionCopy);
    connect(empItemSelModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this, SLOT(on_empItemSelectionChanged()));

    // 代码创建其他界面组件
    copyComboBox = new QComboBox(this);
    copyComboBox->addItem("表格  ");
    copyComboBox->addItem("树状  ");
    ui->toolBar->addWidget(new QLabel("输出格式：", this));
    ui->toolBar->addWidget(copyComboBox);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionAbout);
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
