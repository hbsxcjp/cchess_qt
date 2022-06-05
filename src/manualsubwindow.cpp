#include "manualsubwindow.h"
#include "boardscene.h"
#include "boardview.h"
#include "command.h"
#include "common.h"
#include "manual.h"
#include "manualIO.h"
#include "manualmove.h"
#include "move.h"
#include "moveitem.h"
#include "moveview.h"
#include "tools.h"
#include "ui_manualsubwindow.h"

#include <QClipboard>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMdiArea>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QScrollBar>
#include <QSound>
#include <QTimer>

static const int MoveCount { 5 };

static const QStringList StateStrings { "布局", "打谱", "演示" };

ManualSubWindow::ManualSubWindow(QWidget* parent)
    : QWidget(parent)
    , isUntitled(true)
    , isModified(false)
    , titleName_(QString())
    , state_(SubWinState::NOTSTATE)
    , manual_(new Manual)
    , commandContainer_(new CommandContainer)
    , ui(new Ui::ManualSubWindow)
{
    ui->setupUi(this);
    connect(this, &ManualSubWindow::manualMoveModified, this, &ManualSubWindow::manualModified);
    connect(this, &ManualSubWindow::manualMoveModified, this, &ManualSubWindow::manualMoveOpened);

    connect(this, &ManualSubWindow::manualMoveOpened, ui->moveView, &MoveView::resetMoveNodeItems);
    connect(this, &ManualSubWindow::manualMoveOpened, this, &ManualSubWindow::manualMoveWalked);

    connect(this, &ManualSubWindow::manualMoveWalked, this, &ManualSubWindow::updateMoveActionState);
    connect(this, &ManualSubWindow::manualMoveWalked, ui->boardView, &BoardView::updateShowPieceItem);
    connect(this, &ManualSubWindow::manualMoveWalked, ui->moveView, &MoveView::updateSelectedNodeItem);

    connect(ui->moveView, &MoveView::mousePressed, this, &ManualSubWindow::on_curMoveChanged);
    connect(ui->moveView, &MoveView::wheelScrolled, this, &ManualSubWindow::on_wheelScrolled);

    ui->boardView->setManualSubWindow(this);
    ui->moveView->setManualSubWindow(this);

    setBtnAction();
}

ManualSubWindow::~ManualSubWindow()
{
    delete commandContainer_;
    delete manual_;
    delete ui;
}

void ManualSubWindow::newFile()
{
    static int sequenceNumber = 1;
    isUntitled = true;
    titleName_ = (QString("未命名%2.%3")
                      .arg(sequenceNumber++)
                      .arg(ManualIO::getSuffixName(StoreType::PGN_ZH)));
    setWindowTitle(titleName_ + "[*]");
    playSound("NEWGAME.WAV");

    setState(SubWinState::PLAY);
    emit manualMoveOpened();
}

bool ManualSubWindow::save()
{
    if (isUntitled)
        return saveAs();
    else
        return saveFile(titleName_);
}

bool ManualSubWindow::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, "另存为", "./", getFilter(true));
    if (fileName.isEmpty())
        return false;

    return saveFile(fileName);
}

bool ManualSubWindow::saveFile(const QString& fileName)
{
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    bool succeeded = manual_->write(fileName);
    QGuiApplication::restoreOverrideCursor();

    if (succeeded)
        setTitleName(fileName);

    return succeeded;
}

QString ManualSubWindow::getFriendlyFileName() const
{
    return QFileInfo(titleName_).fileName();
}

bool ManualSubWindow::setState(SubWinState state)
{
    if (isState(state))
        return true;

    if (!acceptChangeState(state))
        return false;

    state_ = state;
    setStateButtonMenu();
    return true;
}

QList<Coord> ManualSubWindow::getAllowCoords(Piece* piece, const Coord& fromCoord, bool fromAtBoard) const
{
    if (isState(SubWinState::LAYOUT))
        return manual_->getCanPutCoords(piece);
    else if (fromAtBoard)
        return manual_->getCanMoveCoords(fromCoord);

    return {};
}

