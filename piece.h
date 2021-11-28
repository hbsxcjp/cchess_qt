#ifndef PIECE_H
#define PIECE_H

#include <QList>
#include <QMetaType>
#include <QPair>
#include <QString>
#include <QVector>
#include <QtCore>

#define COLORNUM 2
#define KINDNUM 7
#define PIECENUM 32
#define SEATROW 10
#define SEATCOL 9
#define SEATNUM (SEATROW * SEATCOL)

class Piece;
using PPiece = Piece*;

using Seat = QPair<int, int>;
using PSeat = Seat*;
using SeatPiece = QPair<Seat, PPiece>;

using MovSeat = QPair<Seat, Seat>;
using MovSeatPiece = QPair<MovSeat, PPiece>;

class SeatManager {
public:
    enum Seatside {
        HERE,
        THERE
    };

    enum ChangeType {
        EXCHANGE,
        ROTATE,
        SYMMETRY,
        NOCHANGE
    };

    static int rotateRow(int row);
    static int rotateCol(int col);
    static bool isValied(const Seat& seat);
    static bool isValied(const MovSeat& movseat);

    static int index(const Seat& seat);
    static int rowcol(const Seat& seat);
    static int rowcols(const MovSeat& movseat);

    static Seat seat(int rowcol);
    static MovSeat movseat(int rowcols);

    // 棋子可至全部位置
    static QList<Seat> allSeats();

    static void changeSeat(Seat& seat, ChangeType ct);
    static QString printSeat(const Seat& seat);
    static QString printSeatList(const QList<Seat>& seatList);

private:
};

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

    // Piece() {};
    Piece(Color color = Color::RED, Kind kind = Kind::KING);

    Color color() const { return color_; }
    Kind kind() const { return kind_; }
    static Color getColor(QChar ch);
    static Kind getKind(QChar ch);

    QChar ch() const;
    QChar name() const;
    QChar printName() const;

    // 棋子可置入位置
    QList<Seat> put(SeatManager::Seatside homeSide) const;

    // 棋子从某位置可移至位置
    QList<Seat> move(Seat seat, SeatManager::Seatside homeSide) const;

    // 测试函数
    const QString toString() const;

private:
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

private:
    QList<PPiece> pieces_[COLORNUM][KINDNUM] {};
};

class PieceManager {
public:
    static Piece::Color getOtherColor(Piece::Color color);
    static const QString getZhChars();
    static const QString getICCSChars();
    static const QString& getFENStr();
    static const QString& getChChars();
    static const QString& getNameChars();
    static const QChar getFENSplitChar();
    static bool redIsBottom(const QString& fen);
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
    static const QString getPreChars__(int length);

    static const QString preChars_;
    static const QString nameChars_;
    static const QString movChars_;
    static const QMap<Piece::Color, QString> numChars_;
    static const QString ICCS_ColChars_;
    static const QString ICCS_RowChars_;
    static const QString FENStr_;
    static const QChar nullChar_;
    static const QString chChars_;
    static const QChar FENSplitChar_;
};

Q_DECLARE_METATYPE(SeatManager::Seatside)

#endif // PIECE_H
