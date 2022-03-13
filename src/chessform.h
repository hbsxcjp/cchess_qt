#ifndef CHESSFORM_H
#define CHESSFORM_H

#include "boardgraphicsscene.h"
#include "instance.h"
#include <QGraphicsScene>
#include <QMdiSubWindow>
#include <QSettings>
#include <QWidget>

namespace Ui {
class ChessForm;
}

class ChessForm : public QWidget {
    Q_OBJECT

public:
    explicit ChessForm(QWidget* parent = Q_NULLPTR);
    ~ChessForm();

    void newFile();
    bool loadFile(const QString& fileName);
    bool save();
    bool saveAs();
    bool saveFile(const QString& fileName);

    bool needNotSave() const;
    QString getFriendlyFileName() const;
    const QString& getFileName() const { return curFileName; }

    static QString getFilter(bool isSave = false);

protected:
    void closeEvent(QCloseEvent* event) override;

    void paintEvent(QPaintEvent* event) override;

    void mousePressEvent(QMouseEvent* event) override;

private slots:
    void updateButton();

    void documentWasModified();

    void on_actStartMove_triggered();
    void on_actPreMove_triggered();
    void on_actNextMove_triggered();
    void on_actOtherMove_triggered();
    void on_actEndMove_triggered();

    void on_actLeavePiece_triggered(bool checked);
    void on_actMoveInfo_triggered(bool checked);
    void on_actStudy_triggered(bool checked);

    void on_actAllLeave_triggered();
    void on_actChangeStatus_triggered(bool checked);
    void on_actLockInstance_triggered(bool checked);

    void on_boardGraphicsView_customContextMenuRequested(const QPoint& pos);
    void on_leaveGraphicsView_customContextMenuRequested(const QPoint& pos);
    void on_infoTabWidget_customContextMenuRequested(const QPoint& pos);
    void on_studyTabWidget_customContextMenuRequested(const QPoint& pos);
    void on_ChessForm_customContextMenuRequested(const QPoint& pos);

signals:
    void instanceChanged();

private:
    QMdiSubWindow* getSubWindow() const;
    void resetSize();
    void playSound(const QString& fileName);

    void writeSettings() const;
    void readSettings();

    bool maybeSave();
    void setCurrentFile(const QString& fileName);

    bool isUntitled, isModified, moveSound;
    QString curFileName, soundDir;
    Instance* instance;

    const int leftWidth { 200 }, boardWidth { 521 }, boardHeight { 577 };
    BoardGraphicsScene* boardScene;
    Ui::ChessForm* ui;
};

#endif // CHESSFORM_H
