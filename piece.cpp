#include "piece.h"
#include "seat.h"
#include <functional>

Piece::Piece(Color color, Kind kind)
    : color_(color)
    , kind_(kind)
{
}

Piece::Color Piece::getColor(QChar ch)
{
    return ch.isLower() ? Color::BLACK : Color::RED;
}

Piece::Kind Piece::getKind(QChar ch)
{
    int k = Pieces::getChChars().indexOf(ch.toUpper());
    if (k < 0)
        return Kind::NOTKIND;

    return Kind(k);
}

QChar Piece::ch() const
{
    return Pieces::getChChars().at(color_ * KINDNUM + kind_);
}

QChar Piece::name() const
{
    switch (kind_) {
    case Kind::KING:
    case Kind::ADVISOR:
    case Kind::BISHOP:
        return Pieces::getNameChars().at(kind_ * 2 + color_);
    case Kind::KNIGHT:
    case Kind::ROOK:
    case Kind::CANNON:
        return Pieces::getNameChars().at(kind_ + 3);
    default: //    case Kind::PAWN:
        return Pieces::getNameChars().at(color_ + 9);
    }
}

QChar Piece::printName() const
{
    if (color_ == Color::BLACK
        && (kind_ == Kind::KNIGHT || kind_ == Kind::ROOK || kind_ == Kind::CANNON))
        return (kind_ == Kind::KNIGHT) ? L'馬' : (kind_ == Kind::ROOK ? L'車' : L'砲');
    else
        return name();
}

const QString Piece::toString() const
{
    return QString() + QChar(color_ == Color::RED ? L'红' : L'黑') + printName() + ch();
}

Pieces::Pieces()
{
    int kindNums[] { 1, 2, 2, 2, 2, 2, 5 };
    for (int c = 0; c < COLORNUM; ++c)
        for (int k = 0; k < KINDNUM; ++k) {
            auto& kpies = pieces_[c][k];
            for (int i = 0; i < kindNums[k]; ++i)
                kpies.append(new Piece(Piece::Color(c), Piece::Kind(k)));
        }
}

Pieces::~Pieces()
{
    for (int c = 0; c < COLORNUM; ++c)
        for (int k = 0; k < KINDNUM; ++k) {
            auto& kpies = pieces_[c][k];
            for (int i = 0; i < kpies.count(); ++i)
                delete kpies.at(i);
        }
}

QList<PPiece> Pieces::getAllPiece(bool onlyKind) const
{
    QList<PPiece> pieceList;
    for (int c = 0; c < COLORNUM; ++c) {
        for (int k = 0; k < KINDNUM; ++k) {
            if (onlyKind)
                pieceList.append(pieces_[c][k][0]);
            else
                pieceList.append(pieces_[c][k]);
        }
    }

    return pieceList;
}

QList<PPiece> Pieces::getColorPiece(Piece::Color color) const
{
    QList<PPiece> pieceList;
    for (int k = 0; k < KINDNUM; ++k)
        pieceList.append(pieces_[color][k]);

    return pieceList;
}

Piece::Color Pieces::getOtherColor(Piece::Color color)
{
    return color == Piece::Color::RED ? Piece::Color::BLACK : Piece::Color::RED;
}

const QString Pieces::getZhChars()
{
    return (preChars_ + nameChars_ + movChars_
        + numChars_[Piece::Color::RED] + numChars_[Piece::Color::BLACK]);
}

const QString Pieces::getICCSChars() { return ICCS_ColChars_ + ICCS_RowChars_; }

const QString& Pieces::getFENStr() { return FENStr_; }

const QString& Pieces::getChChars() { return chChars_; }

const QString& Pieces::getNameChars() { return nameChars_; }

const QChar Pieces::getFENSplitChar() { return FENSplitChar_; }

int Pieces::getRowFromICCSChar(QChar ch) { return ICCS_RowChars_.indexOf(ch); }

int Pieces::getColFromICCSChar(QChar ch) { return ICCS_ColChars_.indexOf(ch); }

QChar Pieces::getOtherChar(QChar ch)
{
    return ch.isLetter() ? (ch.isUpper() ? ch.toLower() : ch.toUpper()) : ch;
}

