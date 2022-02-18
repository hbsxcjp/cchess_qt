#include "chessform.h"
#include "ui_chessform.h"

ChessForm::ChessForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ChessForm)
{
    ui->setupUi(this);
}

ChessForm::~ChessForm()
{
    delete ui;
}
