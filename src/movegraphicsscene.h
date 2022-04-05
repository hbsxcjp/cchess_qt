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

public slots:
    void updateMoveItemShow();

private:
    int width_ { 640 };
    int height_ { 3200 };

    Instance* instance_;
    QGraphicsItem* parentItem;
    MoveNodeItem* rootNodeItem;
};

#endif // MOVEGRAPHICSSCENE_H
