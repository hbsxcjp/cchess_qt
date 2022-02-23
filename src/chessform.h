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
    explicit ChessForm(QWidget* parent = Q_NULLPTR);
    ~ChessForm();

    void newFile();
    bool loadFile(const QString& fileName);
    bool save();
    bool saveAs();
    bool saveFile(const QString& fileName);

    QString userFriendlyCurrentFile();
    QString getFileName() { return curFileName; }

    static QString getFilter(bool isSave = false);

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void updateForm();
    void updateMoved();
    void updateButtons();

    void on_startBtn_clicked();
    void on_backBtn_clicked();
    void on_goBtn_clicked();
    void on_otherBtn_clicked();
    void on_endBtn_clicked();

    void documentWasModified();

signals:
    void instanceMoved();

private:
    bool maybeSave();
    void setCurrentFile(const QString& fileName);

    bool isUntitled;
    bool isModified;
    QString curFileName;
    Instance* instance;

    Ui::ChessForm* ui;
};

#endif // CHESSFORM_H