bool ManualSubWindow::appendMove(const CoordPair& coordPair)
{
    if (!canUseModifyCommand())
        return false;

    return appendCommand(new AppendModifyCommand(manual_, coordPair));
}

QString ManualSubWindow::getFilter(bool isSave)
{
    QStringList filter = ManualIO::getSuffixNames();
    if (isSave)
        filter.removeFirst(); // 不保存第一种格式

    QString result;
    for (QString& suffix : filter)
        result.append(QString("棋谱文件(*.%1);;").arg(suffix));
    result.remove(result.length() - 2, 2);

    return result;
}

bool ManualSubWindow::loadTitleName(const QString& titleName, const InfoMap& infoMap)
{
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    bool succeeded { false };
    if (infoMap.isEmpty())
        succeeded = manual_->read(titleName);
    else
        succeeded = manual_->read(infoMap);
    if (succeeded) {
        setTitleName(titleName);
        readSettings();
        setState(SubWinState::DISPLAY);

        emit manualMoveOpened();
    } else {
        Tools::messageBox("打开棋谱",
            QString("不能打开棋谱: %1\n\n请检查文件或记录是否存在？\n").arg(titleName),
            "关闭");
    }
    QGuiApplication::restoreOverrideCursor();

    return succeeded;
}

void ManualSubWindow::closeEvent(QCloseEvent* event)
{
    if (maybeClose()) {
        if (!isUntitled)
            writeSettings();

        event->accept();
    } else {
        event->ignore();
    }
}

void ManualSubWindow::updateMoveActionState()
{
    Move* curMove = manual_->manualMove()->move();
    bool canUseMove = canUseMoveCommand(),
         isStart = curMove->isRoot(),
         isEnd = !curMove->hasNext(),
         hasOther = curMove->hasOther(),
         isOther = curMove->isOther();

    ui->actBackStart->setEnabled(canUseMove && !isStart);
    ui->actBackOther->setEnabled(canUseMove && isOther);
    ui->actBackInc->setEnabled(canUseMove && !isStart);
    ui->actBackNext->setEnabled(canUseMove && !isStart);
    ui->actGoNext->setEnabled(canUseMove && !isEnd);
    ui->actGoOther->setEnabled(canUseMove && hasOther);
    ui->actGoInc->setEnabled(canUseMove && !isEnd);
    ui->actGoEnd->setEnabled(canUseMove && !isEnd);
    ui->btnRevoke->setEnabled(canUseMove);
    ui->btnRecover->setEnabled(canUseMove);
    if (canUseMove) {
        setRevokeButtonMenu();
        setRecoverButtonMenu();
    }
    ui->actDeleteMove->setEnabled(!isStart);

    ui->remarkTextEdit->setPlainText(manual_->manualMove()->getCurRemark());
    ui->noteTextEdit->setPlainText(manual_->getPieceChars() + "\n\n" + manual_->boardString(true));
}

void ManualSubWindow::manualModified()
{
    isModified = true;
    setWindowModified(true);
}

bool ManualSubWindow::appendCommand(Command* command)
{
    bool success = commandContainer_->append(command, allowPush(command->type()));
    if (success)
        commandEffect(command->type());

    return success;
}

bool ManualSubWindow::allowPush(CommandType type) const
{
    switch (state_) {
    case SubWinState::LAYOUT:
        return type == CommandType::Put;
    case SubWinState::PLAY:
        return type == CommandType::MoveModify;
    case SubWinState::DISPLAY:
        return type == CommandType::MoveWalk;
    default:
        break;
    }

    return true;
}

void ManualSubWindow::revoke(int num)
{
    CommandType type {};
    if (commandContainer_->revoke(num, type))
        commandEffect(type);
}

void ManualSubWindow::recover(int num)
{
    CommandType type {};
    if (commandContainer_->recover(num, type))
        commandEffect(type);
}

void ManualSubWindow::revokeNum()
{
    revoke(qobject_cast<QAction*>(sender())->data().toInt());
}

