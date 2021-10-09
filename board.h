#ifndef BOARD_H
#define BOARD_H

#include "piece.h"

using SeatPiece = QPair<Seat, PPiece>;

class Board {
public:
    void setPiece(const SeatPiece& seatPiece) { setPiece_(seatPiece.first, seatPiece.second); }
    PPiece movePiece(Seat fromSeat, Seat toSeat);

    // 棋子可移动全部位置
    QList<Seat> allCanMoveSeat(Piece::Color color) const;

    QString getFEN() const;
    bool setFEN(const QString& fen);

    // 测试函数
    const QString toString(bool full = false) const;
    //const QString moveString(Seat seat, Seatside homeSide) const;

private:
    Seatside getSeatside_(Piece::Color color) const { return color == bottomColor_ ? Seatside::HERE : Seatside::THERE; }

    bool inSeat_(PPiece piece) const;
    PPiece getUnUsedPiece_(QChar ch) const;

    PPiece getPiece_(const Seat& seat) const { return seats_[seat.first][seat.second]; }
    void setPiece_(const Seat& seat, PPiece piece = nullptr) { seats_[seat.first][seat.second] = piece; }

    QList<SeatPiece> getSeatPieceList_(Piece::Color color = Piece::Color::NOTCOLOR) const;
    bool setFromSeatPieceList_(const QList<SeatPiece>& seatPieceList);

    QString getPieChars_(const QList<SeatPiece>& seatPieceList) const;
    QList<SeatPiece> getSeatPieceList_pieChars_(const QString& pieChars) const;

    QString pieCharsToFEN_(const QString& pieChars) const;
    QString FENTopieChars_(const QString& fen) const;

    Pieces pieces_ {};
    PPiece seats_[SEATROW][SEATCOL] {};
    Piece::Color bottomColor_ {};
};

#endif // BOARD_H
