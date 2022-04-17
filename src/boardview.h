#ifndef BOARDVIEW_H
#define BOARDVIEW_H

#include <QGraphicsView>

class Instance;
class PieceItem;
class BoardScene;

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
    QRect boardSceneRect() const { return QRect(leftWidth_, 0, boardWidth_, boardHeight_); }

    QString getBackImageFile() const { return backImageFile; }
    QString getPieceImageDir() const { return pieceImageDir; }
    bool getLeaveIsTidy() const { return leaveIsTidy; }
    bool getMoveAnimated() const { return moveAnimated; }

    bool atBoard(const QPointF& pos) const;
    void allPieceToLeave();

signals:

public slots:
    void updatePieceItemShow();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

    //    void focusInEvent(QFocusEvent* event) override;
    //    void focusOutEvent(QFocusEvent* event) override;

private:
    bool canMovePos(const QPointF& fromPos, const QPointF& toPos, QChar ch) const;

    PieceItem* getPieceItemAt(const QPointF& pos) const;
    void showHintItem(const QPointF& scenePos, QChar ch);
    void clearHintItem();

    void writeSettings() const;
    void readSettings();

    void creatPieceItems();

    QPointF getLimitPos(const QPointF& pos) const;
    QPointF getSeatPos(int index) const;
    QPointF getSeatPos(const QPointF& pos) const;
    QPointF getSeatPos(const SeatCoord& seatCoord) const;
    int getBoardIndex(const QPointF& pos) const;

    SeatCoord getSeatCoord(const QPointF& pos) const;
    static QPointF getScenePos(const SeatCoord& showCoord,
        qreal startX, qreal spacingX, qreal startY, qreal spacingY);

    static const int leftWidth_ { 200 };
    static const int boardWidth_ { 521 };
    static const int boardHeight_ { 577 };
    const qreal posStartX { 4 };
    const qreal posBoardStartX { leftWidth_ + posStartX };
    const qreal posStartY { 3.5 };

    QPointF oldPos;
    QPoint mousePos;

    bool leaveIsTidy;
    bool moveAnimated;
    QString backImageFile;
    QString pieceImageDir;

    PieceItem* shadowItem;
    QGraphicsItem* hintParentItem;
    QGraphicsItem* pieceParentItem;
    QList<PieceItem*> pieceItemList;

    Instance* instance_;
};

#endif // BOARDVIEW_H