void ManualSubWindow::recoverNum()
{
    recover(qobject_cast<QAction*>(sender())->data().toInt());
}

void ManualSubWindow::clearRevokes()
{
    commandContainer_->clearRevokes();
    ui->btnRevoke->setEnabled(false);
    setRecoverButtonMenu();
}

void ManualSubWindow::clearRecovers()
{
    commandContainer_->clearRecovers();
    ui->btnRecover->setEnabled(false);
    //    setRevokeButtonMenu();
}

void ManualSubWindow::commandEffect(CommandType type)
{
    switch (type) {
    case CommandType::Put:
        emit manualPiecePuted();
        playSound("MOVE.WAV");
        break;
    case CommandType::MoveModify:
        //    clearRevokes(); // 如何做到保留可撤销命令，去除不可撤销的移动历史命令？
        //    clearRecovers(); //

        emit manualMoveModified();
        playSound("PROMOTE.WAV");
        break;
    case CommandType::MoveWalk:
        emit manualMoveWalked();
        playSound("MOVE2.WAV");
        break;
    default:
        break;
    }
}

void ManualSubWindow::toggleState()
{
    setState(SubWinState(qobject_cast<QAction*>(sender())->data().toInt()));
}

void ManualSubWindow::on_actRevoke_triggered()
{
    revoke(1);
}

void ManualSubWindow::on_actRecover_triggered()
{
    recover(1);
}

void ManualSubWindow::on_actBackStart_triggered()
{
    if (!canUseMoveCommand())
        return;

    appendCommand(new BackStartMoveCommand(manual_));
}

void ManualSubWindow::on_actBackInc_triggered()
{
    if (!canUseMoveCommand())
        return;

    appendCommand(new BackIncMoveCommand(manual_, MoveCount));
}

void ManualSubWindow::on_actBackNext_triggered()
{
    if (!canUseMoveCommand())
        return;

    appendCommand(new BackToPreMoveCommand(manual_));
}

void ManualSubWindow::on_actBackOther_triggered()
{
    if (!canUseMoveCommand())
        return;

    appendCommand(new BackOtherMoveCommand(manual_));
}

void ManualSubWindow::on_actGoNext_triggered()
{
    if (!canUseMoveCommand())
        return;

    appendCommand(new GoNextMoveCommand(manual_));
}

void ManualSubWindow::on_actGoOther_triggered()
{
    if (!canUseMoveCommand())
        return;

    appendCommand(new GoOtherMoveCommand(manual_));
}

void ManualSubWindow::on_actGoInc_triggered()
{
    if (!canUseMoveCommand())
        return;

    appendCommand(new GoIncMoveCommand(manual_, MoveCount));
}

void ManualSubWindow::on_actGoEnd_triggered()
{
    if (!canUseMoveCommand())
        return;

    appendCommand(new GoEndMoveCommand(manual_));
}

void ManualSubWindow::on_curMoveChanged(Move* move)
{
    if (!canUseMoveCommand())
        return;

    appendCommand(new GoToMoveCommand(manual_, move));
}

void ManualSubWindow::on_actAllLeave_triggered()
{
    ui->boardView->allPieceToLeave();
    setState(SubWinState::LAYOUT);
}

void ManualSubWindow::on_boardView_customContextMenuRequested(const QPoint& pos)
{
    Q_UNUSED(pos)
    QMenu* menu = new QMenu(this);
    menu->addAction(ui->actBackStart);
    menu->addAction(ui->actBackInc);
    menu->addAction(ui->actBackNext);
    menu->addAction(ui->actBackOther);
    menu->addAction(ui->actGoNext);
    menu->addAction(ui->actGoOther);
    menu->addAction(ui->actGoInc);
    menu->addAction(ui->actGoEnd);
    menu->addSeparator();
    menu->addAction(ui->actAllLeave);
    //    menu->addAction(ui->actChangeStatus);
    menu->addSeparator();
    menu->addAction(ui->actLeavePiece);
    menu->addAction(ui->actStudy);
    menu->addAction(ui->actMoveInfo);
    menu->exec(QCursor::pos());
    delete menu;
}

