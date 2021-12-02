#include "piece.h"
#include "seat.h"
#include <functional>

static const QList<Color> allColorList { Color::RED, Color::BLACK };

static const QList<Kind> allKindList { Kind::KING, Kind::ADVISOR, Kind::BISHOP,
    Kind::KNIGHT, Kind::ROOK, Kind::CANNON, Kind::PAWN };

Piece::Piece(Color color, Kind kind)
    : color_(color)
    , kind_(kind)
{
}

QList<QPair<int, int>> Piece::allPutRowCol()
{
    QList<QPair<int, int>> rcPairs;
    for (int row = 0; row < SEATROW; ++row)
        for (int col = 0; col < SEATCOL; ++col)
            rcPairs.append({ row, col });

    return rcPairs;
}

QList<QPair<int, int>> Piece::rookCannonmoveRowCol(int row, int col)
{
    QList<QPair<int, int>> rcPairs;
    // 先行后列，先小后大。顺序固定，为Board::canMove()分析走棋规则打下基础
    for (int r = row - 1; r >= 0; --r)
        rcPairs.append({ r, col });
    for (int r = row + 1; r < SEATROW; ++r)
        rcPairs.append({ r, col });
    for (int c = col - 1; c >= 0; --c)
        rcPairs.append({ row, c });
    for (int c = col + 1; c < SEATCOL; ++c)
        rcPairs.append({ row, c });

    return rcPairs;
}

QList<QPair<int, int>> Piece::getValidRowCol(QList<QPair<int, int>> rcPairs)
{
    for (auto iter = rcPairs.begin(); iter != rcPairs.end(); ++iter)
        if (!Seats::isValidRow(iter->first) || !Seats::isValidCol(iter->second))
            rcPairs.erase(iter);

    return rcPairs;
}

QChar Piece::printName() const
{
    if (color_ == Color::BLACK) {
        if (kind_ == Kind::KNIGHT)
            return L'馬';
        else if (kind_ == Kind::ROOK)
            return L'車';
        else if (kind_ == Kind::CANNON)
            return L'砲';
    }

    return name();
}

QString Piece::toString() const
{
    return QString() + QChar(color_ == Color::RED ? L'红' : L'黑') + printName() + ch();
}

QList<QPair<int, int>> King::putRowCol(Side homeSide) const
{
    QList<QPair<int, int>> rcPairs;
    int rlow = homeSide == Side::HERE ? 0 : 7,
        rhigh = homeSide == Side::HERE ? 3 : SEATROW;
    for (int row = rlow; row < rhigh; ++row)
        for (int col = 3; col < 6; ++col)
            rcPairs.append({ row, col });

    return rcPairs;
}

QList<QPair<int, int>> King::moveRowCol(int row, int col, Side homeSide) const
{
    Q_UNUSED(homeSide);

    QList<QPair<int, int>> rcPairs {
        { row, col - 1 }, { row, col + 1 },
        { row - 1, col }, { row + 1, col }
    };
    return getValidRowCol(rcPairs);
}

QList<QPair<int, int>> Advisor::putRowCol(Side homeSide) const
{
    QList<QPair<int, int>> rcPairs;
    int rlow = homeSide == Side::HERE ? 0 : 7,
        rhigh = homeSide == Side::HERE ? 3 : SEATROW;
    rcPairs.append({ rlow + 1, 4 });
    for (int row = rlow; row < rhigh; ++row)
        for (int col = 3; col < 6; col += 2)
            rcPairs.append({ row, col });

    return rcPairs;
}

QList<QPair<int, int>> Advisor::moveRowCol(int row, int col, Side homeSide) const
{
    Q_UNUSED(homeSide);

    QList<QPair<int, int>> rcPairs {
        { row - 1, col - 1 }, { row - 1, col + 1 },
        { row + 1, col - 1 }, { row + 1, col + 1 }
    };
    return getValidRowCol(rcPairs);
}

QList<QPair<int, int>> Bishop::putRowCol(Side homeSide) const
{
    QList<QPair<int, int>> rcPairs;
    int rlow = homeSide == Side::HERE ? 0 : 5,
        rhigh = homeSide == Side::HERE ? 5 : SEATROW;
    for (int row = rlow; row < rhigh; row += 4)
        for (int col = 2; col < SEATCOL; col += 4)
            rcPairs.append({ row, col });
    for (int col = 0; col < SEATCOL; col += 4)
        rcPairs.append({ rlow + 2, col });

    return rcPairs;
}

QList<QPair<int, int>> Bishop::moveRowCol(int row, int col, Side homeSide) const
{
    Q_UNUSED(homeSide);

    QList<QPair<int, int>> rcPairs {
        { row - 2, col - 2 }, { row - 2, col + 2 },
        { row + 2, col - 2 }, { row + 2, col + 2 }
    };
    return getValidRowCol(rcPairs);
}

QList<QPair<int, int>> Knight::putRowCol(Side homeSide) const
{
    Q_UNUSED(homeSide);

    return allPutRowCol();
}

QList<QPair<int, int>> Knight::moveRowCol(int row, int col, Side homeSide) const
{
    Q_UNUSED(homeSide);

    QList<QPair<int, int>> rcPairs {
        { row - 2, col - 1 }, { row - 2, col + 1 },
        { row - 1, col - 2 }, { row - 1, col + 2 },
        { row + 1, col - 2 }, { row + 1, col + 2 },
        { row + 2, col - 1 }, { row + 2, col + 1 }
    };
    return getValidRowCol(rcPairs);
}

