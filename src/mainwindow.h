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

private slots:
    void updateActions();
    void setCurrentChessForm(QMdiSubWindow* subWindow);

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

    void on_actTileWindow_triggered();

    void on_actCascadeWindow_triggered();

private:
    ChessForm* currentChessForm;

    Ui::MainWindow* ui;
};
#endif // MAINWINDOW_H
