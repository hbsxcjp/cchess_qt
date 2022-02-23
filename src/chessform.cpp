#include "chessform.h"
#include "instanceio.h"
#include "move.h"
#include "tools.h"
#include "ui_chessform.h"
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>

ChessForm::ChessForm(QWidget* parent)
    : QWidget(parent)
    , isUntitled(true)
    , isModified(false)
    , curFileName(QString())
    , instance(new Instance)
    , ui(new Ui::ChessForm)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Dialog);

    connect(this, &ChessForm::instanceMoved, this, &ChessForm::updateMoved);
    ui->setupUi(this);

    updateForm();
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
    InstanceIO::write(instance, fileName);
    QGuiApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    return true;
}

QString ChessForm::userFriendlyCurrentFile()
{
    return Tools::strippedName(curFileName);
}

QString ChessForm::getFilter(bool isSave)
{
    QStringList filter = InstanceIO::getSuffixNames();
    if (isSave)
        filter.removeFirst(); // 不保存第一种格式

    for (QString& suffix : filter)
        suffix.prepend("*.");

    return QString("棋谱文件(%1)").arg(filter.join(' '));
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

bool ChessForm::maybeSave()
{
    if (!isModified)
        return true;

    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, "保存棋谱",
            QString("'%1' 已被编辑.\n"
                    "需要保存所做的修改吗？")
                .arg(userFriendlyCurrentFile()),
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
    setWindowTitle(userFriendlyCurrentFile() + "[*]");
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
