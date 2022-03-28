#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileSystemModel>
#include <QItemSelectionModel>
#include <QMainWindow>
#include <QMdiSubWindow>
#include <QSqlRecord>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlQueryModel>
#include <QtSql/QSqlTableModel>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class DataBase;
class ChessForm;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void on_actTest_triggered();

    // 文件菜单
    void on_actNew_triggered();
    void on_actOpen_triggered();
    void on_actSave_triggered();
    void on_actSaveAs_triggered();
    void on_actRecentFileClear_triggered();
    void on_actClose_triggered();
    void on_actCloseAll_triggered();
    void on_actExit_triggered();

    // 窗口菜单
    void on_actNextWindow_triggered();
    void on_actPreWindow_triggered();
    void on_actTabShowWindow_triggered(bool checked);
    void on_actTileWindow_triggered();
    void on_actCascadeWindow_triggered();

    void on_actAbout_triggered();

    // 更新动作按钮
    void updateMainActions();

    // 更新菜单
    void updateFileMenu();
    void updateWindowMenu();

    // 打开模型索引指定文件
    void openChessFile(const QModelIndex& index);

    // 历史文件动作
    void updateRecentFileActions();
    void openRecentFile();

    // 选项设置
    void on_actOption_triggered();

    // 目录与数据库导航
    void on_navTabWidget_currentChanged(int index);

    // 数据库查找
    void on_actClearFilter_triggered();
    void on_actSearchData_triggered();

    // 打开棋谱记录
    void openSelectedItem();

private:
    bool openTitleName(const QString& titleName);

    void writeSettings();
    void readSettings();

    void saveFile(bool isSaveAs);

    void initMenu();
    void initFileTree();
    void initInsTableModelView();
    void updateDataTable();

    void handleRecentFiles(const QString& fileName);

    ChessForm* createChessForm();
    ChessForm* getChessForm(QMdiSubWindow* subWindow) const;
    ChessForm* activeChessForm() const;
    QMdiSubWindow* findChessForm(const QString& titleName) const;

    QAction* windowMenuSeparatorAct;

    QFileSystemModel* fileModel;

    QItemSelectionModel* insItemSelModel;
    DataBase* dataBase;

    Ui::MainWindow* ui;
};

class MyFileSystemModel : public QFileSystemModel {
    using QFileSystemModel::QFileSystemModel;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};

#endif // MAINWINDOW_H
