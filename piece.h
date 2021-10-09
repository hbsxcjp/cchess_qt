#ifndef PIECE_H
#define PIECE_H

#include <QList>
#include <QMetaType>
#include <QPair>
#include <QString>
#include <QVector>
#include <QtCore>

//#define CREATE_TESTPIECE_TEXT
#define COLORNUM 2
#define KINDNUM 7
#define SEATROW 10
#define SEATCOL 9
#define SEATNUM (SEATROW * SEATCOL)

enum Seatside {
    HERE,
    THERE
};

using Seat = QPair<int, int>;
class Piece;
using PPiece = Piece*;

// 棋子类
class Piece {
public:
    enum Color {
        RED,
        BLACK,
        NOTCOLOR
    };

    enum Kind {
        KING,
        ADVISOR,
        BISHOP,
        KNIGHT,
        ROOK,
        CANNON,
        PAWN,
        NOTKIND
    };

    //Piece() {};
    Piece(Color color = Color::RED, Kind kind = Kind::KING);

    Color color() const { return color_; }
    Kind kind() const { return kind_; }
    static Color getColor(QChar ch);
    static Kind getKind(QChar ch);

    QChar ch() const;
    QChar name() const;
    QChar printName() const;

    // 棋子可置入位置
    QList<Seat> put(Seatside homeSide) const;

    // 棋子从某位置可移至位置
    QList<Seat> move(Seat seat, Seatside homeSide) const;

    // 测试函数
    const QString toString() const;
    const QString putString(Seatside homeSide) const;
    const QString moveString(Seat seat, Seatside homeSide) const;

private:
    Color color_;
    Kind kind_;
};

// 一副棋子类
class Pieces {
public:
    Pieces();
    ~Pieces();

    QList<PPiece> getColorKindPiece(Piece::Color color, Piece::Kind kind) const { return pieces_[color][kind]; }
    QList<PPiece> getColorPiece(Piece::Color color) const;
    QList<PPiece> getAllPiece(bool onlyKind = false) const;

    // 棋子可至全部位置
    QList<Seat> getAllSeat() const;

    // 测试函数

private:
    QList<PPiece> pieces_[COLORNUM][KINDNUM] {};
};

int rotateRow(int row);
int rotateCol(int col);
Seat& rotateSeat(Seat& seat);
QString printSeat(const Seat& seat);
QString printSeatList(const QList<Seat>& seatList);

extern const QChar NullChar;
extern const QChar FENSplitChar;
extern const QVector<QString> Chars;
extern const QString FEN;

Q_DECLARE_METATYPE(PPiece)
Q_DECLARE_METATYPE(Piece)
Q_DECLARE_METATYPE(Seatside)

#endif // PIECE_H
