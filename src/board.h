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
enum class PieceColor;
enum class PieceKind;

enum class SeatSide;
enum class ChangeType;

using MovSeat = QPair<PSeat, PSeat>;

class Board {
public:
    Board();
    ~Board();

    void clear();
    void initFEN();

    PPiece getPiece(SeatCoord seatCoord) const;
    QList<SeatCoord> getLiveSeatCoordList(PieceColor color) const;

    // 棋子可移动位置
    // 1.可移动位置；2.规则已排除位置；3.同色已排除位置；4.将帅对面或被将军已排除位置
    QList<QList<SeatCoord>> canMove(SeatCoord seatCoord) const;
    QMap<PSeat, QList<SeatCoord>> allCanMove(PieceColor color) const;
    bool isCanMove(SeatCoord fromSeatCoord, SeatCoord toSeatCoord) const;
    bool isCanMove(const MovSeat& movSeat) const;

    // 某方棋子是否正在被对方将军
    bool isFace() const;
    bool isKilling(PieceColor color) const;
    bool isFailed(PieceColor color) const;

    QString getPieceChars() const;
    QString getFEN() const;
    bool setFEN(const QString& fen);

    SeatSide getHomeSide(PieceColor color) const;

    MovSeat getChangeMovSeat(MovSeat movSeat, ChangeType ct) const;
    bool changeLayout(ChangeType ct);

    QString getZhStr(const MovSeat& movSeat) const;
    MovSeat getMovSeat(const QString& zhStr) const;

    MovSeat getMovSeat_rowcols(const QString& rowcols) const;
    MovSeat getMovSeat(QPair<SeatCoord, SeatCoord> seatCoordlPair) const;

    QString toString(bool full = false) const;

private:
    PSeat getSeat_(int row, int col) const;
    PSeat getSeat_(SeatCoord seatCoord) const;

    // 取得与棋子特征有关的位置
    PSeat getKingSeat_(PieceColor color) const;
    QList<PSeat> getLiveSeatList_(PieceColor color) const;
    QList<PSeat> getLiveSeatList_(PieceColor color, PieceKind kind) const;
    QList<PSeat> getLiveSeatList_(PieceColor color, QChar name) const;
    QList<PSeat> getLiveSeatList_(PieceColor color, QChar name, int col) const;
    QList<PSeat> getSortPawnLiveSeatList_(PieceColor color, bool isBottom) const;

    static bool isKindName_(QChar name, QList<PieceKind> kinds);

    QList<SeatCoord> filterKillSeatCoord_(PSeat fromSeat, QList<SeatCoord>& seatCoordList) const;

    bool setBottomColor_();

    Pieces* pieces_;
    Seats* seats_;
    PieceColor bottomColor_;
};

#endif // BOARD_H
