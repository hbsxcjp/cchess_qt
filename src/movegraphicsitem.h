#ifndef MOVEGRAPHICSITEM_H
#define MOVEGRAPHICSITEM_H

#include <QFont>
#include <QGraphicsItem>

class Move;
using PMove = Move*;
class MoveGraphicsScene;

class MoveNodeItem : public QGraphicsItem {
public:
    MoveNodeItem(PMove move, MoveGraphicsScene* moveScene, QGraphicsItem* parent);

    //
    void addMoveNodeItem(QGraphicsItem* parent);
    void updateNodeItemShow();

    MoveNodeItem* nextItem() const { return nextNodeItem_; };
    MoveNodeItem* otherItem() const { return otherNodeItem_; };

protected:
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    QPainterPath shape() const override;

private:
    QRectF outlineRect() const;
    int roundness(double size) const;

    PMove move_;
    MoveGraphicsScene* moveScene_;

    QString text_;
    QFont font_;
    QColor textColor;
    QColor backgroundColor;
    QColor outlineColor;

    MoveNodeItem* nextNodeItem_;
    MoveNodeItem* otherNodeItem_;
};

#endif // MOVEGRAPHICSITEM_H
