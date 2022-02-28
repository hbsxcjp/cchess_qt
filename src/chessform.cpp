#include "chessform.h"
#include "instanceio.h"
#include "move.h"
#include "tools.h"
#include "ui_chessform.h"
#include <QCloseEvent>
#include <QFileDialog>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QPainter>
#include <QTimer>

static const QStringList stringLiterals {
    QStringLiteral("showOption"),
    QStringLiteral("leftShow"),
    QStringLiteral("rightShow"),
    QStringLiteral("downShow"),
    QStringLiteral("rightTabIndex"),
    QStringLiteral("downTabIndex")
};

enum StringLiteralIndex {
    SHOWOPTION,
    LEFTSHOW,
    RIGHTSHOW,
    DOWNSHOW,
    RIGHTTABINDEX,
    DOWNTABINDEX
};

ChessForm::ChessForm(QWidget* parent)
    : QWidget(parent)
    , isUntitled(true)
    , isModified(false)
    , curFileName(QString())
    , instance(new Instance)
    , ui(new Ui::ChessForm)
{
    ui->setupUi(this);
    connect(this, &ChessForm::instanceMoved, this, &ChessForm::updateMoved);

    bigBoardRect.setRect(0, 0, leftWidth + boardWidth, boardHeight);
    smallBoardRect.setRect(leftWidth, 0, boardWidth, boardHeight);

    boardScene = new BoardGraphicsScene(leftWidth, boardWidth, boardHeight);
    ui->boardGraphicsView->setScene(boardScene);
    ui->boardGraphicsView->setSceneRect(smallBoardRect);
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

QString ChessForm::getFriendlyFileName()
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

void ChessForm::writeSettings(QSettings& settings)
{
    settings.beginGroup(stringLiterals[StringLiteralIndex::SHOWOPTION]);

    settings.setValue(stringLiterals[StringLiteralIndex::LEFTSHOW], ui->leftBtn->isChecked());
    settings.setValue(stringLiterals[StringLiteralIndex::RIGHTSHOW], ui->rightBtn->isChecked());
    settings.setValue(stringLiterals[StringLiteralIndex::DOWNSHOW], ui->downBtn->isChecked());
    settings.setValue(stringLiterals[StringLiteralIndex::RIGHTTABINDEX], ui->infoTabWidget->currentIndex());
    settings.setValue(stringLiterals[StringLiteralIndex::DOWNTABINDEX], ui->studyTabWidget->currentIndex());

    settings.endGroup();
}

void ChessForm::readSettings(QSettings& settings)
{
    settings.beginGroup(stringLiterals[StringLiteralIndex::SHOWOPTION]);

    ui->leftBtn->setChecked(settings.value(stringLiterals[StringLiteralIndex::LEFTSHOW]).toBool());
    ui->rightBtn->setChecked(settings.value(stringLiterals[StringLiteralIndex::RIGHTSHOW]).toBool());
    ui->downBtn->setChecked(settings.value(stringLiterals[StringLiteralIndex::DOWNSHOW]).toBool());
    ui->infoTabWidget->setCurrentIndex(settings.value(stringLiterals[StringLiteralIndex::RIGHTTABINDEX]).toInt());
    ui->studyTabWidget->setCurrentIndex(settings.value(stringLiterals[StringLiteralIndex::DOWNTABINDEX]).toInt());

    settings.endGroup();
}

void ChessForm::closeEvent(QCloseEvent* event)
{
    if (maybeSave()) {
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
    updateButtons();
}

void ChessForm::updateMoved()
{
}

void ChessForm::updateButtons()
{
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

bool ChessForm::maybeSave()
{
    if (!isModified)
        return true;

    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, "保存棋谱",
            QString("'%1' 已被编辑.\n"
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
    setWindowModified(false);
    setWindowTitle(getFriendlyFileName() + "[*]");
}

void ChessForm::on_actNextMove_triggered()
{
    instance->goNext();
}

void ChessForm::on_leftBtn_toggled(bool checked)
{
    int width = checked ? bigBoardRect.width() : smallBoardRect.width();
    ui->navigateWidget->setFixedWidth(width);
    ui->boardGraphicsView->setFixedWidth(width);
    ui->boardGraphicsView->setSceneRect(checked ? bigBoardRect : smallBoardRect);

    QMdiSubWindow* subWindow = qobject_cast<QMdiSubWindow*>(parent());
    subWindow->resize(subWindow->width() + (checked ? leftWidth : -leftWidth), subWindow->height());
}

void ChessForm::on_downBtn_toggled(bool checked)
{
    int height = ui->studyTabWidget->height() + 6; // 6：布局的内部组件间隔距离
    ui->studyTabWidget->setVisible(checked);
    QMdiSubWindow* subWindow = qobject_cast<QMdiSubWindow*>(parent());
    subWindow->resize(subWindow->width(), subWindow->height() + (checked ? height : -height));
}

void ChessForm::on_rightBtn_toggled(bool checked)
{
    int width = ui->infoTabWidget->width() + 6; // 6：布局的内部组件间隔距离
    ui->infoTabWidget->setVisible(checked);
    QMdiSubWindow* subWindow = qobject_cast<QMdiSubWindow*>(parent());
    subWindow->resize(subWindow->width() + (checked ? width : -width), subWindow->height());
}
