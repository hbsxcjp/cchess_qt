#ifndef PIECE_H
#define PIECE_H

#include <QList>
#include <QMap>
#include <QPair>
#include <QString>

#define COLORNUM 2
#define KINDNUM 7
#define PIECENUM 32

class Seat;
class Seats;
using PSeat = Seat*;
using SeatCoord = QPair<int, int>;
enum class Side;

class Piece;
using PPiece = Piece*;

enum class Color {
    RED,
    BLACK
};

enum class Kind {
    KING,
    ADVISOR,
    BISHOP,
    KNIGHT,
    ROOK,
    CANNON,
    PAWN
};

// 棋子类
class Piece {
    friend class Pieces;

public:
    virtual ~Piece() = default;

    Color color() const { return color_; }
    Kind kind() const { return kind_; }
    bool isSameColor(PPiece piece) const { return piece && color_ == piece->color(); }

    PSeat getSeat() const { return seat_; }
    void setSeat(PSeat seat) { seat_ = seat; }

    QChar ch() const;
    QChar name() const;

    // 棋子可置入位置坐标
    virtual QList<SeatCoord> putTo(Side homeSide) const;

    // 棋子从某位置可移至位置(排除不符合走棋规则的位置, 排除目标同色的位置)
    // 1.可移动位置；2.规则已排除位置；3.同色已排除位置
    QList<QList<SeatCoord>> moveTo(const Seats* seats, Side homeSide) const;

    QChar printName() const;
    QString toString() const;

protected:
    // 棋子从某位置可移至位置
    virtual QList<SeatCoord> moveTo_(Side homeSide) const = 0;
    virtual QList<SeatCoord> ruleFiter_(const Seats* seats, QList<SeatCoord>& seatCoordList) const;

private:
    Piece(Color color, Kind kind);

    QList<SeatCoord> colorFilter_(const Seats* seats, QList<SeatCoord>& seatCoordList) const;

    const Color color_;
    const Kind kind_;

    PSeat seat_;
};

class King : public Piece {
    using Piece::Piece;

public:
    QList<SeatCoord> putTo(Side homeSide) const;

protected:
    QList<SeatCoord> moveTo_(Side homeSide) const;
};

class Advisor : public Piece {
    using Piece::Piece;

public:
    QList<SeatCoord> putTo(Side homeSide) const;

protected:
    QList<SeatCoord> moveTo_(Side homeSide) const;
};

class Bishop : public Piece {
    using Piece::Piece;

public:
    QList<SeatCoord> putTo(Side homeSide) const;

protected:
    QList<SeatCoord> moveTo_(Side homeSide) const;
    QList<SeatCoord> ruleFiter_(const Seats* seats, QList<SeatCoord>& seatCoordList) const;
};

class Knight : public Piece {
    using Piece::Piece;

protected:
    QList<SeatCoord> moveTo_(Side homeSide) const;
    QList<SeatCoord> ruleFiter_(const Seats* seats, QList<SeatCoord>& seatCoordList) const;
};

class Rook : public Piece {
    using Piece::Piece;

protected:
    QList<SeatCoord> moveTo_(Side homeSide) const;
    QList<SeatCoord> ruleFiter_(const Seats* seats, QList<SeatCoord>& seatCoordList) const;
};

class Cannon : public Piece {
    using Piece::Piece;

protected:
    QList<SeatCoord> moveTo_(Side homeSide) const;
    QList<SeatCoord> ruleFiter_(const Seats* seats, QList<SeatCoord>& seatCoordList) const;
};

class Pawn : public Piece {
    using Piece::Piece;

public:
    QList<SeatCoord> putTo(Side homeSide) const;

protected:
    QList<SeatCoord> moveTo_(Side homeSide) const;
};

// 一副棋子类
class Pieces {
public:
    Pieces();
    ~Pieces();

    // 取得未在棋盘上的棋子
    PPiece getNotLivePiece(QChar ch) const;
    PPiece getOtherPiece(const PPiece& piece) const;

    // 取得与棋子特征有关的棋子
    QList<PPiece> getColorPiece(Color color) const;
    QList<PPiece> getColorKindPiece(Color color, Kind kind) const { return pieces_[int(color)][int(kind)]; }

    // 取得与棋子特征有关的位置
    PSeat getKingSeat(Color color) const;
    QList<PSeat> getLiveSeatList(Color color) const;
    QList<PSeat> getLiveSeatList(Color color, Kind kind) const;
    QList<PSeat> getLiveSeatList(Color color, QChar name) const;
    QList<PSeat> getLiveSeatList(Color color, QChar name, int col) const;
    QList<PSeat> getSortPawnLiveSeatList(Color color, bool isBottom) const;

    QString toString() const;

    static QString getZhChars();

    static Kind getKind(QChar ch);
    static bool isKindName(QChar name, QList<Kind> kinds);
    static bool isPiece(QChar name) { return nameChars.join("").indexOf(name) >= 0; }

    static int getRowFrom(QChar ch) { return iccsRowChars.indexOf(ch); }
    static int getColFrom(QChar ch) { return iccsColChars.indexOf(ch); }

    static QChar getOtherChar(QChar ch)
    {
        return ch.isLetter() ? (ch.isUpper() ? ch.toLower() : ch.toUpper()) : ch;
    }
    static QChar getIccsChar(int col) { return iccsColChars.at(col); }

    static Color getColor(QChar ch) { return ch.isLower() ? Color::BLACK : Color::RED; }
    static Color getOtherColor(Color color) { return color == Color::RED ? Color::BLACK : Color::RED; }
    static Color getColorFromZh(QChar numZh)
    {
        return numChars[int(Color::RED)].indexOf(numZh) >= 0 ? Color::RED : Color::BLACK;
    }

    static int getIndex(int seatsLen, bool isBottom, QChar preChar);
    static QChar getIndexChar(int seatsLen, bool isBottom, int index)
    {
        return getPreChars_(seatsLen).at(isBottom ? seatsLen - 1 - index : index);
    }

    static int getMovNum(bool isBottom, QChar movChar)
    {
        return (movChars.indexOf(movChar) - 1) * (isBottom ? 1 : -1);
    }
    static QChar getMovChar(bool isSameRow, bool isBottom, bool isLowToUp)
    {
        return movChars.at(isSameRow ? 1 : (isBottom == isLowToUp ? 2 : 0));
    }

    static int getNum(Color color, QChar numChar) { return numChars[int(color)].indexOf(numChar) + 1; }
    static QChar getNumChar(Color color, int num) { return numChars[int(color)].at(num - 1); }

    static int getCol(bool isBottom, int num);
    static QChar getColChar(Color color, bool isBottom, int col);

    static const QList<Color> allColorList;
    static const QList<Kind> allKindList;
    static const QList<Kind> strongeKindList;
    static const QList<Kind> lineKindList;

    static const QString preChars;
    static const QString movChars;
    static const QStringList chars;
    static const QStringList nameChars;
    static const QStringList numChars;
    static const QString iccsColChars;
    static const QString iccsRowChars;
    static const QString FENStr;
    static const QChar nullChar;
    static const QChar FENSplitChar;

private:
    static PPiece getPiece_(Color color, Kind kind);

    static QList<PSeat> getLiveSeatList_(const QList<PPiece>& pieceList);
    static const QString getPreChars_(int length);

    QList<PPiece> pieces_[COLORNUM][KINDNUM] {};
};

#endif // PIECE_H
