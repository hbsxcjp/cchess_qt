#ifndef BOARD_H
#define BOARD_H

#include <QList>

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

    QList<PSeat> getLiveSeatList(Color color) const;

    // 棋子可移动位置
    // 1.可移动位置；2.规则已排除位置；3.同色已排除位置；4.将帅对面或被将军已排除位置
    QList<QList<SeatCoord>> canMove(SeatCoord seatCoord) const;
    QMap<PSeat, QList<SeatCoord>> allCanMove(Color color) const;
    bool isCanMove(SeatCoord fromSeatCoord, SeatCoord toSeatCoord) const;
    bool isCanMove(const MovSeat& movSeat) const;

    // 某方棋子是否正在被对方将军
    bool isFace() const;
    bool isKilling(Color color) const;
    bool isFailed(Color color) const;

    QString getFEN() const;
    bool setFEN(const QString& fen);

    MovSeat getChangeMovSeat(MovSeat movSeat, ChangeType ct) const;
    void changeLayout(ChangeType ct);

    QString getZhStr(const MovSeat& movSeat, bool ignoreError = false) const;
    MovSeat getMovSeat(const QString& zhStr, bool ignoreError = false) const;
    MovSeat getMovSeat(int rowcols) const;
    MovSeat getMovSeat(QPair<SeatCoord, SeatCoord> seatCoordlPair) const;

    QString getZhChars() const;
    QString toString(bool full = false) const;

private:
    QList<SeatCoord> filterKillSeatCoord_(PSeat fromSeat, QList<SeatCoord>& seatCoordList) const;

    Side getHomeSide_(Color color) const;
    void setBottomColor_();

    Pieces* pieces_;
    Seats* seats_;
    Color bottomColor_;
};

#endif // BOARD_H
