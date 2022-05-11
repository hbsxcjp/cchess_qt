#ifndef BOARDVIEW_H
#define BOARDVIEW_H

#include <QGraphicsView>

#define LEFTWIDTH 200
#define BOARDWIDTH 521
#define BOARDHEIGHT 577
#define SCENESTARTX 3
#define BOARDSTARTX (LEFTWIDTH + SCENESTARTX)
#define SCENESTARTY 3

#define NOTBOARDINDEX (-1)
#define HINTZVALUE 4
#define MOVEZVALUE 8
#define OUTSIZE (-1000)

class Manual;
class PieceItem;
class BoardScene;

enum class PieceColor;
enum class ManualStatus;

using Coord = QPair<int, int>;

class BoardView : public QGraphicsView {
    Q_OBJECT

public:
    BoardView(QWidget* parent = Q_NULLPTR);
    ~BoardView();

    void setManual(Manual* manual);
    QRect boardSceneRect() const { return QRect(LEFTWIDTH, 0, BOARDWIDTH, BOARDHEIGHT); }

    QString getBackImageFile() const { return backImageFile; }
    QString getPieceImageDir() const { return pieceImageDir; }
    bool getLeaveIsTidy() const { return leaveIsTidy; }
    bool getMoveAnimated() const { return moveAnimated; }

    bool atBoard(const QPointF& pos) const;
    void allPieceToLeave();

    QPointF getLimitPos(const QPointF& pos) const;
    QPointF getSeatPos(const QPointF& pos) const;
    bool canMovePos(const QPointF& fromPos, const QPointF& toPos, const PieceItem* item) const;

    void showHint(const QPointF& scenePos, PieceItem* item);
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

    Manual* manual;
};

#endif // BOARDVIEW_H
