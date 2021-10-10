#ifndef BOARD_H
#define BOARD_H

#include "piece.h"

using SeatPiece = QPair<Seat, PPiece>;

class Board {
public:
    void clean();

    QList<SeatPiece> getSeatPieceList() const;
    QList<SeatPiece> getColorLiveSeatList(Piece::Color color) const;

    void setPiece(const SeatPiece& seatPiece) { setPiece_(seatPiece.first, seatPiece.second); }
    PPiece movePiece(Seat fromSeat, Seat toSeat);

    // 棋子可移动位置
    QList<Seat> canMove(const Seat& seat);
    QList<Seat> allCanMove(Piece::Color color);
    bool isCanMove(const Seat& fromSeat, const Seat& toSeat);

    void changeSide(SeatManager::ChangeType ct);

    QString movSeatToStr(const MovSeat& movSeat) const;
    MovSeat strToMovSeat(const QString& zhStr, bool ignoreError = false) const;

    QString getFEN() const;
    bool setFEN(const QString& fen);

    // 某方棋子是否正在被对方将军
    bool isKilling(Piece::Color color) const;
    bool isKilled(Piece::Color color) const { return isFace_() || isKilling(color); }

    QString pieCharsToFEN_(const QString& pieChars) const;
    QString FENTopieChars_(const QString& fen) const;

    const QString toString(bool full = false) const;

private:
    // 棋子可移动位置，已考虑走棋规则、棋局盘面，不考虑将帅对面、被将军等情况
    QList<Seat> canMove_(const Seat& seat) const;
    Seat getKingSeat_(Piece::Color color) const;
    QList<Seat> getLiveSeatList_(Piece::Color color, QChar name) const;
    QList<Seat> getLiveSeatList_(Piece::Color color, QChar name, int col) const;
    QList<Seat> getLiveSeatList_(Piece::Color color, QChar name, int col, bool getStronge) const;
    QList<Seat> getSortPawnLiveSeats_(bool isBottom, Piece::Color color, QChar name) const;

    bool isFace_() const;

    SeatManager::Seatside getSeatside_(Piece::Color color) const
    {
        return color == bottomColor_ ? SeatManager::Seatside::HERE : SeatManager::Seatside::THERE;
    }

    bool inSeat_(PPiece piece) const;
    PPiece getUnUsedPiece_(QChar ch) const;

    PPiece getPiece_(const Seat& seat) const { return seats_[seat.first][seat.second]; }
    void setPiece_(const Seat& seat, PPiece piece = nullptr) { seats_[seat.first][seat.second] = piece; }
    void setBottomColor_();
    bool setFromSeatPieceList_(const QList<SeatPiece>& seatPieceList);

    QString getPieChars_(const QList<SeatPiece>& seatPieceList) const;
    QList<SeatPiece> getSeatPieceList_pieChars_(const QString& pieChars) const;

    Pieces pieces_ {};
    PPiece seats_[SEATROW][SEATCOL] {};
    Piece::Color bottomColor_ {};
};

#endif // BOARD_H