void ManualSubWindow::on_moveView_customContextMenuRequested(const QPoint& pos)
{
    Q_UNUSED(pos)
    QMenu* menu = new QMenu(this);
    menu->addAction(ui->actAlignLeft);
    menu->addAction(ui->actAlignCenter);
    menu->addAction(ui->actAlignRight);
    menu->addSeparator();
    menu->addAction(ui->actFitStart);
    menu->addAction(ui->actFitWidth);
    menu->addAction(ui->actFitAll);
    menu->addAction(ui->actZoomOut);
    menu->addAction(ui->actZoomIn);
    menu->addSeparator();
    menu->addAction(ui->actDeleteMove);
    menu->addSeparator();
    menu->addAction(ui->actExportMove);
    menu->addSeparator();
    menu->addAction(ui->actMoveInfo);
    menu->exec(QCursor::pos());
    delete menu;
}

void ManualSubWindow::on_studyTabWidget_customContextMenuRequested(const QPoint& pos)
{
    Q_UNUSED(pos)
    QMenu* menu = new QMenu(this);
    menu->addAction(ui->actStudy);
    menu->exec(QCursor::pos());
    delete menu;
}

void ManualSubWindow::on_ManualSubWindow_customContextMenuRequested(const QPoint& pos)
{
    Q_UNUSED(pos)
    QMenu* menu = new QMenu(this);
    menu->addAction(ui->actLeavePiece);
    menu->addAction(ui->actStudy);
    menu->addAction(ui->actMoveInfo);
    menu->exec(QCursor::pos());
    delete menu;
}

void ManualSubWindow::on_actShowInfo_triggered()
{
    std::function<void(QList<QPair<QLineEdit*, InfoIndex>>)>
        setTexts_ = [&](QList<QPair<QLineEdit*, InfoIndex>> lineEditIndexs) {
            for (auto& lineEditIndex : lineEditIndexs) {
                lineEditIndex.first->setText(manual_->getInfoValue(lineEditIndex.second));
                lineEditIndex.first->setCursorPosition(0);
            }
        };

    setTexts_({ { ui->titleLineEdit, InfoIndex::TITLE },
        { ui->eventLineEdit, InfoIndex::EVENT },
        { ui->dateLineEdit, InfoIndex::DATE },
        { ui->siteLineEdit, InfoIndex::SITE },
        { ui->redLineEdit, InfoIndex::RED },
        { ui->blackLineEdit, InfoIndex::BLACK },
        { ui->eccoSnLineEdit, InfoIndex::ECCOSN },
        { ui->eccoNameLineEdit, InfoIndex::ECCONAME },
        { ui->resultLineEdit, InfoIndex::RESULT },
        { ui->writerLineEdit, InfoIndex::WRITER },
        { ui->sourceLineEdit, InfoIndex::SOURCE } });
}

void ManualSubWindow::on_actSaveInfo_triggered()
{
    std::function<void(QList<QPair<QLineEdit*, InfoIndex>>)>
        saveTexts_ = [&](QList<QPair<QLineEdit*, InfoIndex>> lineEditIndexs) {
            for (auto& lineEditIndex : lineEditIndexs) {
                manual_->setInfoValue(lineEditIndex.second, lineEditIndex.first->text());
            }
        };

    saveTexts_({ { ui->titleLineEdit, InfoIndex::TITLE },
        { ui->eventLineEdit, InfoIndex::EVENT },
        { ui->dateLineEdit, InfoIndex::DATE },
        { ui->siteLineEdit, InfoIndex::SITE },
        { ui->redLineEdit, InfoIndex::RED },
        { ui->blackLineEdit, InfoIndex::BLACK },
        { ui->eccoSnLineEdit, InfoIndex::ECCOSN },
        { ui->eccoNameLineEdit, InfoIndex::ECCONAME },
        { ui->resultLineEdit, InfoIndex::RESULT },
        { ui->writerLineEdit, InfoIndex::WRITER },
        { ui->sourceLineEdit, InfoIndex::SOURCE } });

    manualModified();
}

