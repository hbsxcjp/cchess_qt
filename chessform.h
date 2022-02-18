#ifndef CHESSFORM_H
#define CHESSFORM_H

#include <QWidget>

namespace Ui {
class ChessForm;
}

class ChessForm : public QWidget {
    Q_OBJECT

public:
    explicit ChessForm(QWidget* parent = nullptr);
    ~ChessForm();

private:
    Ui::ChessForm* ui;
};

#endif // CHESSFORM_H