QList<QPair<int, int>> Rook::putRowCol(Side homeSide) const
{
    Q_UNUSED(homeSide);

    return allPutRowCol();
}

QList<QPair<int, int>> Rook::moveRowCol(int row, int col, Side homeSide) const
{
    Q_UNUSED(homeSide);

    return Piece::rookCannonmoveRowCol(row, col);
}

QList<QPair<int, int>> Cannon::putRowCol(Side homeSide) const
{
    Q_UNUSED(homeSide);

    return allPutRowCol();
}

QList<QPair<int, int>> Cannon::moveRowCol(int row, int col, Side homeSide) const
{
    Q_UNUSED(homeSide);

    return Piece::rookCannonmoveRowCol(row, col);
}

QList<QPair<int, int>> Pawn::putRowCol(Side homeSide) const
{
    QList<QPair<int, int>> rcPairs;
    int rlow = homeSide == Side::HERE ? 3 : 5,
        rhigh = homeSide == Side::HERE ? 5 : 7;
    for (int row = rlow; row < rhigh; ++row)
        for (int col = 0; col < SEATCOL; col += 2)
            rcPairs.append({ row, col });

    rlow = homeSide == Side::HERE ? 5 : 0;
    rhigh = homeSide == Side::HERE ? SEATROW : 5;
    for (int row = rlow; row < rhigh; ++row)
        for (int col = 0; col < SEATCOL; ++col)
            rcPairs.append({ row, col });

    return rcPairs;
}

QList<QPair<int, int>> Pawn::moveRowCol(int row, int col, Side homeSide) const
{
    QList<QPair<int, int>> rcPairs {
        { row, col - 1 }, { row, col + 1 }
    };
    rcPairs.append({ row + (homeSide == Side::HERE ? 1 : -1), col });

    return getValidRowCol(rcPairs);
}

Pieces::Pieces()
{
    for (Color color : allColorList)
        for (Kind kind : allKindList) {
            auto& kpies = pieces_[color][kind];
            switch (kind) {
            case Kind::KING:
                kpies.append(new King(color, kind));
                break;
            case Kind::ADVISOR:
                for (int i = 0; i < 2; ++i)
                    kpies.append(new Advisor(color, kind));
                break;
            case Kind::BISHOP:
                for (int i = 0; i < 2; ++i)
                    kpies.append(new Bishop(color, kind));
                break;
            case Kind::KNIGHT:
                for (int i = 0; i < 2; ++i)
                    kpies.append(new Knight(color, kind));
                break;
            case Kind::ROOK:
                for (int i = 0; i < 2; ++i)
                    kpies.append(new Rook(color, kind));
                break;
            case Kind::CANNON:
                for (int i = 0; i < 2; ++i)
                    kpies.append(new Cannon(color, kind));
                break;
            default: // Kind::PAWN
                for (int i = 0; i < 5; ++i)
                    kpies.append(new Pawn(color, kind));
                break;
            }
        }
}

Pieces::~Pieces()
{
    for (Color color : allColorList)
        for (Kind kind : allKindList) {
            auto& kpies = pieces_[color][kind];
            for (int i = 0; i < kpies.count(); ++i)
                delete kpies[i];
        }
}

QList<PPiece> Pieces::getAllPiece(bool onlyKind) const
{
    QList<PPiece> pieceList;
    for (Color color : allColorList) {
        for (Kind kind : allKindList) {
            if (onlyKind)
                pieceList.append(pieces_[color][kind][0]);
            else
                pieceList.append(pieces_[color][kind]);
        }
    }

    return pieceList;
}

QList<PPiece> Pieces::getColorPiece(Color color) const
{
    QList<PPiece> pieceList;
    for (Kind kind : allKindList)
        pieceList.append(pieces_[color][kind]);

    return pieceList;
}

QString Pieces::getNameChars() const
{
    QString names;
    for (auto& piece : getAllPiece(true))
        names.append(piece->name());

    return names;
}

QString Pieces::getZhChars()
{
    return (preChars_ + getNameChars() + movChars_
        + numChars_[Color::RED] + numChars_[Color::BLACK]);
}

QString Pieces::getChChars()
{
    QString names;
    for (auto& piece : getAllPiece(true))
        names.append(piece->ch());

    return names;
}

bool Pieces::isKindName(QChar name, QList<Kind> kinds)
{
    for (Color color : allColorList)
        for (Kind kind : kinds)
            if (name == getColorKindPiece(color, kind)[0]->name())
                return true;

    return false;
}

int Pieces::getIndex(const int seatsLen, const bool isBottom, QChar preChar)
{
    int index = getPreChars_(seatsLen).indexOf(preChar);
    return isBottom ? seatsLen - 1 - index : index;
}

const QString Pieces::getPreChars_(int length)
{
    return (length == 2 ? QString(preChars_).remove(1, 1) // "前后"
                        : (length == 3 ? preChars_ // "前中后"
                                       : numChars_[Color::RED].left(5))); // "一二三四五";
}

const QString Pieces::preChars_ { "前中后" };
const QString Pieces::movChars_ { "退平进" };
const QStringList Pieces::numChars_ {
    "一二三四五六七八九",
    "１２３４５６７８９"
};
const QString Pieces::ICCS_ColChars_ { "abcdefghi" };
const QString Pieces::ICCS_RowChars_ { "0123456789" };
const QString Pieces::FENStr_ { "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR" };
const QChar Pieces::nullChar_ { '_' };
const QChar Pieces::FENSplitChar_ { '/' };
