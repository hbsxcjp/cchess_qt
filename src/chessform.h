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

    // 设置棋谱状态
    void on_actAllLeave_triggered();
    void on_actChangeStatus_triggered(bool checked);
    void on_actLockInstance_triggered(bool checked);

    // 局部区域右键菜单
    void on_boardGraphicsView_customContextMenuRequested(const QPoint& pos);
    void on_moveInfoTabWidget_customContextMenuRequested(const QPoint& pos);
    void on_studyTabWidget_customContextMenuRequested(const QPoint& pos);
    void on_ChessForm_customContextMenuRequested(const QPoint& pos);

    // 棋谱信息
    void on_actShowInfo_triggered();
    void on_actSaveInfo_triggered();
    // 着法注解
    void on_remarkTextEdit_textChanged();

    // 棋谱着法、信息操作
    void on_moveInfoTabWidget_currentChanged(int index);
    void on_moveTabWidget_currentChanged(int index);
    void on_pgnTypeComboBox_currentIndexChanged(int index);
    void on_actAdjustPlace_triggered();
    void on_actSaveMove_triggered();

    // 用户界面局部隐藏或显示
    void on_actLeavePiece_toggled(bool checked);
    void on_actStudy_toggled(bool checked);
    void on_actMoveInfo_toggled(bool checked);

signals:
    // 棋谱改动信号
    void insCurMoveChanged();

private:
    QMdiSubWindow* getSubWindow() const;

    void playSound(const QString& fileName);

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

    const int leftWidth { 200 }, boardWidth { 521 }, boardHeight { 577 }, boardSide { 2 };
    BoardGraphicsScene* boardScene;
    Ui::ChessForm* ui;
};

#endif // CHESSFORM_H
