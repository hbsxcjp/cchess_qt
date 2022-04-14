#ifndef MOVEVIEW_H
#define MOVEVIEW_H

#include <QGraphicsView>

class Move;
using PMove = Move*;

class Instance;
class MoveNodeItem;
enum class MoveNodeItemAlign;

class MoveView : public QGraphicsView {
    Q_OBJECT

public:
    MoveView(QWidget* parent = Q_NULLPTR);

    int getNodeItemNumPerPage() const;

    void setNodeItemLayout(MoveNodeItemAlign align);
    void setInstance(Instance* instance);
    void resetNodeItems();

signals:
    void mousePressed(PMove move);

public slots:
    void updateNodeItemSelected();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    QPointF lastPos;
    int margin_ { 15 };
    int hspacing_ { 30 };
    int vspacing_ { 20 };

    Instance* ins;
    QGraphicsItem* parentItem;
    MoveNodeItem* rootNodeItem;
};

#endif // MOVEVIEW_H
