#ifndef BOARDGRAPHICSSCENE_H
#define BOARDGRAPHICSSCENE_H

#include <QGraphicsScene>

#define NOTBOARDINDEX (-1)

class Instance;
class PieceGraphicsItem;
enum class PieceColor;
enum class SeatSide;

enum class InsStatus;

using SeatCoord = QPair<int, int>;

class BoardGraphicsScene : public QGraphicsScene {
    Q_OBJECT

public:
    BoardGraphicsScene(int leftWidth, int boardWidth, int boardHeight, Instance* ins);
    ~BoardGraphicsScene();

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
    void updatePieceItemPos();

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

    const int leftWidth_, boardWidth_, boardHeight_;
    qreal posStartX, posBoardStartX, posStartY, pieceDiameter, halfDiameter;

    bool leaveIsTidy, moveAnimated;
    QString backImageFile;

    Instance* instance;
    PieceGraphicsItem* shadowItem;
    QList<PieceGraphicsItem*> pieceItemList;
    QGraphicsItem *hintParentItem, *pieceParentItem;
};

#endif // BOARDGRAPHICSSCENE_H
