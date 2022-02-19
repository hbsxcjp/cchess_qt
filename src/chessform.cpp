#include "chessform.h"
#include "instanceio.h"
#include "move.h"
#include "ui_chessform.h"
#include <QTimer>

ChessForm::ChessForm(const QString& fileName, QWidget* parent)
    : QWidget(parent)
    , fileName_(fileName)
    , instance_(new Instance)
    , ui(new Ui::ChessForm)
{
    connect(this, &ChessForm::instanceMoved, this, &ChessForm::updateForm);
    ui->setupUi(this);

    QTimer::singleShot(0, this, &ChessForm::loadFile);
}

ChessForm::~ChessForm()
{
    delete instance_;
    delete ui;
}

void ChessForm::loadFile()
{
    readFile(fileName_);
    updateForm();
}

void ChessForm::saveFile()
{
    writeFile(fileName_);
}

void ChessForm::readFile(const QString& fileName)
{
    if (fileName.isEmpty())
        return;

    delete instance_;
    instance_ = new Instance;
    InstanceIO::read(instance_, fileName);
}

void ChessForm::writeFile(const QString& fileName)
{
    if (fileName.isEmpty())
        return;

    InstanceIO::write(instance_, fileName);
}

void ChessForm::on_startBtn_clicked()
{
    instance_->backStart();
    emit instanceMoved();
}

void ChessForm::on_backBtn_clicked()
{
    instance_->backOne();
    emit instanceMoved();
}

void ChessForm::on_goBtn_clicked()
{
    instance_->goNext();
    emit instanceMoved();
}

void ChessForm::on_otherBtn_clicked()
{
    instance_->goOther();
    emit instanceMoved();
}

void ChessForm::on_endBtn_clicked()
{
    instance_->goEnd();
    emit instanceMoved();
}

void ChessForm::updateForm()
{

    updateButtons_();
}

void ChessForm::updateButtons_()
{
    bool isStart = instance_->isStartMove(),
         isEnd = instance_->isEndMove(),
         hasOther = instance_->hasOtherMove();
    ui->startBtn->setEnabled(!isStart);
    ui->backBtn->setEnabled(!isStart);
    ui->goBtn->setEnabled(!isEnd);
    ui->endBtn->setEnabled(!isEnd);
    ui->otherBtn->setEnabled(hasOther);
}
