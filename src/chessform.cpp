#include "chessform.h"
#include "boardgraphicsscene.h"
#include "ecco.h"
#include "instance.h"
#include "instanceio.h"
#include "move.h"
#include "publicString.h"
#include "tools.h"
#include "ui_chessform.h"
#include <QCloseEvent>
#include <QFileDialog>
#include <QMdiArea>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QSound>
#include <QTimer>

ChessForm::ChessForm(QWidget* parent)
    : QWidget(parent)
    , isUntitled(true)
    , isModified(false)
    , formTitleName(QString())
    , instance(new Instance)
    , boardScene(new BoardGraphicsScene(leftWidth, boardWidth, boardHeight, instance))
    , ui(new Ui::ChessForm)
{
    ui->setupUi(this);
    connect(this, &ChessForm::insCurMoveChanged, this, &ChessForm::updateInsCurMoveGUI);
    connect(this, &ChessForm::insCurMoveChanged, boardScene, &BoardGraphicsScene::updatePieceItemPos);

    initViewScene();
    setBtnAction();
}

ChessForm::~ChessForm()
{
    delete boardScene;
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

    on_actLockInstance_triggered(false);
    playSound("NEWGAME.WAV");
    emit insCurMoveChanged();
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

        on_actLockInstance_triggered(true);
        //        playSound("DRAW.WAV");
        showInfo();
        emit insCurMoveChanged();
    } else {
        QMessageBox::warning(this, "打开棋谱",
            QString("不能打开棋谱 %1\n请检查文件或记录是否存在？\n")
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

void ChessForm::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    //    static int boardWidth = 521, boardHeight = 577, hside = 0, vside = 0;

    //    QPainter painter(this);
    //    painter.drawImage(QRect(hside, vside, boardWidth, boardHeight), QImage(":/res/IMAGES_L/WOOD.JPG"));

    //    QRect viewRect(2, 2, 515, 572), winRect(-9, -10, 18, 20);
    //    painter.setViewport(viewRect);
    //    painter.setWindow(winRect);

    //    painter.drawImage(QRect(-9, -10, 2, 2), QImage(":/res/IMAGES_L/WOOD/BR.GIF"));
    //    painter.drawImage(QRect(7, 8, 2, 2), QImage(":/res/IMAGES_L/WOOD/RR.GIF"));

    //    painter.setRenderHint(QPainter::Antialiasing, true);
    //    painter.setPen(QPen(Qt::darkGray, 3, Qt::SolidLine, Qt::RoundCap));
    //    painter.setBrush(QBrush(Qt::lightGray, Qt::Dense6Pattern));
    //    painter.drawRect(22, 22, 460, 530);
}

void ChessForm::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event)
    //    ui->moveTextEdit->setPlainText(QString("x: %1, y: %2").arg(event->x()).arg(event->y()));
}

void ChessForm::updateInsCurMoveGUI()
{
    bool isStart = instance->isStartMove(),
         isEnd = instance->isEndMove(),
         hasOther = instance->hasOtherMove();

    //    ui->startBtn->setEnabled(!isStart);
    ui->btnPreMove->setEnabled(!isStart);
    ui->btnNextMove->setEnabled(!isEnd);
    ui->btnEndMove->setEnabled(!isEnd);
    ui->btnOtherMove->setEnabled(hasOther);

    ui->remarkTextEdit->setPlainText(instance->getCurMove()->remark());
}

void ChessForm::documentWasModified()
{
    setWindowModified(isModified);
}

void ChessForm::on_actStartMove_triggered()
{
    instance->backStart();
    playSound("MOVE2.WAV");
    emit insCurMoveChanged();
}

void ChessForm::on_actPreMove_triggered()
{
    instance->backOne();
    playSound("MOVE.WAV");
    emit insCurMoveChanged();
}

void ChessForm::on_actNextMove_triggered()
{
    instance->goNext();
    playSound("MOVE2.WAV");
    emit insCurMoveChanged();
}

void ChessForm::on_actOtherMove_triggered()
{
    instance->goOther();
    playSound("CHECK2.WAV");
    emit insCurMoveChanged();
}

void ChessForm::on_actEndMove_triggered()
{
    instance->goEnd();
    playSound("WIN.WAV");
    emit insCurMoveChanged();
}

void ChessForm::on_actLeavePiece_triggered(bool checked)
{
    ui->leaveGraphicsView->setVisible(checked);
    resetSize();
}

void ChessForm::on_actMoveInfo_triggered(bool checked)
{
    ui->infoTabWidget->setVisible(checked);
    resetSize();
}

void ChessForm::on_actStudy_triggered(bool checked)
{
    ui->studyTabWidget->setVisible(checked);
    resetSize();
}

void ChessForm::on_actAllLeave_triggered()
{
    boardScene->allPieceToLeave();
}

