#ifndef MOVEVIEW_H
#define MOVEVIEW_H

#include <QGraphicsView>

class Move;
class Manual;
class MoveNodeItem;
class ManualSubWindow;

enum class MoveNodeItemAlign;

class MoveView : public QGraphicsView {
    Q_OBJECT

public:
    MoveView(QWidget* parent = Q_NULLPTR);
    void setManualSubWindow(ManualSubWindow* manualSubWindow);

    void setNodeItemLayout(MoveNodeItemAlign align);
    int getNodeItemNumPerPage() const;

signals:
    void mousePressed(Move* move);
    void wheelScrolled(bool isUp);

public slots:
    void resetNodeItems();
    void updateNodeItemSelected();

protected:
    void mouseDoubleClickEvent(QMouseEvent* /*event*/) override { } // 覆盖默认行为
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    QPointF lastPos;
    int margin_ { 15 };
    int hspacing_ { 30 };
    int vspacing_ { 20 };

    QGraphicsItem* nodeParentItem;
    MoveNodeItem* rootNodeItem;

    ManualSubWindow* manualSubWindow_;
};

#endif // MOVEVIEW_H
