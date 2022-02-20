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
    curFileName = (QString("未命名_%2.%3")
                       .arg(sequenceNumber++)
                       .arg(InstanceIO::getSuffixName(StoreType::PGN_ZH)));
    setWindowTitle(curFileName + "[*]");

    //    connect(document(), &QTextDocument::contentsChanged,            this, &MdiChild::documentWasModified);
}

bool ChessForm::loadFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, "打开棋谱",
            QString("不能打开棋谱文件 %1:\n%2.")
                .arg(fileName)
                .arg(file.errorString()));
        return false;
    }

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);

    bool succeeded = InstanceIO::read(instance, fileName);
    if (succeeded) {
        setCurrentFile(fileName);
        updateForm();
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
    QString fileName = QFileDialog::getSaveFileName(this, "另存为",
        "./", QString("棋谱文件(%1)").arg(InstanceIO::getSuffixNames().join(' ')));
    if (fileName.isEmpty())
        return false;

    return saveFile(fileName);
}

bool ChessForm::saveFile(const QString& fileName)
{
    QString errorMessage;

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

    //    document()->setModified(false);
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