void ChessForm::on_actChangeStatus_triggered(bool checked)
{
    instance->setStatus(checked ? InsStatus::PLAY : InsStatus::LAYOUT);
}

QMdiSubWindow* ChessForm::getSubWindow() const
{
    return qobject_cast<QMdiSubWindow*>(parent());
}

void ChessForm::showInfo()
{
    ui->titleLineEdit->setText(instance->getInfoValue(InfoIndex::TITLE));
    ui->eventLineEdit->setText(instance->getInfoValue(InfoIndex::EVENT));
    ui->dateLineEdit->setText(instance->getInfoValue(InfoIndex::DATE));
    ui->siteLineEdit->setText(instance->getInfoValue(InfoIndex::SITE));
    ui->redLineEdit->setText(instance->getInfoValue(InfoIndex::RED));
    ui->blackLineEdit->setText(instance->getInfoValue(InfoIndex::BLACK));
    ui->eccoSnLineEdit->setText(instance->getInfoValue(InfoIndex::ECCOSN));
    ui->eccoNameLineEdit->setText(instance->getInfoValue(InfoIndex::ECCONAME));
    ui->resultLineEdit->setText(instance->getInfoValue(InfoIndex::RESULT));
    ui->writerLineEdit->setText(instance->getInfoValue(InfoIndex::WRITER));
    ui->sourceLineEdit->setText(instance->getInfoValue(InfoIndex::SOURCE));
}

void ChessForm::saveInfo()
{
    instance->setInfoValue(InfoIndex::TITLE, ui->titleLineEdit->text());
    instance->setInfoValue(InfoIndex::EVENT, ui->eventLineEdit->text());
    instance->setInfoValue(InfoIndex::DATE, ui->dateLineEdit->text());
    instance->setInfoValue(InfoIndex::SITE, ui->siteLineEdit->text());
    instance->setInfoValue(InfoIndex::RED, ui->redLineEdit->text());
    instance->setInfoValue(InfoIndex::BLACK, ui->blackLineEdit->text());
    instance->setInfoValue(InfoIndex::ECCOSN, ui->eccoSnLineEdit->text());
    instance->setInfoValue(InfoIndex::ECCONAME, ui->eccoNameLineEdit->text());
    instance->setInfoValue(InfoIndex::RESULT, ui->resultLineEdit->text());
    instance->setInfoValue(InfoIndex::WRITER, ui->writerLineEdit->text());
    instance->setInfoValue(InfoIndex::SOURCE, ui->sourceLineEdit->text());
    isModified = true;
    documentWasModified();
}

void ChessForm::resetSize()
{
    int leftWidth = ((ui->leaveGraphicsView->isVisible()
                             ? ui->leaveGraphicsView->width()
                             : 0)
            + ui->boardGraphicsView->width()),
        rightWidth = (ui->infoTabWidget->isVisible()
                ? ui->infoTabWidget->width() + ui->upHorizontalLayout->spacing()
                : 0),
        upHeight = (ui->boardGraphicsView->height()
            + ui->boardVerticalLayout->spacing() + ui->navigateWidget->height()),
        downHeight = (ui->studyTabWidget->isVisible()
                ? ui->studyTabWidget->height() + layout()->spacing()
                : 0);

    ui->navigateWidget->setFixedWidth(leftWidth);
    QSize size(leftWidth + rightWidth, upHeight + downHeight);
    resize(size);
}

void ChessForm::playSound(const QString& fileName)
{
    QSound::play(soundDir.arg(fileName));
}

void ChessForm::initViewScene()
{
    int boardSide = 2;
    ui->leaveGraphicsView->setFixedSize(leftWidth + boardSide, boardHeight + boardSide);
    ui->boardGraphicsView->setFixedSize(boardWidth + boardSide, boardHeight + boardSide);

    ui->leaveGraphicsView->setScene(boardScene);
    ui->boardGraphicsView->setScene(boardScene);

    QRect leftRect(0, 0, leftWidth, boardHeight),
        rightRect(leftWidth, 0, boardWidth, boardHeight);
    ui->leaveGraphicsView->setSceneRect(leftRect);
    ui->boardGraphicsView->setSceneRect(rightRect);
}

void ChessForm::setBtnAction()
{
    // 多个快捷键
    ui->actPreMove->setShortcuts({ Qt::Key_Up, Qt::Key_Left });
    ui->actNextMove->setShortcuts({ Qt::Key_Down, Qt::Key_Right });

    // 棋谱导航
    ui->btnStartMove->setDefaultAction(ui->actStartMove);
    ui->btnPreMove->setDefaultAction(ui->actPreMove);
    ui->btnNextMove->setDefaultAction(ui->actNextMove);
    ui->btnOtherMove->setDefaultAction(ui->actOtherMove);
    ui->btnEndMove->setDefaultAction(ui->actEndMove);

    // 设置状态
    ui->btnAllLeave->setDefaultAction(ui->actAllLeave);
    ui->btnChangeStatus->setDefaultAction(ui->actChangeStatus);
    ui->btnLockInstance->setDefaultAction(ui->actLockInstance);

    // 局部区域隐藏或显示
    ui->btnLeavePiece->setDefaultAction(ui->actLeavePiece);
    ui->btnStudy->setDefaultAction(ui->actStudy);
    ui->btnMoveInfo->setDefaultAction(ui->actMoveInfo);

    // 放弃或保存棋谱信息
    ui->btnShowInfo->setDefaultAction(ui->actShowInfo);
    ui->btnSaveInfo->setDefaultAction(ui->actSaveInfo);
}

