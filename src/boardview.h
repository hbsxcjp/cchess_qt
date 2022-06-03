#ifndef BOARDVIEW_H
#define BOARDVIEW_H

#include <QGraphicsView>

class Piece;

class Manual;
class PieceItem;
class BoardScene;
class ManualSubWindow;

enum class PieceColor;

using Coord = QPair<int, int>;

class BoardView : public QGraphicsView {
    Q_OBJECT

public:
    BoardView(QWidget* parent = Q_NULLPTR);
    ~BoardView();

    void setManualSubWindow(ManualSubWindow* manualSubWindow);
    QRect boardRect() const;

    QString getBackImageFile() const { return backImageFile; }
    QString getPieceImageDir() const { return pieceImageDir; }
    bool getLeaveIsTidy() const { return leaveIsTidy; }
    bool getMoveAnimated() const { return moveAnimated; }

    bool atBoard(const QPointF& pos) const;
    void allPieceToLeave();

    QPointF getLimitPos(const QPointF& seatPos) const;
    QPointF getMovedPos(Piece* piece, const QPointF& fromSeatPos,
        const QPointF& toPos, const QPointF& mousePos) const;

    void showHintItem(Piece* piece, const QPointF& fromPos);
    void clearHintItem();
signals:

public slots:
    void updatePieceItemShow();

protected:
    //    void mousePressEvent(QMouseEvent* event) override;
    //    void mouseMoveEvent(QMouseEvent* event) override;
    //    void mouseReleaseEvent(QMouseEvent* event) override;

    //    void focusInEvent(QFocusEvent* event) override;
    //    void focusOutEvent(QFocusEvent* event) override;

private:
    void writeSettings() const;
    void readSettings();

    void creatMarginItems();
    void creatPieceItems();
    QList<PieceItem*> getPieceItems() const;

    QPointF getSeatPos(int index) const;
    QPointF getSeatPos(const Coord& coord) const;

    Coord getCoord(const QPointF& pos) const;
    static QPointF getScenePos(const Coord& showCoord,
        qreal startX, qreal spacingX, qreal startY, qreal spacingY);

    bool leaveIsTidy;
    bool moveAnimated;
    QString backImageFile;
    QString pieceImageDir;

    QGraphicsItem* hintParentItem;
    QGraphicsItem* pieceParentItem;
    QGraphicsPixmapItem* shadowItem;

    ManualSubWindow* manualSubWindow_;
};

#endif // BOARDVIEW_H
