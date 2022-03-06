#include "chessform.h"
#include "instanceio.h"
#include "move.h"
#include "publicString.h"
#include "tools.h"
#include "ui_chessform.h"
#include <QCloseEvent>
#include <QFileDialog>
#include <QMdiArea>
#include <QMessageBox>
#include <QPainter>
#include <QTimer>

ChessForm::ChessForm(QWidget* parent)
    : QWidget(parent)
    , isUntitled(true)
    , isModified(false)
    , curFileName(QString())
    , instance(new Instance)
    , ui(new Ui::ChessForm)
{
    ui->setupUi(this);
    //    connect(this, &ChessForm::instanceMoved, this, &ChessForm::updateMoved);

    int boardSide = 2;
    ui->leaveGraphicsView->setFixedSize(leftWidth + boardSide, boardHeight + boardSide);
    ui->boardGraphicsView->setFixedSize(boardWidth + boardSide, boardHeight + boardSide);

    boardScene = new BoardGraphicsScene(leftWidth, boardWidth, boardHeight, instance);
    ui->leaveGraphicsView->setScene(boardScene);
    ui->boardGraphicsView->setScene(boardScene);

    QRect leftRect(0, 0, leftWidth, boardHeight),
        rightRect(leftWidth, 0, boardWidth, boardHeight);
    ui->leaveGraphicsView->setSceneRect(leftRect);
    ui->boardGraphicsView->setSceneRect(rightRect);
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
    curFileName = (QString("未命名%2.%3")
                       .arg(sequenceNumber++)
                       .arg(InstanceIO::getSuffixName(StoreType::PGN_ZH)));
    setWindowTitle(curFileName + "[*]");
    //    connect(document(), &QTextDocument::contentsChanged,            this, &MdiChild::documentWasModified);

    updateForm();
}

bool ChessForm::loadFile(const QString& fileName)
{
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    bool succeeded = InstanceIO::read(instance, fileName);
    if (succeeded) {
        setCurrentFile(fileName);
        readSettings();
        updateForm();
    } else {
        QMessageBox::warning(this, "打开棋谱",
            QString("不能打开棋谱文件 %1\n请检查文件是否存在，文件是否为棋谱类型？\n")
                .arg(fileName));
    }
    QGuiApplication::restoreOverrideCursor();
    //    connect(document(), &QTextDocument::contentsChanged, this, &MdiChild::documentWasModified);

    return succeeded;
}

bool ChessForm::save()
{
    if (isUntitled) {
        return saveAs();
    } else {
        return saveFile(curFileName);
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
        setCurrentFile(fileName);

    return succeeded;
}

bool ChessForm::needNotSave() const
{
    return !isUntitled && !isModified;
}

QString ChessForm::getFriendlyFileName() const
{
    return QFileInfo(curFileName).fileName();
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

void ChessForm::on_startBtn_clicked()
{
    instance->backStart();
    emit instanceMoved();
}

void ChessForm::on_backBtn_clicked()
{
    instance->backOne();
    emit instanceMoved();
}

void ChessForm::on_goBtn_clicked()
{
    instance->goNext();
    emit instanceMoved();
}

void ChessForm::on_otherBtn_clicked()
{
    instance->goOther();
    emit instanceMoved();
}

void ChessForm::on_endBtn_clicked()
{
    instance->goEnd();
    emit instanceMoved();
}

void ChessForm::documentWasModified()
{
    setWindowModified(isModified);
}

void ChessForm::updateForm()
{
    boardScene->loadPieceItems();

    bool hasInstance = bool(instance),
         isStart = hasInstance && instance->isStartMove(),
         isEnd = hasInstance && instance->isEndMove(),
         hasOther = hasInstance && instance->hasOtherMove();
    ui->startBtn->setEnabled(hasInstance && !isStart);
    ui->backBtn->setEnabled(hasInstance && !isStart);
    ui->goBtn->setEnabled(hasInstance && !isEnd);
    ui->endBtn->setEnabled(hasInstance && !isEnd);
    ui->otherBtn->setEnabled(hasInstance && hasOther);
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
    ui->moveTextEdit->setPlainText(QString("x: %1, y: %2").arg(event->x()).arg(event->y()));
}

void ChessForm::on_actNextMove_triggered()
{
    instance->goNext();
}

void ChessForm::on_leftBtn_toggled(bool checked)
{
    ui->leaveGraphicsView->setVisible(checked);
    resetSize();
}

void ChessForm::on_rightBtn_toggled(bool checked)
{
    ui->infoTabWidget->setVisible(checked);
    resetSize();
}

void ChessForm::on_downBtn_toggled(bool checked)
{
    ui->studyTabWidget->setVisible(checked);
    resetSize();
}

QMdiSubWindow* ChessForm::getSubWindow() const
{
    return qobject_cast<QMdiSubWindow*>(parent());
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
    //    getSubWindow()->resize(size);
}

void ChessForm::writeSettings() const
{
    QSettings settings;
    settings.beginGroup(stringLiterals[StringIndex::MAINWINDOW]);
    QStringList fileNameList = settings.value(stringLiterals[StringIndex::ACTIVEFILENAMES], {}).value<QStringList>();
    fileNameList.append(curFileName);
    settings.setValue(stringLiterals[StringIndex::ACTIVEFILENAMES], fileNameList);

    settings.beginGroup(curFileName);
    settings.setValue(stringLiterals[StringIndex::LEFTSHOW], ui->leftBtn->isChecked());
    settings.setValue(stringLiterals[StringIndex::RIGHTSHOW], ui->rightBtn->isChecked());
    settings.setValue(stringLiterals[StringIndex::DOWNSHOW], ui->downBtn->isChecked());
    settings.setValue(stringLiterals[StringIndex::RIGHTTABINDEX], ui->infoTabWidget->currentIndex());
    settings.setValue(stringLiterals[StringIndex::DOWNTABINDEX], ui->studyTabWidget->currentIndex());
    settings.setValue(stringLiterals[StringIndex::WINGEOMETRY], getSubWindow()->saveGeometry());
    settings.endGroup();

    settings.endGroup();
}

void ChessForm::readSettings()
{
    QSettings settings;
    settings.beginGroup(stringLiterals[StringIndex::MAINWINDOW]);

    settings.beginGroup(curFileName);
    ui->leftBtn->setChecked(settings.value(stringLiterals[StringIndex::LEFTSHOW], true).toBool());
    ui->rightBtn->setChecked(settings.value(stringLiterals[StringIndex::RIGHTSHOW], true).toBool());
    ui->downBtn->setChecked(settings.value(stringLiterals[StringIndex::DOWNSHOW], true).toBool());
    ui->infoTabWidget->setCurrentIndex(settings.value(stringLiterals[StringIndex::RIGHTTABINDEX], 0).toInt());
    ui->studyTabWidget->setCurrentIndex(settings.value(stringLiterals[StringIndex::DOWNTABINDEX], 0).toInt());
    QVariant winGeometry = settings.value(stringLiterals[StringIndex::WINGEOMETRY]);
    if (!winGeometry.isNull())
        getSubWindow()->restoreGeometry(winGeometry.toByteArray());
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
                .arg(getFriendlyFileName()),
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

void ChessForm::setCurrentFile(const QString& fileName)
{
    curFileName = QFileInfo(fileName).canonicalFilePath();
    isUntitled = false;
    isModified = false;
    setWindowModified(false);
    setWindowTitle(getFriendlyFileName() + "[*]");
}