void ChessForm::writeSettings() const
{
    QSettings settings;
    settings.beginGroup(stringLiterals[StringIndex::MAINWINDOW]);

    settings.beginGroup(formTitleName);
    settings.setValue(stringLiterals[StringIndex::LEFTSHOW], ui->actLeavePiece->isChecked());
    settings.setValue(stringLiterals[StringIndex::RIGHTSHOW], ui->actMoveInfo->isChecked());
    settings.setValue(stringLiterals[StringIndex::DOWNSHOW], ui->actStudy->isChecked());
    settings.setValue(stringLiterals[StringIndex::RIGHTTABINDEX], ui->infoTabWidget->currentIndex());
    settings.setValue(stringLiterals[StringIndex::DOWNTABINDEX], ui->studyTabWidget->currentIndex());
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
    bool leftShow = settings.value(stringLiterals[StringIndex::LEFTSHOW], true).toBool(),
         rightShow = settings.value(stringLiterals[StringIndex::RIGHTSHOW], true).toBool(),
         downShow = settings.value(stringLiterals[StringIndex::DOWNSHOW], true).toBool();
    ui->actLeavePiece->setChecked(leftShow);
    ui->actMoveInfo->setChecked(rightShow);
    ui->actStudy->setChecked(downShow);
    on_actLeavePiece_triggered(leftShow);
    on_actMoveInfo_triggered(rightShow);
    on_actStudy_triggered(downShow);

    ui->infoTabWidget->setCurrentIndex(settings.value(stringLiterals[StringIndex::RIGHTTABINDEX], 0).toInt());
    ui->studyTabWidget->setCurrentIndex(settings.value(stringLiterals[StringIndex::DOWNTABINDEX], 0).toInt());
    QVariant winGeometry = settings.value(stringLiterals[StringIndex::WINGEOMETRY]);
    if (!winGeometry.isNull())
        getSubWindow()->restoreGeometry(winGeometry.toByteArray());
    moveSound = settings.value(stringLiterals[StringIndex::MOVESOUND], true).toBool();
    soundDir = settings.value(stringLiterals[StringIndex::MOVESOUNDDIR], "./res/SOUNDS/%1").toString();
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

void ChessForm::on_actLockInstance_triggered(bool checked)
{
    instance->setStatus(checked ? InsStatus::MOVEDEMO : InsStatus::PLAY);
    ui->btnChangeStatus->setChecked(!checked);
    ui->actChangeStatus->setEnabled(!checked);
    ui->actLockInstance->setChecked(checked);
}

void ChessForm::on_boardGraphicsView_customContextMenuRequested(const QPoint& pos)
{
    Q_UNUSED(pos)
    QMenu* menu = new QMenu(this);
    menu->addAction(ui->actStartMove);
    menu->addAction(ui->actPreMove);
    menu->addAction(ui->actNextMove);
    menu->addAction(ui->actEndMove);
    menu->addAction(ui->actOtherMove);
    menu->addSeparator();
    menu->addAction(ui->actAllLeave);
    menu->addAction(ui->actChangeStatus);
    menu->addAction(ui->actLockInstance);
    menu->addSeparator();
    menu->addAction(ui->actLeavePiece);
    menu->addAction(ui->actStudy);
    menu->addAction(ui->actMoveInfo);
    menu->exec(QCursor::pos());
    delete menu;
}

void ChessForm::on_leaveGraphicsView_customContextMenuRequested(const QPoint& pos)
{
    Q_UNUSED(pos)
    QMenu* menu = new QMenu(this);
    menu->addAction(ui->actAllLeave);
    menu->addAction(ui->actChangeStatus);
    menu->addSeparator();
    menu->addAction(ui->actLeavePiece);
    menu->exec(QCursor::pos());
    delete menu;
}

void ChessForm::on_infoTabWidget_customContextMenuRequested(const QPoint& pos)
{
    Q_UNUSED(pos)
    QMenu* menu = new QMenu(this);
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
    showInfo();
}

void ChessForm::on_actSaveInfo_triggered()
{
    saveInfo();
}

void ChessForm::on_remarkTextEdit_textChanged()
{
    instance->getCurMove()->setRemark(ui->remarkTextEdit->toPlainText());
}
