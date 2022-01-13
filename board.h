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
enum class Kind;

enum class Side;
enum class ChangeType;

using MovSeat = QPair<PSeat, PSeat>;

class Board {
public:
    Board();
    ~Board();

    void clear();
    void initFEN();

    PPiece getPiece(SeatCoord seatCoord) const;
    QList<SeatCoord> getLiveSeatCoordList(Color color) const;

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

    MovSeat getMovSeat_rowcols(const QString& rowcols) const;
    MovSeat getMovSeat(QPair<SeatCoord, SeatCoord> seatCoordlPair) const;

    QString toString(bool full = false) const;

private:
    PSeat getSeat_(int row, int col) const;
    PSeat getSeat_(SeatCoord seatCoord) const;

    // 取得与棋子特征有关的位置
    PSeat getKingSeat_(Color color) const;
    QList<PSeat> getLiveSeatList_(Color color) const;
    QList<PSeat> getLiveSeatList_(Color color, Kind kind) const;
    QList<PSeat> getLiveSeatList_(Color color, QChar name) const;
    QList<PSeat> getLiveSeatList_(Color color, QChar name, int col) const;
    QList<PSeat> getSortPawnLiveSeatList_(Color color, bool isBottom) const;

    static bool isKindName_(QChar name, QList<Kind> kinds);

    QList<SeatCoord> filterKillSeatCoord_(PSeat fromSeat, QList<SeatCoord>& seatCoordList) const;

    Side getHomeSide_(Color color) const;
    void setBottomColor_();

    Pieces* pieces_;
    Seats* seats_;
    Color bottomColor_;
};

#endif // BOARD_H
