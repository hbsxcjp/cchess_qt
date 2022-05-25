#ifndef CHESSFORM_H
#define CHESSFORM_H

#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QMdiSubWindow>
#include <QSettings>
#include <QStack>
#include <QWidget>

class Move;
class Manual;
using InfoMap = QMap<QString, QString>;

class MoveCommand;
class MoveCommandContainer;

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

private slots:
    // 更新当前着法
    void updateMoveButtonEnabled();

    // 编辑状态
    void documentWasModified();

    // 棋谱着法导航
    void on_actBackStart_triggered();
    void on_actBackInc_triggered();
    void on_actBackNext_triggered();
    void on_actBackOther_triggered();
    void on_actGoNext_triggered();
    void on_actGoOther_triggered();
    void on_actGoInc_triggered();
    void on_actGoEnd_triggered();
    void on_curMoveChanged(Move* move);

    // 设置棋谱状态
    void on_actAllLeave_triggered();
    void on_actChangeStatus_triggered(bool checked);

    // 局部区域右键菜单
    void on_boardView_customContextMenuRequested(const QPoint& pos);
    void on_moveView_customContextMenuRequested(const QPoint& pos);
    void on_studyTabWidget_customContextMenuRequested(const QPoint& pos);
    void on_ChessForm_customContextMenuRequested(const QPoint& pos);

    // 棋谱信息
    void on_actShowInfo_triggered();
    void on_actSaveInfo_triggered();
    void on_actCopyInfo_triggered();
    // 着法注解
    void on_remarkTextEdit_textChanged();

    // 棋谱着法、信息操作
    void on_moveInfoTabWidget_currentChanged(int index);
    void on_moveTabWidget_currentChanged(int index);
    void on_pgnTypeComboBox_currentIndexChanged(int index);
    void on_scopeComboBox_currentIndexChanged(int index);
    void on_actCopyPgntext_triggered();

    // 用户界面局部隐藏或显示
    void on_actLeavePiece_toggled(bool checked);
    void on_actStudy_toggled(bool checked);
    void on_actMoveInfo_toggled(bool checked);

    // 图形视图位置调整
    void on_actAlignLeft_triggered();
    void on_actAlignCenter_triggered();
    void on_actAlignRight_triggered();
    void on_actFitStart_triggered();
    void on_actFitWidth_triggered();
    void on_actFitAll_triggered();
    void on_actZoomIn_triggered();
    void on_actZoomOut_triggered();
    void on_wheelScrolled(bool isUp);
    void on_actExportMove_triggered();

signals:
    void manualModified();
    void manualMoved();

private:
    QMdiSubWindow* getSubWindow() const;

    void playSound(const QString& fileName) const;

    // 设置按钮动作
    void setBtnAction();

    // 保存和读取用户界面状态
    void writeSettings() const;
    void readSettings();

    // 辅助文件处理
    bool maybeSave();
    void setFormTitleName(const QString& titleName);

    //    void appendCommand(MoveCommand* command);
    //    template <typename ConcreteCommand>
    //    MoveCommand* createCommand();
    //    template <typename ConcreteCommand>
    //    MoveCommand* createCommand(int count);
    //    template <typename ConcreteCommand>
    //    MoveCommand* createCommand(Move* move);

    bool isUntitled;
    bool isModified;
    bool moveSound;
    QString formTitleName;
    QString soundDir;
    qreal scaleStepValue { 0.05 };

    Manual* manual;
    MoveCommandContainer* moveCommandContainer;

    Ui::ChessForm* ui;
};

#endif // CHESSFORM_H
