#ifndef PIECE_H
#define PIECE_H

#include <QList>
#include <QPair>
#include <QString>

#define COLORNUM 2
#define KINDNUM 7
#define PIECENUM 32

class Piece;
using PPiece = Piece*;

class Pieces;

// 棋子类
class Piece {
    friend Pieces;

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

    Color color() const { return color_; }
    Kind kind() const { return kind_; }
    static Color getColor(QChar ch);
    static Kind getKind(QChar ch);

    QChar ch() const;
    QChar name() const;
    QChar printName() const;

    // 测试函数
    const QString toString() const;

private:
    Piece(Color color = Color::RED, Kind kind = Kind::KING);

    const Color color_;
    const Kind kind_;
};

// 一副棋子类
class Pieces {
public:
    Pieces();
    ~Pieces();

    QList<PPiece> getAllPiece(bool onlyKind = false) const;
    QList<PPiece> getColorPiece(Piece::Color color) const;
    QList<PPiece> getColorKindPiece(Piece::Color color, Piece::Kind kind) const { return pieces_[color][kind]; }

    static Piece::Color getOtherColor(Piece::Color color);
    static const QString getZhChars();
    static const QString getICCSChars();
    static const QString& getFENStr();
    static const QString& getChChars();
    static const QString& getNameChars();
    static const QChar getFENSplitChar();
    static int getRowFromICCSChar(QChar ch);
    static int getColFromICCSChar(QChar ch);
    static QChar getOtherChar(QChar ch);
    static QChar getColICCSChar(int col);

    static QChar getName(QChar ch);
    static Piece::Color getColor(QChar ch);
    static Piece::Color getColorFromZh(QChar numZh);
    static int getIndex(int seatsLen, bool isBottom, QChar preChar);
    static QChar getIndexChar(int seatsLen, bool isBottom, int index);
    static QChar nullChar();
    static QChar redKingChar();
    static int getMovNum(bool isBottom, QChar movChar);
    static QChar getMovChar(bool isSameRow, bool isBottom, bool isLowToUp);
    static int getNum(Piece::Color color, QChar numChar);
    static QChar getNumChar(Piece::Color color, int num);
    static int getCol(bool isBottom, int num);
    static QChar getColChar(Piece::Color color, bool isBottom, int col);
    static bool isKing(QChar name);
    static bool isAdvBish(QChar name);
    static bool isStronge(QChar name);
    static bool isLineMove(QChar name);
    static bool isPawn(QChar name);
    static bool isPiece(QChar name);

private:
    static const QString getPreChars_(int length);

    static const QString chChars_;
    static const QString preChars_;
    static const QString nameChars_;
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
