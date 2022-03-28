#ifndef CHESSFORM_H
#define CHESSFORM_H

#include <QGraphicsScene>
#include <QMdiSubWindow>
#include <QSettings>
#include <QWidget>

class Instance;
class BoardGraphicsScene;
using InfoMap = QMap<QString, QString>;

namespace Ui {
class ChessForm;
}

class ChessForm : public QWidget {
    Q_OBJECT

public:
    explicit ChessForm(QWidget* parent = Q_NULLPTR);
    ~ChessForm();

    // 文件处理
    void newFile();
    bool save();
    bool saveAs();
    bool saveFile(const QString& fileName);
    bool needNotSave() const;
    QString getFriendlyFileName() const;

    const QString& getTitleName() const { return formTitleName; }
    static QString getFilter(bool isSave = false);

    // 文件或记录处理
    bool loadTitleName(const QString& titleName, const InfoMap& infoMap);

protected:
    void closeEvent(QCloseEvent* event) override;

    void paintEvent(QPaintEvent* event) override;

    void mousePressEvent(QMouseEvent* event) override;

private slots:
    // 更新当前着法
    void updateInsCurMoveGUI();

    // 棋谱编辑状态
    void documentWasModified();

    // 棋谱着法导航
    void on_actStartMove_triggered();
    void on_actPreMove_triggered();
    void on_actNextMove_triggered();
    void on_actOtherMove_triggered();
    void on_actEndMove_triggered();

    // 用户界面局部隐藏或显示
    void on_actLeavePiece_triggered(bool checked);
    void on_actMoveInfo_triggered(bool checked);
    void on_actStudy_triggered(bool checked);

    // 设置棋谱状态
    void on_actAllLeave_triggered();
    void on_actChangeStatus_triggered(bool checked);
    void on_actLockInstance_triggered(bool checked);

    // 局部区域右键菜单
    void on_boardGraphicsView_customContextMenuRequested(const QPoint& pos);
    void on_leaveGraphicsView_customContextMenuRequested(const QPoint& pos);
    void on_infoTabWidget_customContextMenuRequested(const QPoint& pos);
    void on_studyTabWidget_customContextMenuRequested(const QPoint& pos);
    void on_ChessForm_customContextMenuRequested(const QPoint& pos);

    // 棋谱信息
    void on_actShowInfo_triggered();
    void on_actSaveInfo_triggered();
    // 着法注解
    void on_remarkTextEdit_textChanged();

signals:
    // 棋谱改动信号
    void insCurMoveChanged();

private:
    QMdiSubWindow* getSubWindow() const;

    // 显示、保存棋谱信息
    void showInfo();
    void saveInfo();

    // 辅助局部区域隐藏或显示
    void resetSize();
    void playSound(const QString& fileName);

    // 初始化视图和场景
    void initViewScene();
    // 设置按钮动作
    void setBtnAction();

    // 保存和读取用户界面状态
    void writeSettings() const;
    void readSettings();

    // 辅助文件处理
    bool maybeSave();
    void setFormTitleName(const QString& titleName);

    bool isUntitled, isModified, moveSound;
    QString formTitleName, soundDir;
    Instance* instance;

    const int leftWidth { 200 }, boardWidth { 521 }, boardHeight { 577 };
    BoardGraphicsScene* boardScene;
    Ui::ChessForm* ui;
};

#endif // CHESSFORM_H