void ManualSubWindow::on_actCopyInfo_triggered()
{
    QApplication::clipboard()->setText(ManualIO::getInfoString(manual_).remove("\n\n"));
}

void ManualSubWindow::on_remarkTextEdit_textChanged()
{
    manual_->manualMove()->setCurRemark(ui->remarkTextEdit->toPlainText());
}

void ManualSubWindow::on_moveInfoTabWidget_currentChanged(int index)
{
    if (index == 0) {

    } else if (index == 1) {
        if (ui->titleLineEdit->text().isEmpty())
            on_actShowInfo_triggered();
    }
}

void ManualSubWindow::on_moveTabWidget_currentChanged(int index)
{
    if (index == 0) {

    } else if (index == 1) {
        if (ui->pgnTextEdit->toPlainText().isEmpty())
            on_pgnTypeComboBox_currentIndexChanged(0);
    }
}

void ManualSubWindow::on_pgnTypeComboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index)

    int pgnTypeIndex = ui->pgnTypeComboBox->currentIndex(),
        scopeIndex = ui->scopeComboBox->currentIndex();
    StoreType storeType = StoreType(pgnTypeIndex + int(StoreType::PGN_ICCS));
    ui->pgnTextEdit->setLineWrapMode(storeType == StoreType::PGN_CC
            ? QPlainTextEdit::LineWrapMode::NoWrap
            : QPlainTextEdit::LineWrapMode::WidgetWidth);
    ui->pgnTextEdit->setPlainText(scopeIndex == 1
            ? manual_->toString(storeType)
            : manual_->toMoveString(storeType));
}

void ManualSubWindow::on_scopeComboBox_currentIndexChanged(int index)
{
    on_pgnTypeComboBox_currentIndexChanged(index);
}

void ManualSubWindow::on_actCopyPgntext_triggered()
{
    QApplication::clipboard()->setText(ui->pgnTextEdit->toPlainText());
}

void ManualSubWindow::on_actLeavePiece_toggled(bool checked)
{
    QRectF sceneRect = checked ? ui->boardView->scene()->sceneRect() : ui->boardView->boardRect();

    const int margin { 2 };
    ui->boardView->setSceneRect(sceneRect);
    ui->boardView->setFixedSize(sceneRect.width() + margin, sceneRect.height() + margin);
}

void ManualSubWindow::on_actStudy_toggled(bool checked)
{
    ui->studyTabWidget->setVisible(checked);
}

void ManualSubWindow::on_actMoveInfo_toggled(bool checked)
{
    ui->moveInfoTabWidget->setVisible(checked);
}

void ManualSubWindow::on_actAlignLeft_triggered()
{
    ui->moveView->setNodeItemLayout(MoveNodeItemAlign::LEFT);
}

void ManualSubWindow::on_actAlignCenter_triggered()
{
    ui->moveView->setNodeItemLayout(MoveNodeItemAlign::CENTER);
}

void ManualSubWindow::on_actAlignRight_triggered()
{
    ui->moveView->setNodeItemLayout(MoveNodeItemAlign::RIGHT);
}

void ManualSubWindow::on_actFitStart_triggered()
{
    ui->moveView->setTransform(QTransform());
}

void ManualSubWindow::on_actFitWidth_triggered()
{
    ui->moveView->fitInView(QRectF(0, 0, ui->moveView->sceneRect().width(), 100),
        Qt::AspectRatioMode::KeepAspectRatio);
}

void ManualSubWindow::on_actFitAll_triggered()
{
    ui->moveView->fitInView(ui->moveView->sceneRect(),
        Qt::AspectRatioMode::KeepAspectRatio);
}

void ManualSubWindow::on_actZoomIn_triggered()
{
    qreal coefficient = 1 + scaleStepValue;
    ui->moveView->scale(coefficient, coefficient);
}

void ManualSubWindow::on_actZoomOut_triggered()
{
    qreal coefficient = 1 / (1 + scaleStepValue);
    ui->moveView->scale(coefficient, coefficient);
}

