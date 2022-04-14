#ifndef BOARDSCENE_H
#define BOARDSCENE_H

#include <QGraphicsScene>

#define NOTBOARDINDEX (-1)

class Instance;
class PieceItem;
enum class PieceColor;
enum class SeatSide;

enum class InsStatus;

using SeatCoord = QPair<int, int>;

class BoardScene : public QGraphicsScene {
    Q_OBJECT

public:
    BoardScene(Instance* ins, QObject* parent = Q_NULLPTR);
    ~BoardScene();

    int boardWidth() const { return boardWidth_; }
    QRectF boardSceneRect() const { return QRectF(leftWidth_, 0, boardWidth_, boardHeight_); }
    QRectF leaveSceneRect() const { return QRectF(0, 0, leftWidth_, boardHeight_); }

    QPointF getLimitPos(const QPointF& pointf) const;
    QPointF getSeatPos(int index) const;
    QPointF getSeatPos(const QPointF& pos) const;
    int getBoardIndex(const QPointF& pos) const;
    QPointF getCenterSeatPos(const QPointF& pos) const;
    SeatCoord getSeatCoord(const QPointF& pointf) const;

    bool atBoard(const QPointF& pos) const;
    bool canMovePos(const QPointF& fromPos, const QPointF& toPos, QChar ch) const;

    void showHintItem(const QPointF& scenePos, QChar ch);
    void clearHintItem();

    void allPieceToLeave();

public slots:
    void updatePieceItemShow();

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;

private:
    void writeSettings() const;
    void readSettings();

    void creatPieceItems();

    QPointF getSeatPos(const SeatCoord& seatCoord) const;
    QList<SeatCoord> getPutSeatCoordList(QChar ch) const;
    QList<SeatCoord> getMoveSeatCoordList(const SeatCoord& seatCoord) const;
    void showHintItem(QList<SeatCoord> seatCoordList);

    static SeatCoord getSeatCoord(int colX, int rowY);
    static QPointF getScenePos(const SeatCoord& seatCoord,
        qreal startX, qreal spacingX, qreal startY, qreal spacingY);

    const int leftWidth_ { 200 };
    const int boardWidth_ { 521 };
    const int boardHeight_ { 577 };
    const qreal posStartX { 4 };
    const qreal posBoardStartX { leftWidth_ + posStartX };
    const qreal posStartY { 3.5 };
    const qreal pieceDiameter { 57 };
    const qreal halfDiameter { pieceDiameter / 2 };

    bool leaveIsTidy;
    bool moveAnimated;
    QString backImageFile;

    Instance* instance;
    PieceItem* shadowItem;
    QList<PieceItem*> pieceItemList;
    QGraphicsItem* hintParentItem;
    QGraphicsItem* pieceParentItem;
};

#endif // BOARDSCENE_H
