#ifndef CHESSFORM_H
#define CHESSFORM_H

#include "instance.h"
#include <QWidget>

namespace Ui {
class ChessForm;
}

class ChessForm : public QWidget {
    Q_OBJECT

public:
    explicit ChessForm(const QString& fileName = QString(), QWidget* parent = Q_NULLPTR);
    ~ChessForm();

    void loadFile();
    void saveFile();

    void readFile(const QString& fileName);
    void writeFile(const QString& fileName);

private slots:
    void updateForm();

    void on_startBtn_clicked();

    void on_backBtn_clicked();

    void on_goBtn_clicked();

    void on_otherBtn_clicked();

    void on_endBtn_clicked();

signals:
    void instanceMoved();

private:
    void updateButtons_();

    QString fileName_;
    Instance* instance_;

    Ui::ChessForm* ui;
};

#endif // CHESSFORM_H
