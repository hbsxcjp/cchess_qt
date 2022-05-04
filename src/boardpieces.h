#ifndef BOARDPIECES_H
#define BOARDPIECES_H

#include <QList>

class Seat;
class Piece;
enum class PieceColor;
enum class PieceKind;

// 一副棋子类
class BoardPieces {
public:
    BoardPieces();
    ~BoardPieces();

    // 取得未在棋盘上的棋子
    Piece* getNonLivePiece(PieceColor color, PieceKind kind) const;
    Piece* getOtherPiece(Piece* piece) const;

    // 取得与棋子特征有关的棋子
    QList<Piece*> getAllPiece() const;

    // 取得与棋子特征有关的位置
    Seat* getKingSeat(PieceColor color) const;
    QList<Seat*> getLiveSeats() const;
    QList<Seat*> getLiveSeats(PieceColor color) const;
    QList<Seat*> getLiveSeats(PieceColor color, PieceKind kind) const;
    QList<Seat*> getLiveSeats(PieceColor color, QChar name) const;
    QList<Seat*> getLiveSeats(PieceColor color, QChar name, int col) const;
    QList<Seat*> getLiveSeats_SortPawn(PieceColor color, bool isBottom) const;

    QString toString() const;

private:
    QList<Piece*> getPieces(PieceColor color) const;
    QList<Piece*> getPieces(PieceColor color, PieceKind kind) const;

    static QList<Seat*> getLiveSeats(const QList<Piece*>& pieces);

    QList<QList<QList<Piece*>>> pieces;
};

#endif // BOARDPIECES_H
