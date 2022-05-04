#include "chessform.h"
#include "boardscene.h"
#include "boardview.h"
#include "common.h"
#include "instance.h"
#include "instanceio.h"
#include "move.h"
#include "moveitem.h"
#include "moveview.h"
#include "tools.h"
#include "ui_chessform.h"
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

ChessForm::ChessForm(QWidget* parent)
    : QWidget(parent)
    , isUntitled(true)
    , isModified(false)
    , formTitleName(QString())
    , instance(new Instance)
    , ui(new Ui::ChessForm)
{
    ui->setupUi(this);
    connect(this, &ChessForm::instanceModified, ui->moveView, &MoveView::resetNodeItems);
    connect(this, &ChessForm::instanceModified, this, &ChessForm::instanceMoved);

    connect(this, &ChessForm::instanceMoved, this, &ChessForm::updateMoveButtonEnabled);
    connect(this, &ChessForm::instanceMoved, ui->boardView, &BoardView::updatePieceItemShow);
    connect(this, &ChessForm::instanceMoved, ui->moveView, &MoveView::updateNodeItemSelected);

    connect(ui->moveView, &MoveView::mousePressed, this, &ChessForm::on_curMoveChanged);
    connect(ui->moveView, &MoveView::wheelScrolled, this, &ChessForm::on_wheelScrolled);

    ui->boardView->setInstance(instance);
    ui->moveView->setInstance(instance);

    setBtnAction();
}

ChessForm::~ChessForm()
{
    delete instance;
    delete ui;
}

void ChessForm::newFile()
{
    static int sequenceNumber = 1;
    isUntitled = true;
    formTitleName = (QString("未命名%2.%3")
                         .arg(sequenceNumber++)
                         .arg(InstanceIO::getSuffixName(StoreType::PGN_ZH)));
    setWindowTitle(formTitleName + "[*]");

    playSound("NEWGAME.WAV");
    emit instanceModified();
}

bool ChessForm::save()
{
    if (isUntitled) {
        return saveAs();
    } else {
        return saveFile(formTitleName);
    }
}

bool ChessForm::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, "另存为", "./", getFilter(true));
    if (fileName.isEmpty())
        return false;

    return saveFile(fileName);
}

bool ChessForm::saveFile(const QString& fileName)
{
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    bool succeeded = InstanceIO::write(instance, fileName);
    QGuiApplication::restoreOverrideCursor();

    if (succeeded)
        setFormTitleName(fileName);

    return succeeded;
}

bool ChessForm::needNotSave() const
{
    return !isUntitled && !isModified;
}

QString ChessForm::getFriendlyFileName() const
{
    return QFileInfo(formTitleName).fileName();
}

QString ChessForm::getFilter(bool isSave)
{
    QStringList filter = InstanceIO::getSuffixNames();
    if (isSave)
        filter.removeFirst(); // 不保存第一种格式

    QString result;
    for (QString& suffix : filter)
        result.append(QString("棋谱文件(*.%1);;").arg(suffix));
    result.remove(result.length() - 2, 2);

    return result;
}

bool ChessForm::loadTitleName(const QString& titleName, const InfoMap& infoMap)
{
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    bool succeeded { false };
    if (infoMap.isEmpty())
        succeeded = InstanceIO::read(instance, titleName);
    else
        succeeded = InstanceIO::read(instance, infoMap);
    if (succeeded) {
        setFormTitleName(titleName);
        readSettings();

        //        playSound("DRAW.WAV");
        emit instanceModified();
    } else {
        QMessageBox::warning(this, "打开棋谱",
            QString("不能打开棋谱: %1\n请检查文件或记录是否存在？\n")
                .arg(titleName));
    }
    QGuiApplication::restoreOverrideCursor();

    return succeeded;
}