void ManualSubWindow::on_wheelScrolled(bool isUp)
{
    isUp ? on_actZoomIn_triggered() : on_actZoomOut_triggered();
}

void ManualSubWindow::on_actDeleteMove_triggered()
{
    if (canUseModifyCommand()) {
        int index = Tools::messageBox("删除着法",
            "执行【删除】命令后，当前及后续着法将全部被删除。\n\n确定执行吗？\n",
            "确定", "取消");
        if (index > 0)
            return;
    } else {
        Tools::messageBox("删除着法",
            "【删除】命令需要在【打谱】模式下执行。\n\n如果确定删除该着法，请先转换为【打谱】模式。\n",
            "关闭");
        return;
    }

    appendCommand(new DeleteModifyCommand(manual_));
}

void ManualSubWindow::on_actExportMove_triggered()
{
}

QMdiSubWindow* ManualSubWindow::getSubWindow() const
{
    return qobject_cast<QMdiSubWindow*>(parent());
}

void ManualSubWindow::setBtnAction()
{
    std::function<void(QList<QPair<QToolButton*, QAction*>>)>
        setActions_ = [](QList<QPair<QToolButton*, QAction*>> buttonActions) {
            for (auto& buttonAction : buttonActions)
                buttonAction.first->setDefaultAction(buttonAction.second);
        };

    setActions_({
        // 棋谱导航
        { ui->btnRevoke, ui->actRevoke },
        { ui->btnRecover, ui->actRecover },

        { ui->btnStartMove, ui->actBackStart },
        { ui->btnSomePreMove, ui->actBackInc },
        { ui->btnPreMove, ui->actBackNext },
        { ui->btnOtherPreMove, ui->actBackOther },
        { ui->btnNextMove, ui->actGoNext },
        { ui->btnOtherMove, ui->actGoOther },
        { ui->btnSomeNextMove, ui->actGoInc },
        { ui->btnEndMove, ui->actGoEnd },

        // 设置状态
        //        { ui->btnChangeStatus, ui->actChangeStatus },

        // 局部区域隐藏或显示
        { ui->btnLeavePiece, ui->actLeavePiece },
        { ui->btnStudy, ui->actStudy },
        { ui->btnMoveInfo, ui->actMoveInfo },

        // 放弃或保存棋谱信息
        { ui->btnShowInfo, ui->actShowInfo },
        { ui->btnSaveInfo, ui->actSaveInfo },
        { ui->btnCopyInfo, ui->actCopyInfo },

        { ui->btnCopyPgntext, ui->actCopyPgntext },

        // 图形视图调整
        { ui->btnAlignLeft, ui->actAlignLeft },
        { ui->btnAlignCenter, ui->actAlignCenter },
        { ui->btnAlignRight, ui->actAlignRight },
        { ui->btnFitStart, ui->actFitStart },
        { ui->btnFitWidth, ui->actFitWidth },
        { ui->btnFitAll, ui->actFitAll },
        { ui->btnZoomIn, ui->actZoomIn },
        { ui->btnZoomOut, ui->actZoomOut },

        // 调整或保存着法
        { ui->btnDeleteMove, ui->actDeleteMove },
        { ui->btnExportMove, ui->actExportMove },
    });

    // 多个快捷键
    //        ui->actPreMove->setShortcuts({ Qt::Key_Up, Qt::Key_Left });

    QActionGroup* alignGroup = new QActionGroup(this);
    alignGroup->addAction(ui->actAlignLeft);
    alignGroup->addAction(ui->actAlignCenter);
    alignGroup->addAction(ui->actAlignRight);
}

void ManualSubWindow::setRevokeButtonMenu()
{
    setNavButtonMenu(ui->btnRevoke, commandContainer_->getRevokeStrings(), true);
}

void ManualSubWindow::setRecoverButtonMenu()
{
    setNavButtonMenu(ui->btnRecover, commandContainer_->getRecoverStrings(), false);
}

