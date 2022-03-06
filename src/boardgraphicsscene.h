#ifndef BOARDGRAPHICSSCENE_H
#define BOARDGRAPHICSSCENE_H

#include <QGraphicsScene>

class Instance;
class PieceGraphicsItem;
enum class PieceColor;
enum class SeatSide;

enum class InsStatus {
    INITLAYOUT,
    MOVEDEMO,
    PLAY
};

using SeatCoord = QPair<int, int>;

class BoardGraphicsScene : public QGraphicsScene {
public:
    BoardGraphicsScene(int leftWidth, int boardWidth, int boardHeight, Instance* ins);
    ~BoardGraphicsScene();

    bool atBoard(const QPointF& pointf) const;
    bool atLeave(const QPointF& pointf) const;
    QPointF getBoardPointF(const QPointF& itemPointf) const;
    QPointF getLeavePointF(const QPointF& itemPointf) const;

    QPointF getHintPointF(const SeatCoord& seatCoord) const;
    QPointF getSeatPointF(const SeatCoord& seatCoord) const;
    SeatCoord getSeatCoord(const QPointF& pointf) const;

    void loadPieceItems();
    void setPieceItemImageTheme(const QString& pieceImageDir);
    void setBoardImageTheme(const QString& imageFile);

    InsStatus getInsStatus() const { return insStatus; }
    QList<SeatCoord> getPutSeatCoordList(PieceGraphicsItem* item) const;
    void setHintItemList(QList<SeatCoord> seatCoordList);
    void clearHintItemList();

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;

private:
    void writeSettings() const;
    void readSettings();

    PieceGraphicsItem* getLeavePieceItem(QChar ch) const;
    void addPieceGraphicsItems(const QString& chemeDirName);

    QPointF getLimitCenterPointF(const QPointF& itemPointf, int startX, int width, int height) const;
    QPointF getLimitOriginPointF(const QPointF& itemPointf) const;

    QPointF getLeavePointF(const SeatCoord& seatCoord) const;
    SeatCoord getLeaveCoord(const QPointF& pointf) const;
    static SeatCoord showSeatCoord(const SeatCoord& seatCoord);

    static QPointF getPointF(const SeatCoord& seatCoord,
        qreal startX, qreal spacingX, qreal startY, qreal spacingY);
    static SeatCoord getCoord(const QPointF& pointf,
        qreal startX, qreal spacingX, qreal startY, qreal spacingY);

    void creatPieceItems();
    void setPieceItemPoses();

    int getLeavePieceNum(PieceColor color);
    QPointF getNextLeavePointf(PieceColor color);

    const int leftWidth_, boardWidth_, boardHeight_;
    qreal pointStartX, pointBoardStartX, pointStartY, pieceDiameter, halfDiameter;

    bool leaveIsTidy, moveAnimated, moveSound;
    QString backImageFile, pieceImageDir;
    InsStatus insStatus;

    Instance* instance;
    QList<PieceGraphicsItem*> pieceItemList;
    QList<QGraphicsItem*> hintItemList;
};

#endif // BOARDGRAPHICSSCENE_H
