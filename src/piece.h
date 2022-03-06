#ifndef PIECE_H
#define PIECE_H
// 棋子类

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
enum class SeatSide;

class Piece;
using PPiece = Piece*;

enum class PieceColor {
    RED,
    BLACK
};

enum class PieceKind {
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

    PieceColor color() const { return color_; }
    PieceKind kind() const { return kind_; }
    bool isSameColor(PPiece piece) const { return piece && color_ == piece->color(); }

    PSeat getSeat() const { return seat_; }
    void setSeat(PSeat seat) { seat_ = seat; }

    QChar ch() const;
    QChar name() const;

    // 棋子可置入位置坐标
    virtual QList<SeatCoord> putTo(SeatSide homeSide) const;

    // 棋子从某位置可移至位置(排除不符合走棋规则的位置, 排除目标同色的位置)
    // 1.可移动位置；2.规则已排除位置；3.同色已排除位置
    QList<QList<SeatCoord>> moveTo(const Seats* seats, SeatSide homeSide) const;

    QChar printName() const;
    QString toString() const;

protected:
    // 棋子从某位置可移至位置
    virtual QList<SeatCoord> moveTo_(SeatSide homeSide) const = 0;
    virtual QList<SeatCoord> ruleFiter_(const Seats* seats, QList<SeatCoord>& seatCoordList) const;

private:
    Piece(PieceColor color, PieceKind kind);

    QList<SeatCoord> colorFilter_(const Seats* seats, QList<SeatCoord>& seatCoordList) const;

    const PieceColor color_;
    const PieceKind kind_;

    PSeat seat_;
};

class King : public Piece {
    using Piece::Piece;

public:
    QList<SeatCoord> putTo(SeatSide homeSide) const;

protected:
    QList<SeatCoord> moveTo_(SeatSide homeSide) const;
};

class Advisor : public Piece {
    using Piece::Piece;

public:
    QList<SeatCoord> putTo(SeatSide homeSide) const;

protected:
    QList<SeatCoord> moveTo_(SeatSide homeSide) const;
};

class Bishop : public Piece {
    using Piece::Piece;

public:
    QList<SeatCoord> putTo(SeatSide homeSide) const;

protected:
    QList<SeatCoord> moveTo_(SeatSide homeSide) const;
    QList<SeatCoord> ruleFiter_(const Seats* seats, QList<SeatCoord>& seatCoordList) const;
};

class Knight : public Piece {
    using Piece::Piece;

protected:
    QList<SeatCoord> moveTo_(SeatSide homeSide) const;
    QList<SeatCoord> ruleFiter_(const Seats* seats, QList<SeatCoord>& seatCoordList) const;
};

class Rook : public Piece {
    using Piece::Piece;

protected:
    QList<SeatCoord> moveTo_(SeatSide homeSide) const;
    QList<SeatCoord> ruleFiter_(const Seats* seats, QList<SeatCoord>& seatCoordList) const;
};

class Cannon : public Piece {
    using Piece::Piece;

protected:
    QList<SeatCoord> moveTo_(SeatSide homeSide) const;
    QList<SeatCoord> ruleFiter_(const Seats* seats, QList<SeatCoord>& seatCoordList) const;
};

class Pawn : public Piece {
    using Piece::Piece;

public:
    QList<SeatCoord> putTo(SeatSide homeSide) const;

protected:
    QList<SeatCoord> moveTo_(SeatSide homeSide) const;
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
    QList<PPiece> getColorPiece(PieceColor color) const;
    QList<PPiece> getColorKindPiece(PieceColor color, PieceKind kind) const;

    // 取得与棋子特征有关的位置
    PSeat getKingSeat(PieceColor color) const;
    QList<PSeat> getLiveSeatList(PieceColor color) const;
    QList<PSeat> getLiveSeatList(PieceColor color, PieceKind kind) const;
    QList<PSeat> getLiveSeatList(PieceColor color, QChar name) const;
    QList<PSeat> getLiveSeatList(PieceColor color, QChar name, int col) const;
    QList<PSeat> getSortPawnLiveSeatList(PieceColor color, bool isBottom) const;

    QString toString() const;

    static QString getZhChars();
    static QString getIccsChars();

    static PieceKind getKind(QChar ch);
    static bool isKindName(QChar name, QList<PieceKind> kinds);
    static bool isPiece(QChar name);

    static int getRowFrom(QChar ch);
    static int getColFrom(QChar ch);

    static QChar getOtherChar(QChar ch);
    static QChar getIccsChar(int col);

    static PieceColor getColor(QChar ch);
    static PieceColor getOtherColor(PieceColor color);
    static PieceColor getColorFromZh(QChar numZh);

    static int getIndex(int seatsLen, bool isBottom, QChar preChar);
    static QChar getIndexChar(int seatsLen, bool isBottom, int index);

    static int getMovNum(bool isBottom, QChar movChar);
    static QChar getMovChar(bool isSameRow, bool isBottom, bool isLowToUp);

    static int getNum(PieceColor color, QChar numChar);
    static QChar getNumChar(PieceColor color, int num);

    static int getCol(bool isBottom, int num);
    static QChar getColChar(PieceColor color, bool isBottom, int col);

    static const QList<PieceColor> allColorList;
    static const QList<PieceKind> allKindList;
    static const QList<PieceKind> strongeKindList;
    static const QList<PieceKind> lineKindList;

    static const QString preChars;
    static const QString movChars;
    static const QStringList chars;
    static const QStringList nameChars;
    static const QStringList numChars;
    static const QString iccsColChars;
    static const QString iccsRowChars;
    static const QString FENStr;
    static const QString allChars;
    static const QChar nullChar;
    static const QChar FENSplitChar;

private:
    static PPiece getPiece_(PieceColor color, PieceKind kind);

    static QList<PSeat> getLiveSeatList_(const QList<PPiece>& pieceList);
    static const QString getPreChars_(int length);

    QList<PPiece> pieces_[COLORNUM][KINDNUM] {};
};

#endif // PIECE_H