void ManualSubWindow::setNavButtonMenu(QToolButton* btn, QStringList commandStrings, bool isRevoke)
{
    QMenu* oldMenu = btn->menu();
    if (oldMenu)
        oldMenu->deleteLater(); // 回到程序界面事件循环时执行

    bool isEmpty = commandStrings.isEmpty();
    btn->setEnabled(!isEmpty);
    if (isEmpty) {
        btn->setMenu(Q_NULLPTR);
        return;
    }

    QIcon icon(QString(":/res/icon/%1.svg").arg(isRevoke ? "back" : "forward"));
    QMenu* menu = new QMenu(this);
    int num = 0;
    for (auto& text : commandStrings) {
        QAction* action = menu->addAction(text);
        action->setIcon(icon);
        action->setData(++num);
        auto func = isRevoke ? &ManualSubWindow::revokeNum : &ManualSubWindow::recoverNum;
        connect(action, &QAction::triggered, this, func);
    }

    menu->addSeparator();
    QAction* action = menu->addAction("清除历史着法");
    action->setData(++num);
    auto func = isRevoke ? &ManualSubWindow::clearRevokes : &ManualSubWindow::clearRecovers;
    connect(action, &QAction::triggered, this, func);

    btn->setMenu(menu);
}

void ManualSubWindow::setStateButtonMenu()
{
    QMenu* oldMenu = ui->btnTurnState->menu();
    if (oldMenu)
        oldMenu->deleteLater(); // 回到程序界面事件循环时执行

    QMenu* menu = new QMenu(this);
    QList<QList<SubWinState>> changedStates {
        { SubWinState::PLAY },
        { SubWinState::LAYOUT, SubWinState::DISPLAY },
        { SubWinState::LAYOUT, SubWinState::PLAY }
    };
    for (auto& state : changedStates.at(int(state_))) {
        int stateIndex = int(state);
        QAction* action = menu->addAction(QString("转至\t%1 模式").arg(StateStrings.at(stateIndex)));
        action->setData(stateIndex);
        connect(action, &QAction::triggered, this, &ManualSubWindow::toggleState);
    }

    ui->btnTurnState->setMenu(menu);
    ui->btnTurnState->setText(StateStrings.at(int(state_)));
}

void ManualSubWindow::playSound(const QString& fileName) const
{
    QSound::play(soundDir.arg(fileName));
}

bool ManualSubWindow::acceptChangeState(SubWinState state)
{
    if (isState(SubWinState::NOTSTATE))
        return true;

    auto confirm = [](const QString& text) {
        return (Tools::messageBox(
                    "转换模式",
                    QString("转换为%1，历史操作记录会被清除。\n\n确定转换吗？\n").arg(text),
                    "确定", "取消")
            == 0);
    };

    switch (state) {
    case SubWinState::LAYOUT: {
        if (!confirm("【布局】模式后，现有棋局着法将全部被删除"))
            return false;

        manual_->manualMove()->backStart();
        manual_->manualMove()->goNext();
        appendCommand(new DeleteModifyCommand(manual_));
    } break;
    case SubWinState::PLAY:
        if (isState(SubWinState::DISPLAY)
            && !confirm("【打谱】模式后，当前的后续着法将被删除（变着不会被删）"))
            return false;
        break;
    case SubWinState::DISPLAY:
        if (!confirm("【演示】模式后"))
            return false;
        break;
    default:
        break;
    }

    clearRevokes(); // 如何做到保留可撤销命令，去除不可撤销的移动历史命令？
    clearRecovers(); //
    return true;
}

bool ManualSubWindow::canUsePutCommand() const
{
    return isState(SubWinState::LAYOUT);
}

bool ManualSubWindow::canUseMoveCommand() const
{
    return isState(SubWinState::PLAY) || isState(SubWinState::DISPLAY);
}

bool ManualSubWindow::canUseModifyCommand() const
{
    return isState(SubWinState::PLAY);
}

