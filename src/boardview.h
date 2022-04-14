#ifndef BOARDGRAPHICSVIEW_H
#define BOARDGRAPHICSVIEW_H

#include <QGraphicsView>

#define NOTBOARDINDEX (-1)

class Instance;
class BoardScene;
class PieceItem;

enum class PieceColor;
enum class SeatSide;

enum class InsStatus;

using SeatCoord = QPair<int, int>;

class BoardView : public QGraphicsView {
    Q_OBJECT

public:
    BoardView(QWidget* parent = Q_NULLPTR);
    ~BoardView();

    void setInstance(Instance* instance);

public slots:
    void updatePieceItemShow();

private:
};

#endif // BOARDGRAPHICSVIEW_H
