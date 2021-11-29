#ifndef BOARD_H
#define BOARD_H

#include "piece.h"

class Board {
public:
    void clean();
    void reinit();

    QList<SeatPiece> getSeatPieceList() const;
    QList<SeatPiece> getColorSeatPieceList(Piece::Color color) const;

    PPiece getPiece(const Seat& seat) const { return seats_[seat.first][seat.second]; }
    void setPiece(const SeatPiece& seatPiece) { setPiece_(seatPiece.first, seatPiece.second); }
    PPiece movePiece(const MovSeat& movseat);

    // 棋子可移动位置
    QList<Seat> canMove(const Seat& seat);
    QList<Seat> allCanMove(Piece::Color color);
    bool isCanMove(const MovSeat& movSeat);

    // 某方棋子是否正在被对方将军
    bool isKilling(Piece::Color color) const;
    bool isFailed(Piece::Color color) { return allCanMove(color).count() == 0; }

    void changeSide(SeatManager::ChangeType ct);

    QString movSeatToStr(const MovSeat& movSeat) const;
    MovSeat strToMovSeat(const QString& zhStr, bool ignoreError = false) const;

    QString getFEN() const;
    bool setFEN(const QString& fen);

    static QString pieCharsToFEN(const QString& pieChars);
    static QString FENTopieChars(const QString& fen);

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

    SeatManager::SeatSide getSeatside_(Piece::Color color) const
    {
        return color == bottomColor_ ? SeatManager::SeatSide::HERE : SeatManager::SeatSide::THERE;
    }

    bool inSeat_(PPiece piece) const;
    PPiece getUnUsedPiece_(QChar ch) const;

    void setPiece_(const Seat& seat, PPiece piece = nullptr) { seats_[seat.first][seat.second] = piece; }
    void setBottomColor_();
    bool setFromSeatPieceList_(const QList<SeatPiece>& seatPieceList);

    QString getPieChars_(const QList<SeatPiece>& seatPieceList) const;
    QList<SeatPiece> getSeatPieceList_pieChars_(const QString& pieChars) const;

    Pieces pieces_ {};
    PPiece seats_[SEATROW][SEATCOL] {};
    Piece::Color bottomColor_ {};
};

extern const QVector<QString> fens;
extern const QString skipExplain;
#endif // BOARD_H