void ManualSubWindow::writeSettings() const
{
    QSettings settings;
    settings.beginGroup(stringLiterals[StringIndex::MAINWINDOW]);

    settings.beginGroup(titleName_);
    settings.setValue(stringLiterals[StringIndex::LEFTSHOW], ui->actLeavePiece->isChecked());
    settings.setValue(stringLiterals[StringIndex::RIGHTSHOW], ui->actMoveInfo->isChecked());
    settings.setValue(stringLiterals[StringIndex::DOWNSHOW], ui->actStudy->isChecked());
    settings.setValue(stringLiterals[StringIndex::MOVEINFOTABINDEX], ui->moveInfoTabWidget->currentIndex());
    settings.setValue(stringLiterals[StringIndex::MOVETABINDEX], ui->moveTabWidget->currentIndex());
    settings.setValue(stringLiterals[StringIndex::MOVEPGNTEXTINDEX], ui->pgnTypeComboBox->currentIndex());
    settings.setValue(stringLiterals[StringIndex::STUDYTABINDEX], ui->studyTabWidget->currentIndex());
    settings.setValue(stringLiterals[StringIndex::WINGEOMETRY], getSubWindow()->saveGeometry());
    settings.setValue(stringLiterals[StringIndex::MOVESOUND], moveSound);
    settings.setValue(stringLiterals[StringIndex::MOVESOUNDDIR], soundDir);
    settings.endGroup();

    settings.endGroup();
}

void ManualSubWindow::readSettings()
{
    QSettings settings;
    settings.beginGroup(stringLiterals[StringIndex::MAINWINDOW]);

    settings.beginGroup(titleName_);
    ui->actLeavePiece->setChecked(settings.value(stringLiterals[StringIndex::LEFTSHOW], true).toBool());
    ui->actMoveInfo->setChecked(settings.value(stringLiterals[StringIndex::RIGHTSHOW], true).toBool());
    ui->actStudy->setChecked(settings.value(stringLiterals[StringIndex::DOWNSHOW], true).toBool());

    int moveInfoIndex = settings.value(stringLiterals[StringIndex::MOVEINFOTABINDEX], 0).toInt(),
        moveIndex = settings.value(stringLiterals[StringIndex::MOVETABINDEX], 0).toInt(),
        pgnTextIndex = settings.value(stringLiterals[StringIndex::MOVEPGNTEXTINDEX], 0).toInt();
    ui->moveInfoTabWidget->setCurrentIndex(moveInfoIndex);
    ui->moveTabWidget->setCurrentIndex(moveIndex);
    ui->pgnTypeComboBox->setCurrentIndex(pgnTextIndex);
    on_moveInfoTabWidget_currentChanged(moveInfoIndex);
    if (moveInfoIndex == 0) {
        on_moveTabWidget_currentChanged(moveIndex);
    }
    ui->studyTabWidget->setCurrentIndex(settings.value(stringLiterals[StringIndex::STUDYTABINDEX], 0).toInt());
    QVariant winGeometry = settings.value(stringLiterals[StringIndex::WINGEOMETRY]);
    if (!winGeometry.isNull())
        getSubWindow()->restoreGeometry(winGeometry.toByteArray());

    moveSound = settings.value(stringLiterals[StringIndex::MOVESOUND], true).toBool();
    soundDir = settings.value(stringLiterals[StringIndex::MOVESOUNDDIR], ":/res/SOUNDS/%1").toString();
    settings.endGroup();

    settings.endGroup();
}

bool ManualSubWindow::maybeClose()
{
    if (!isUntitled && !isModified)
        return true;

    int index = Tools::messageBox("保存棋谱",
        QString("'%1':\n尚未保存最新修改.\n\n需要保存最新的修改吗？").arg(titleName_),
        "保存", "放弃", "取消");

    if (index == 0)
        return save();
    else if (index == 1)
        return true;

    return false;
}

void ManualSubWindow::setTitleName(const QString& titleName)
{
    titleName_ = QFileInfo::exists(titleName) ? QFileInfo(titleName).canonicalFilePath() : titleName;
    isUntitled = false;
    isModified = false;
    setWindowModified(false);
    setWindowTitle(titleName_ + "[*]");
}
