#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "chessform.h"
#include <QMainWindow>
#include <QMdiSubWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    bool openFile(const QString& fileName);

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void on_actTest_triggered();

    void on_actNew_triggered();
    void on_actOpen_triggered();
    void on_actSave_triggered();
    void on_actSaveAs_triggered();

    void on_actClose_triggered();
    void on_actCloseAll_triggered();
    void on_actExit_triggered();

    void on_actAbout_triggered();

    void on_actNextWindow_triggered();
    void on_actPreWindow_triggered();
    void on_actTabShowWindow_triggered(bool checked);
    void on_actTileWindow_triggered();
    void on_actCascadeWindow_triggered();

    void updateActions();
    void updateWindowMenu();

    void updateRecentFileActions();
    void openRecentFile();

    ChessForm* createChessForm();

private:
    enum { MaxRecentFiles = 10 };

    void writeSettings();
    void readSettings();
    bool loadFile(const QString& fileName);

    void createActions();
    static bool hasRecentFiles();
    void prependToRecentFiles(const QString& fileName);
    void setRecentFilesVisible(bool visible);

    ChessForm* activeChessForm() const;
    QMdiSubWindow* findChessForm(const QString& fileName) const;

    QAction* recentFileActs[MaxRecentFiles];
    QAction* recentFileSeparator;
    QAction* recentFileSubMenuAct;
    QAction* windowMenuSeparatorAct;
    Ui::MainWindow* ui;
};
#endif // MAINWINDOW_H
