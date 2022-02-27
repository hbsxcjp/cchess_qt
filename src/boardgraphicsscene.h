#ifndef BOARDGRAPHICSSCENE_H
#define BOARDGRAPHICSSCENE_H

#include <QGraphicsScene>

class BoardGraphicsScene : public QGraphicsScene {
public:
    BoardGraphicsScene(int leftWidth, int boardWidth, int boardHeight);

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;

private:
    void addPieceGraphicsItems();

    const int leftWidth_;
    const int boardWidth_;
    const int boardHeight_;
};

#endif // BOARDGRAPHICSSCENE_H
