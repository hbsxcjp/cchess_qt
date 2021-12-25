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
    BLACK,
    NOTCOLOR
};

enum class Kind {
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
    friend class Pieces;

public:
    virtual ~Piece() = default;

    Color color() const { return color_; }
    Kind kind() const { return kind_; }
    bool isSameColor(PPiece piece) const { return piece && color_ == piece->color(); }

    PSeat getSeat() const { return seat_; }
    void setSeat(PSeat seat) { seat_ = seat; }

    virtual QChar ch() const = 0;
    virtual QChar name() const = 0;

    // 棋子可置入位置坐标
    virtual QList<SeatCoord> putSeatCoord(Side homeSide) const;

    // 棋子从某位置可移至位置(排除不符合走棋规则的位置, 排除目标同色的位置)
    // 1.可移动位置；2.规则已排除位置；3.同色已排除位置
    QList<QList<SeatCoord>> canMoveSeatCoord(const Seats* seats, Side homeSide) const;

    QChar printName() const;
    QString toString() const;

protected:
    // 棋子从某位置可移至位置
    virtual QList<SeatCoord> moveSeatCoord(Side homeSide) const = 0;
    virtual QList<SeatCoord> filterRuleSeatCoord(const Seats* seats, QList<SeatCoord>& seatCoordList) const;

private:
    Piece(Color color, Kind kind);
    QList<SeatCoord> filterColorSeatCoord(const Seats* seats, QList<SeatCoord>& seatCoordList) const;

    const Color color_;
    const Kind kind_;

    PSeat seat_;
};

class King : public Piece {
    using Piece::Piece;

public:
    QChar ch() const { return color() == Color::RED ? 'K' : 'k'; };
    QChar name() const { return color() == Color::RED ? L'帅' : L'将'; }
    QList<SeatCoord> putSeatCoord(Side homeSide) const;

protected:
    QList<SeatCoord> moveSeatCoord(Side homeSide) const;
};

class Advisor : public Piece {
    using Piece::Piece;

public:
    QChar ch() const { return color() == Color::RED ? 'A' : 'a'; };
    QChar name() const { return color() == Color::RED ? L'仕' : L'士'; }
    QList<SeatCoord> putSeatCoord(Side homeSide) const;

protected:
    QList<SeatCoord> moveSeatCoord(Side homeSide) const;
};

class Bishop : public Piece {
    using Piece::Piece;

public:
    QChar ch() const { return color() == Color::RED ? 'B' : 'b'; };
    QChar name() const { return color() == Color::RED ? L'相' : L'象'; }
    QList<SeatCoord> putSeatCoord(Side homeSide) const;

protected:
    QList<SeatCoord> moveSeatCoord(Side homeSide) const;
    QList<SeatCoord> filterRuleSeatCoord(const Seats* seats, QList<SeatCoord>& seatCoordList) const;
};

class Knight : public Piece {
    using Piece::Piece;

public:
    QChar ch() const { return color() == Color::RED ? 'N' : 'n'; };
    QChar name() const { return L'马'; }

protected:
    QList<SeatCoord> moveSeatCoord(Side homeSide) const;
    QList<SeatCoord> filterRuleSeatCoord(const Seats* seats, QList<SeatCoord>& seatCoordList) const;
};

class Rook : public Piece {
    using Piece::Piece;

public:
    QChar ch() const { return color() == Color::RED ? 'R' : 'r'; };
    QChar name() const { return L'车'; }

protected:
    QList<SeatCoord> moveSeatCoord(Side homeSide) const;
    QList<SeatCoord> filterRuleSeatCoord(const Seats* seats, QList<SeatCoord>& seatCoordList) const;
};

class Cannon : public Piece {
    using Piece::Piece;

public:
    QChar ch() const { return color() == Color::RED ? 'C' : 'c'; };
    QChar name() const { return L'炮'; }

protected:
    QList<SeatCoord> moveSeatCoord(Side homeSide) const;
    QList<SeatCoord> filterRuleSeatCoord(const Seats* seats, QList<SeatCoord>& seatCoordList) const;
};

class Pawn : public Piece {
    using Piece::Piece;

public:
    QChar ch() const { return color() == Color::RED ? 'P' : 'p'; };
    QChar name() const { return color() == Color::RED ? L'兵' : L'卒'; }
    QList<SeatCoord> putSeatCoord(Side homeSide) const;

protected:
    QList<SeatCoord> moveSeatCoord(Side homeSide) const;
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
    QList<PPiece> getAllPiece(bool onlyKind = false) const;
    QList<PPiece> getColorPiece(Color color) const;
    QList<PPiece> getColorKindPiece(Color color, Kind kind) const { return pieces_[int(color)][int(kind)]; }

    // 取得与棋子特征有关的位置
    PSeat getKingSeat(Color color) const;
    QList<PSeat> getLiveSeatList(Color color) const;
    QList<PSeat> getLiveSeatList(Color color, Kind kind) const;
    QList<PSeat> getLiveSeatList(Color color, QChar name) const;
    QList<PSeat> getLiveSeatList(Color color, QChar name, int col) const;
    QList<PSeat> getSortPawnLiveSeatList(Color color, bool isBottom) const;

    QString getNameChars() const;
    QString getNameChars(Color color) const;
    QString getNameChars(QList<Kind> kinds) const;
    QString getZhChars() const;
    QString getChChars() const;

    Kind getKind(QChar ch) const;
    bool isKindName(QChar name, QList<Kind> kinds) const;
    bool isPiece(QChar name) const { return getNameChars().indexOf(name) >= 0; }

    QString toString() const;

    static int getRowFromICCSChar(QChar ch) { return ICCS_RowChars.indexOf(ch); }
    static int getColFromICCSChar(QChar ch) { return ICCS_ColChars.indexOf(ch); }

    static QChar getOtherChar(QChar ch)
    {
        return ch.isLetter() ? (ch.isUpper() ? ch.toLower() : ch.toUpper()) : ch;
    }
    static QChar getColICCSChar(int col) { return ICCS_ColChars.at(col); }

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
    static const QStringList numChars;
    static const QString ICCS_ColChars;
    static const QString ICCS_RowChars;
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
