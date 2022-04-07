#ifndef MOVEGRAPHICSSCENE_H
#define MOVEGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QObject>

class Instance;
class MoveNodeItem;

class MoveGraphicsScene : public QGraphicsScene {
    Q_OBJECT

public:
    MoveGraphicsScene(Instance* ins);

    MoveNodeItem* getRootNodeItem() { return rootNodeItem; }

public slots:
    void resetMoveNodeItem();
    void setCurMoveSelected();

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;

private:
    int width_ { 1600 };
    int height_ { 6400 };

    Instance* instance_;
    QGraphicsItem* parentItem;
    MoveNodeItem* rootNodeItem;
};

#endif // MOVEGRAPHICSSCENE_H
