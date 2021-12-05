#ifndef BOARD_H
#define BOARD_H

#include <QList>
#include <QPair>

class Seat;
class Seats;
using PSeat = Seat*;
using SeatCoord = QPair<int, int>;

class Piece;
using PPiece = Piece*;
class Pieces;
enum class Color;

enum class Side;
enum class ChangeType;

using MovSeat = QPair<PSeat, PSeat>;

class Board {
public:
    Board();
    ~Board();

    void clean();
    void reinit();

    // 棋子可移动位置
    QList<QList<SeatCoord>> canMove(SeatCoord seatCoord);
    QList<SeatCoord> allCanMove(Color color);
    bool isCanMove(SeatCoord fromSeatCoord, SeatCoord toSeatCoord);

    // 某方棋子是否正在被对方将军
    bool isFace() const;
    bool isKilling(Color color);
    bool isFailed(Color color) { return allCanMove(color).count() == 0; }

    QString getFEN() const;
    bool setFEN(const QString& fen);

    void changeLayout(ChangeType ct);

    QString movSeatToStr(const MovSeat& movSeat) const;
    MovSeat strToMovSeat(const QString& zhStr, bool ignoreError = false) const;

    QString toString(bool full = false) const;

private:
    Side getHomeSide_(Color color) const;
    void setBottomColor_();

    Pieces* pieces_;
    Seats* seats_;
    Color bottomColor_;
};

#endif // BOARD_H
