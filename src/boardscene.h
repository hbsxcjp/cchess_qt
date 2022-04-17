#ifndef BOARDSCENE_H
#define BOARDSCENE_H

#include <QGraphicsScene>

class BoardScene : public QGraphicsScene {
public:
    using QGraphicsScene::QGraphicsScene;

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;
};

#endif // BOARDSCENE_H
