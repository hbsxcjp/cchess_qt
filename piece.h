#ifndef PIECE_H
#define PIECE_H

#include "seat.h"
#include <QList>
#include <QPair>
#include <QString>

#define COLORNUM 2
#define KINDNUM 7
#define PIECENUM 32

#define SEATROW 10
#define SEATCOL 9
#define SEATNUM (SEATROW * SEATCOL)

class Piece;
using PPiece = Piece*;

class Pieces;

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

// 棋子类
class Piece {
public:
    Piece(Color color, Kind kind);
    virtual ~Piece() = default;

    Color color() const { return color_; }
    Kind kind() const { return kind_; }

    virtual QChar ch() const;
    virtual QChar name() const;

    // 棋子可置入位置
    QList<PSeat> put(const Seats& seats, Side homeSide) const;

    // 棋子从某位置可移至位置
    QList<PSeat> move(const Seats& seats, PSeat fseat, Side homeSide) const;

    QChar printName() const;
    QString toString() const;

private:
    const Color color_;
    const Kind kind_;
};

class King : public Piece {
public:
    using Piece::Piece;

    QChar ch() const { return color() == Color::RED ? 'K' : 'k'; };
    QChar name() const { return color() == Color::RED ? L'帅' : L'将'; }
};

class Advisor : public Piece {
public:
    using Piece::Piece;

    QChar ch() const { return color() == Color::RED ? 'A' : 'a'; };
    QChar name() const { return color() == Color::RED ? L'仕' : L'士'; }
};

class Bishop : public Piece {
public:
    using Piece::Piece;

    QChar ch() const { return color() == Color::RED ? 'B' : 'b'; };
    QChar name() const { return color() == Color::RED ? L'相' : L'象'; }
};

class Knight : public Piece {
public:
    using Piece::Piece;

    QChar ch() const { return color() == Color::RED ? 'N' : 'n'; };
    QChar name() const { return L'马'; }
};

class Rook : public Piece {
public:
    using Piece::Piece;

    QChar ch() const { return color() == Color::RED ? 'R' : 'r'; };
    QChar name() const { return L'车'; }
};

class Cannon : public Piece {
public:
    using Piece::Piece;

    QChar ch() const { return color() == Color::RED ? 'C' : 'c'; };
    QChar name() const { return L'炮'; }
};

class Pawn : public Piece {
public:
    using Piece::Piece;

    QChar ch() const { return color() == Color::RED ? 'P' : 'p'; };
    QChar name() const { return color() == Color::RED ? L'兵' : L'卒'; }
};

// 一副棋子类
class Pieces {
public:
    Pieces();
    ~Pieces();

    QList<PPiece> getAllPiece(bool onlyKind = false) const;
    QList<PPiece> getColorPiece(Color color) const;
    QList<PPiece> getColorKindPiece(Color color, Kind kind) const { return pieces_[color][kind]; }

    QString getNameChars() const;
    QString getZhChars();
    QString getChChars();

    bool isKindName(QChar name, QList<Kind> kinds);
    bool isPiece(QChar name)
    {
        return getNameChars().indexOf(name) >= 0;
    }

    static const QString getICCSChars() { return ICCS_ColChars_ + ICCS_RowChars_; }
    static const QString& getFENStr() { return FENStr_; }

    static int getRowFromICCSChar(QChar ch) { return ICCS_RowChars_.indexOf(ch); }
    static int getColFromICCSChar(QChar ch) { return ICCS_ColChars_.indexOf(ch); }

    static const QChar getFENSplitChar() { return FENSplitChar_; }
    static QChar nullChar() { return nullChar_; }
    static QChar getOtherChar(QChar ch)
    {
        return ch.isLetter() ? (ch.isUpper() ? ch.toLower() : ch.toUpper()) : ch;
    }
    static QChar getColICCSChar(int col) { return ICCS_ColChars_.at(col); }

    static Color getOtherColor(Color color) { return color == Color::RED ? Color::BLACK : Color::RED; }
    static Color getColor(QChar ch) { return ch.isLower() ? Color::BLACK : Color::RED; }
    static Color getColorFromZh(QChar numZh)
    {
        return numChars_[Color::RED].indexOf(numZh) >= 0 ? Color::RED : Color::BLACK;
    }

    static int getIndex(int seatsLen, bool isBottom, QChar preChar);
    static QChar getIndexChar(int seatsLen, bool isBottom, int index)
    {
        return getPreChars_(seatsLen).at(isBottom ? seatsLen - 1 - index : index);
    }

    static int getMovNum(bool isBottom, QChar movChar)
    {
        return (movChars_.indexOf(movChar) - 1) * (isBottom ? 1 : -1);
    }
    static QChar getMovChar(bool isSameRow, bool isBottom, bool isLowToUp)
    {
        return movChars_.at(isSameRow ? 1 : (isBottom == isLowToUp ? 2 : 0));
    }

    static int getNum(Color color, QChar numChar) { return numChars_[color].indexOf(numChar) + 1; }
    static QChar getNumChar(Color color, int num) { return numChars_[color].at(num - 1); }

    static int getCol(bool isBottom, int num) { return isBottom ? SEATCOL - num : num - 1; }
    static QChar getColChar(Color color, bool isBottom, int col) { return numChars_[color].at(isBottom ? SEATCOL - col - 1 : col); }

private:
    static const QString getPreChars_(int length);

    static const QString preChars_;
    static const QString movChars_;
    static const QStringList numChars_;
    static const QString ICCS_ColChars_;
    static const QString ICCS_RowChars_;
    static const QString FENStr_;
    static const QChar nullChar_;
    static const QChar FENSplitChar_;

    QList<PPiece> pieces_[COLORNUM][KINDNUM] {};
};

#endif // PIECE_H