void ChessForm::closeEvent(QCloseEvent* event)
{
    if (maybeSave()) {
        if (!isUntitled)
            writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void ChessForm::updateMoveButtonEnabled()
{
    bool isStart = instance->isStart(),
         isEnd = instance->isEnd(),
         hasOther = instance->hasOther(),
         isOther = instance->isOther();

    ui->btnStartMove->setEnabled(!isStart);
    ui->btnOtherPreMove->setEnabled(isOther);
    ui->btnSomePreMove->setEnabled(!isStart);
    ui->btnPreMove->setEnabled(!isStart);
    ui->btnNextMove->setEnabled(!isEnd);
    ui->btnOtherMove->setEnabled(hasOther);
    ui->btnSomeNextMove->setEnabled(!isEnd);
    ui->btnEndMove->setEnabled(!isEnd);

    ui->remarkTextEdit->setPlainText(instance->getCurMove()->remark());
    ui->noteTextEdit->setPlainText(instance->getPieceChars() + "\n\n" + instance->boardString(true));
}

void ChessForm::documentWasModified()
{
    setWindowModified(isModified);
}

void ChessForm::on_actStartMove_triggered()
{
    instance->backStart();
    playSound("MOVE2.WAV");
    emit instanceMoved();
}

void ChessForm::on_actSomePreMove_triggered()
{
    instance->goOrBackInc(-ui->moveView->getNodeItemNumPerPage());
    playSound("MOVE2.WAV");
    emit instanceMoved();
}

void ChessForm::on_actPreMove_triggered()
{
    instance->backToPre();
    playSound("MOVE.WAV");
    emit instanceMoved();
}

void ChessForm::on_actOtherPreMove_triggered()
{
    instance->backOther();
    playSound("MOVE.WAV");
    emit instanceMoved();
}

void ChessForm::on_actNextMove_triggered()
{
    instance->goNext();
    playSound("MOVE2.WAV");
    emit instanceMoved();
}

void ChessForm::on_actOtherMove_triggered()
{
    instance->goOther();
    playSound("CHECK2.WAV");
    emit instanceMoved();
}

void ChessForm::on_actSomeNextMove_triggered()
{
    instance->goOrBackInc(ui->moveView->getNodeItemNumPerPage());
    playSound("MOVE2.WAV");
    emit instanceMoved();
}

void ChessForm::on_actEndMove_triggered()
{
    instance->goEnd();
    playSound("MOVE2.WAV"); // "WIN.WAV"
    emit instanceMoved();
}

void ChessForm::on_curMoveChanged(Move* move)
{
    instance->goTo(move);
    playSound("MOVE2.WAV");
    emit instanceMoved();
}

void ChessForm::on_actAllLeave_triggered()
{
    ui->boardView->allPieceToLeave();
}

void ChessForm::on_actChangeStatus_triggered(bool checked)
{
    instance->setStatus(checked ? InsStatus::PLAY : InsStatus::LAYOUT);
}

void ChessForm::on_boardView_customContextMenuRequested(const QPoint& pos)
{
    Q_UNUSED(pos)
    QMenu* menu = new QMenu(this);
    menu->addAction(ui->actStartMove);
    menu->addAction(ui->actSomePreMove);
    menu->addAction(ui->actOtherPreMove);
    menu->addAction(ui->actPreMove);
    menu->addAction(ui->actNextMove);
    menu->addAction(ui->actOtherMove);
    menu->addAction(ui->actSomeNextMove);
    menu->addAction(ui->actEndMove);
    menu->addSeparator();
    menu->addAction(ui->actAllLeave);
    menu->addAction(ui->actChangeStatus);
    menu->addSeparator();
    menu->addAction(ui->actLeavePiece);
    menu->addAction(ui->actStudy);
    menu->addAction(ui->actMoveInfo);
    menu->exec(QCursor::pos());
    delete menu;
}

void ChessForm::on_moveView_customContextMenuRequested(const QPoint& pos)
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
    menu->addAction(ui->actExportMove);
    menu->addSeparator();
    menu->addAction(ui->actMoveInfo);
    menu->exec(QCursor::pos());
    delete menu;
}

void ChessForm::on_studyTabWidget_customContextMenuRequested(const QPoint& pos)
{
    Q_UNUSED(pos)
    QMenu* menu = new QMenu(this);
    menu->addAction(ui->actStudy);
    menu->exec(QCursor::pos());
    delete menu;
}

void ChessForm::on_ChessForm_customContextMenuRequested(const QPoint& pos)
{
    Q_UNUSED(pos)
    QMenu* menu = new QMenu(this);
    menu->addAction(ui->actLeavePiece);
    menu->addAction(ui->actStudy);
    menu->addAction(ui->actMoveInfo);
    menu->exec(QCursor::pos());
    delete menu;
}

void ChessForm::on_actShowInfo_triggered()
{
    std::function<void(QList<QPair<QLineEdit*, InfoIndex>>)>
        setTexts_ = [&](QList<QPair<QLineEdit*, InfoIndex>> lineEditIndexs) {
            for (auto& lineEditIndex : lineEditIndexs) {
                lineEditIndex.first->setText(instance->getInfoValue(lineEditIndex.second));
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

void ChessForm::on_actSaveInfo_triggered()
{
    std::function<void(QList<QPair<QLineEdit*, InfoIndex>>)>
        saveTexts_ = [&](QList<QPair<QLineEdit*, InfoIndex>> lineEditIndexs) {
            for (auto& lineEditIndex : lineEditIndexs) {
                instance->setInfoValue(lineEditIndex.second, lineEditIndex.first->text());
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

    isModified = true;
    documentWasModified();
}

void ChessForm::on_actCopyInfo_triggered()
{
    QApplication::clipboard()->setText(InstanceIO::getInfoString(instance).remove("\n\n"));
}

void ChessForm::on_remarkTextEdit_textChanged()
{
    instance->getCurMove()->setRemark(ui->remarkTextEdit->toPlainText());
}

void ChessForm::on_moveInfoTabWidget_currentChanged(int index)
{
    if (index == 0) {

    } else if (index == 1) {
        if (ui->titleLineEdit->text().isEmpty())
            on_actShowInfo_triggered();
    }
}

void ChessForm::on_moveTabWidget_currentChanged(int index)
{
    if (index == 0) {

    } else if (index == 1) {
        if (ui->pgnTextEdit->toPlainText().isEmpty())
            on_pgnTypeComboBox_currentIndexChanged(0);
    }
}

void ChessForm::on_pgnTypeComboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index)

    int pgnTypeIndex = ui->pgnTypeComboBox->currentIndex(),
        scopeIndex = ui->scopeComboBox->currentIndex();
    StoreType storeType = StoreType(pgnTypeIndex + int(StoreType::PGN_ICCS));
    ui->pgnTextEdit->setLineWrapMode(storeType == StoreType::PGN_CC
            ? QPlainTextEdit::LineWrapMode::NoWrap
            : QPlainTextEdit::LineWrapMode::WidgetWidth);
    ui->pgnTextEdit->setPlainText(scopeIndex == 1
            ? instance->toString(storeType)
            : instance->toMoveString(storeType));
}

void ChessForm::on_scopeComboBox_currentIndexChanged(int index)
{
    on_pgnTypeComboBox_currentIndexChanged(index);
}

void ChessForm::on_actCopyPgntext_triggered()
{
    QApplication::clipboard()->setText(ui->pgnTextEdit->toPlainText());
}

void ChessForm::on_actLeavePiece_toggled(bool checked)
{
    QRectF sceneRect = checked ? ui->boardView->scene()->sceneRect() : ui->boardView->boardSceneRect();

    const int margin { 2 };
    ui->boardView->setSceneRect(sceneRect);
    ui->boardView->setFixedSize(sceneRect.width() + margin, sceneRect.height() + margin);
}

void ChessForm::on_actStudy_toggled(bool checked)
{
    ui->studyTabWidget->setVisible(checked);
}

void ChessForm::on_actMoveInfo_toggled(bool checked)
{
    ui->moveInfoTabWidget->setVisible(checked);
}

void ChessForm::on_actAlignLeft_triggered()
{
    ui->moveView->setNodeItemLayout(MoveNodeItemAlign::LEFT);
}

void ChessForm::on_actAlignCenter_triggered()
{
    ui->moveView->setNodeItemLayout(MoveNodeItemAlign::CENTER);
}

void ChessForm::on_actAlignRight_triggered()
{
    ui->moveView->setNodeItemLayout(MoveNodeItemAlign::RIGHT);
}

void ChessForm::on_actFitStart_triggered()
{
    ui->moveView->setTransform(QTransform());
    ui->moveView->updateNodeItemSelected();
}

void ChessForm::on_actFitWidth_triggered()
{
    ui->moveView->fitInView(QRectF(0, 0, ui->moveView->sceneRect().width(), 100),
        Qt::AspectRatioMode::KeepAspectRatio);
    ui->moveView->updateNodeItemSelected();
}

void ChessForm::on_actFitAll_triggered()
{
    ui->moveView->fitInView(ui->moveView->sceneRect(),
        Qt::AspectRatioMode::KeepAspectRatio);
    ui->moveView->updateNodeItemSelected();
}

void ChessForm::on_actZoomIn_triggered()
{
    qreal coefficient = 1 + scaleStepValue;
    ui->moveView->scale(coefficient, coefficient);
    ui->moveView->updateNodeItemSelected();
}

void ChessForm::on_actZoomOut_triggered()
{
    qreal coefficient = 1 / (1 + scaleStepValue);
    ui->moveView->scale(coefficient, coefficient);
    ui->moveView->updateNodeItemSelected();
}

void ChessForm::on_wheelScrolled(bool isUp)
{
    isUp ? on_actZoomIn_triggered() : on_actZoomOut_triggered();
}

void ChessForm::on_actExportMove_triggered()
{
}

QMdiSubWindow* ChessForm::getSubWindow() const
{
    return qobject_cast<QMdiSubWindow*>(parent());
}

void ChessForm::playSound(const QString& fileName) const
{
    QSound::play(soundDir.arg(fileName));
}

void ChessForm::setBtnAction()
{
    std::function<void(QList<QPair<QToolButton*, QAction*>>)>
        setActions_ = [](QList<QPair<QToolButton*, QAction*>> buttonActions) {
            for (auto& buttonAction : buttonActions)
                buttonAction.first->setDefaultAction(buttonAction.second);
        };

    setActions_({
        // 棋谱导航
        { ui->btnStartMove, ui->actStartMove },
        { ui->btnSomePreMove, ui->actSomePreMove },
        { ui->btnPreMove, ui->actPreMove },
        { ui->btnOtherPreMove, ui->actOtherPreMove },
        { ui->btnNextMove, ui->actNextMove },
        { ui->btnOtherMove, ui->actOtherMove },
        { ui->btnSomeNextMove, ui->actSomeNextMove },
        { ui->btnEndMove, ui->actEndMove },

        // 设置状态
        { ui->btnChangeStatus, ui->actChangeStatus },

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
        { ui->btnExportMove, ui->actExportMove },
    });

    // 多个快捷键
    //        ui->actPreMove->setShortcuts({ Qt::Key_Up, Qt::Key_Left });

    QActionGroup* alignGroup = new QActionGroup(this);
    alignGroup->addAction(ui->actAlignLeft);
    alignGroup->addAction(ui->actAlignCenter);
    alignGroup->addAction(ui->actAlignRight);
}

void ChessForm::writeSettings() const
{
    QSettings settings;
    settings.beginGroup(stringLiterals[StringIndex::MAINWINDOW]);

    settings.beginGroup(formTitleName);
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

void ChessForm::readSettings()
{
    QSettings settings;
    settings.beginGroup(stringLiterals[StringIndex::MAINWINDOW]);

    settings.beginGroup(formTitleName);
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

bool ChessForm::maybeSave()
{
    if (needNotSave())
        return true;

    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, "保存棋谱",
            QString("'%1' 是新文件，或已被编辑.\n"
                    "需要保存所做的修改吗？")
                .arg(formTitleName),
            //                               .arg(getFriendlyFileName()),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        return save();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }

    return true;
}

void ChessForm::setFormTitleName(const QString& titleName)
{
    formTitleName = QFileInfo::exists(titleName) ? QFileInfo(titleName).canonicalFilePath() : titleName;
    isUntitled = false;
    isModified = false;
    setWindowModified(false);
    setWindowTitle(formTitleName + "[*]");
}