QChar Pieces::getColICCSChar(int col) { return ICCS_ColChars_.at(col); }

QChar Pieces::getName(QChar ch)
{
    //    int chIndex_nameIndex[][2] {
    //        { 0, 0 }, { 1, 2 }, { 2, 4 }, { 3, 6 }, { 4, 7 }, { 5, 8 }, { 6, 9 },
    //        { 7, 1 }, { 8, 3 }, { 9, 5 }, { 10, 6 }, { 11, 7 }, { 12, 8 }, { 13, 10 }
    //    };
    //    return nameChars_.at(chIndex_nameIndex[chChars_.indexOf(ch)][1]);
    int index = chChars_.indexOf(ch), kind = index % KINDNUM, color = index / KINDNUM;
    if (kind < Piece::Kind::BISHOP)
        return nameChars_.at(kind * 2 + color);
    else if (kind == Piece::Kind::PAWN)
        return nameChars_.at(color + 9);
    else
        return nameChars_.at(kind + 3);
}

Piece::Color Pieces::getColor(QChar ch)
{
    return ch.isLower() ? Piece::Color::BLACK : Piece::Color::RED;
}

Piece::Color Pieces::getColorFromZh(QChar numZh)
{
    return numChars_[Piece::Color::RED].indexOf(numZh) >= 0 ? Piece::Color::RED : Piece::Color::BLACK;
}

int Pieces::getIndex(const int seatsLen, const bool isBottom, QChar preChar)
{
    int index = getPreChars_(seatsLen).indexOf(preChar);
    return isBottom ? seatsLen - 1 - index : index;
}

QChar Pieces::getIndexChar(const int seatsLen, const bool isBottom, const int index)
{
    return getPreChars_(seatsLen).at(isBottom ? seatsLen - 1 - index : index);
}

QChar Pieces::nullChar() { return nullChar_; }

QChar Pieces::redKingChar() { return chChars_[0]; };

int Pieces::getMovNum(bool isBottom, QChar movChar)
{
    return (movChars_.indexOf(movChar) - 1) * (isBottom ? 1 : -1);
}

QChar Pieces::getMovChar(bool isSameRow, bool isBottom, bool isLowToUp)
{
    return movChars_.at(isSameRow ? 1 : (isBottom == isLowToUp ? 2 : 0));
}

int Pieces::getNum(Piece::Color color, QChar numChar)
{
    return numChars_[color].indexOf(numChar) + 1;
}

QChar Pieces::getNumChar(Piece::Color color, int num)
{
    return numChars_[color].at(num - 1);
}

int Pieces::getCol(bool isBottom, const int num)
{
    return isBottom ? SEATCOL - num : num - 1;
}

QChar Pieces::getColChar(const Piece::Color color, bool isBottom, const int col)
{
    return numChars_[color].at(isBottom ? SEATCOL - col - 1 : col);
}

bool Pieces::isKing(QChar name)
{
    return nameChars_.leftRef(2).indexOf(name) >= 0;
}

bool Pieces::isAdvBish(QChar name)
{
    return nameChars_.midRef(2, 4).indexOf(name) >= 0;
}

bool Pieces::isStronge(QChar name)
{
    return nameChars_.midRef(6, 5).indexOf(name) >= 0;
}

bool Pieces::isLineMove(QChar name)
{
    return isKing(name) || nameChars_.midRef(7, 4).indexOf(name) >= 0;
}

bool Pieces::isPawn(QChar name)
{
    return nameChars_.rightRef(2).indexOf(name) >= 0;
}

bool Pieces::isPiece(QChar name)
{
    return nameChars_.indexOf(name) >= 0;
}

const QString Pieces::getPreChars_(int length)
{
    return (length == 2 ? QString(preChars_).remove(1, 1) // "前后"
                        : (length == 3 ? preChars_ // "前中后"
                                       : numChars_[Piece::Color::RED].left(5))); // "一二三四五";
}

const QString Pieces::chChars_ { "KABNRCPkabnrcp" };
const QString Pieces::preChars_ { "前中后" };
const QString Pieces::nameChars_ { "帅将仕士相象马车炮兵卒" };
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
