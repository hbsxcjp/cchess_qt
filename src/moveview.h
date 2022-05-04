#ifndef MOVEVIEW_H
#define MOVEVIEW_H

#include <QGraphicsView>

class Move;
class ChessManual;
class MoveNodeItem;
enum class MoveNodeItemAlign;

class MoveView : public QGraphicsView {
    Q_OBJECT

public:
    MoveView(QWidget* parent = Q_NULLPTR);
    void setManual(ChessManual* manual);

    void setNodeItemLayout(MoveNodeItemAlign align);
    int getNodeItemNumPerPage() const;

signals:
    void mousePressed(Move* move);
    void wheelScrolled(bool isUp);

public slots:
    void resetNodeItems();
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

    ChessManual* manual;
    QGraphicsItem* nodeParentItem;
    MoveNodeItem* rootNodeItem;
};

#endif // MOVEVIEW_H
