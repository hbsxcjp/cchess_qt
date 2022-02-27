#include "piece.h"
#include "seat.h"

Piece::Piece(Color color, Kind kind)
    : color_(color)
    , kind_(kind)
    , seat_(nullptr)
{
}

QChar Piece::ch() const
{
    return Pieces::chars[int(color_)][int(kind_)];
}

QChar Piece::name() const
{
    return Pieces::nameChars[int(color_)][int(kind_)];
}

QList<SeatCoord> Piece::putTo(Side homeSide) const
{
    Q_UNUSED(homeSide)

    return Seats::allSeatCoord();
}

QList<QList<SeatCoord>> Piece::moveTo(const Seats* seats, Side homeSide) const
{
    QList<SeatCoord> canMoveSeatCoordList { moveTo_(homeSide) };

    // 排除不符合走棋规则的位置
    QList<SeatCoord> ruleSeatCoordList { ruleFiter_(seats, canMoveSeatCoordList) };

    // 排除同色棋子的位置
    QList<SeatCoord> colorSeatCoordList { colorFilter_(seats, canMoveSeatCoordList) };

    return { canMoveSeatCoordList, ruleSeatCoordList, colorSeatCoordList };
}

QList<SeatCoord> Piece::ruleFiter_(const Seats* seats, QList<SeatCoord>& seatCoordList) const
{
    Q_UNUSED(seats)
    Q_UNUSED(seatCoordList)

    return {};
}

QList<SeatCoord> Piece::colorFilter_(const Seats* seats, QList<SeatCoord>& seatCoordList) const
{
    QList<SeatCoord> colorSeatCoordList;

    // 排除目标同色的位置
    QMutableListIterator<SeatCoord> seatCoordIter(seatCoordList);
    while (seatCoordIter.hasNext()) {
        auto& seatCoord = seatCoordIter.next();
        if (isSameColor(seats->getSeat(seatCoord)->getPiece())) {
            colorSeatCoordList.append(seatCoord);
            seatCoordIter.remove();
        }
    }

    return colorSeatCoordList;
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
    return QString("%1%2%3").arg(QChar(color_ == Color::RED ? L'红' : L'黑')).arg(printName()).arg(ch());
}

QList<SeatCoord> King::putTo(Side homeSide) const
{
    return Seats::kingPutTo(homeSide);
}

QList<SeatCoord> King::moveTo_(Side homeSide) const
{
    Q_UNUSED(homeSide)

    return Seats::kingMoveTo(getSeat());
}

QList<SeatCoord> Advisor::putTo(Side homeSide) const
{
    return Seats::advisorPutTo(homeSide);
}

QList<SeatCoord> Advisor::moveTo_(Side homeSide) const
{
    return Seats::advisorMoveTo(getSeat(), homeSide);
}

QList<SeatCoord> Bishop::putTo(Side homeSide) const
{
    return Seats::bishopPutTo(homeSide);
}

QList<SeatCoord> Bishop::moveTo_(Side homeSide) const
{
    Q_UNUSED(homeSide)

    return Seats::bishopMoveTo(getSeat());
}

QList<SeatCoord> Bishop::ruleFiter_(const Seats* seats, QList<SeatCoord>& seatCoordList) const
{
    return seats->bishopRuleFilter(getSeat(), seatCoordList);
}

QList<SeatCoord> Knight::moveTo_(Side homeSide) const
{
    Q_UNUSED(homeSide)

    return Seats::knightMoveTo(getSeat());
}

QList<SeatCoord> Knight::ruleFiter_(const Seats* seats, QList<SeatCoord>& seatCoordList) const
{
    return seats->knightRuleFilter(getSeat(), seatCoordList);
}

QList<SeatCoord> Rook::moveTo_(Side homeSide) const
{
    Q_UNUSED(homeSide)

    return Seats::rookCannonMoveTo(getSeat());
}

QList<SeatCoord> Rook::ruleFiter_(const Seats* seats, QList<SeatCoord>& seatCoordList) const
{
    return seats->rookRuleFilter(seatCoordList);
}

QList<SeatCoord> Cannon::moveTo_(Side homeSide) const
{
    Q_UNUSED(homeSide)

    return Seats::rookCannonMoveTo(getSeat());
}

QList<SeatCoord> Cannon::ruleFiter_(const Seats* seats, QList<SeatCoord>& seatCoordList) const
{
    return seats->cannonRuleFilter(seatCoordList);
}

QList<SeatCoord> Pawn::putTo(Side homeSide) const
{
    return Seats::pawnPutTo(homeSide);
}

QList<SeatCoord> Pawn::moveTo_(Side homeSide) const
{
    return Seats::pawnMoveTo(getSeat(), homeSide);
}

Pieces::Pieces()
{
    QVector<int> kindNum { 1, 2, 2, 2, 2, 2, 5 };
    for (Color color : allColorList)
        for (Kind kind : allKindList) {
            auto& pieceList = pieces_[int(color)][int(kind)];
            for (int i = kindNum[int(kind)]; i > 0; --i)
                pieceList.append(getPiece_(color, kind));
        }
}

Pieces::~Pieces()
{
    for (int color = 0; color < COLORNUM; ++color)
        for (int kind = 0; kind < KINDNUM; ++kind)
            for (auto& piece : pieces_[color][kind])
                delete piece;
}

PPiece Pieces::getNotLivePiece(QChar ch) const
{
    if (ch == Pieces::nullChar)
        return nullptr;

    for (auto& piece : getColorKindPiece(getColor(ch), getKind(ch)))
        if (!piece->getSeat())
            return piece;

    return nullptr;
}

PPiece Pieces::getOtherPiece(const PPiece& piece) const
{
    if (!piece)
        return piece;

    Color color = piece->color();
    Kind kind = piece->kind();
    int index = getColorKindPiece(color, kind).indexOf(piece);
    return getColorKindPiece(getOtherColor(color), kind).at(index);
}

QList<PPiece> Pieces::getColorPiece(Color color) const
{
    QList<PPiece> pieceList;
    for (Kind kind : allKindList)
        pieceList.append(pieces_[int(color)][int(kind)]);

    return pieceList;
}

QList<PPiece> Pieces::getColorKindPiece(Color color, Kind kind) const
{
    return pieces_[int(color)][int(kind)];
}

PSeat Pieces::getKingSeat(Color color) const
{
    return getColorKindPiece(color, Kind::KING)[0]->getSeat();
}

QList<PSeat> Pieces::getLiveSeatList(Color color) const
{
    return getLiveSeatList_(getColorPiece(color));
}

QList<PSeat> Pieces::getLiveSeatList(Color color, Kind kind) const
{
    return getLiveSeatList_(getColorKindPiece(color, kind));
}

QList<PSeat> Pieces::getLiveSeatList(Color color, QChar name) const
{
    int index = nameChars[int(color)].indexOf(name);
    Q_ASSERT(index > -1);

    return getLiveSeatList(color, Kind(index));
}

QList<PSeat> Pieces::getLiveSeatList(Color color, QChar name, int col) const
{
    QList<PSeat> seatList { getLiveSeatList(color, name) };
    QMutableListIterator<PSeat> seatIter(seatList);
    while (seatIter.hasNext())
        if (seatIter.next()->col() != col)
            seatIter.remove();

    return seatList;
}

QList<PSeat> Pieces::getSortPawnLiveSeatList(Color color, bool isBottom) const
{
    // 最多5个兵, 按列、行建立映射，按列、行排序
    QMap<int, QMap<int, PSeat>> colRowSeatList;
    for (auto& seat : getLiveSeatList(color, Kind::PAWN)) {
        // 根据isBottom值排序
        int colIndex = isBottom ? -seat->col() : seat->col(),
            rowIndex = isBottom ? -seat->row() : seat->row();
        colRowSeatList[colIndex][rowIndex] = seat;
    }

    QList<PSeat> seatList;
    for (auto& colSeatList : colRowSeatList.values()) {
        // 舍弃少于2个的QMap
        if (colSeatList.size() > 1)
            seatList.append(colSeatList.values());
    }

    return seatList;
}

QString Pieces::toString() const
{
    QString qstr;
    for (auto color : allColorList)
        for (auto& piece : getColorPiece(color))
            qstr.append(piece->toString());

    return qstr;
}

QString Pieces::getZhChars()
{
    return (preChars + nameChars.join("") + movChars + numChars.join(""));
}

QString Pieces::getIccsChars()
{
    return Pieces::iccsRowChars + Pieces::iccsColChars;
}

Kind Pieces::getKind(QChar ch)
{
    int index = chars.join("").indexOf(ch);
    Q_ASSERT(index > -1);

    return Kind(index % KINDNUM);
}

bool Pieces::isKindName(QChar name, QList<Kind> kinds)
{
    for (auto kind : kinds)
        if (nameChars[int(Color::RED)][int(kind)] == name
            || nameChars[int(Color::BLACK)][int(kind)] == name)
            return true;

    return false;
}

bool Pieces::isPiece(QChar name)
{
    return nameChars.join("").indexOf(name) >= 0;
}

int Pieces::getRowFrom(QChar ch)
{
    return iccsRowChars.indexOf(ch);
}

int Pieces::getColFrom(QChar ch)
{
    return iccsColChars.indexOf(ch);
}

QChar Pieces::getOtherChar(QChar ch)
{
    return ch.isLetter() ? (ch.isUpper() ? ch.toLower() : ch.toUpper()) : ch;
}

QChar Pieces::getIccsChar(int col)
{
    return iccsColChars.at(col);
}

Color Pieces::getColor(QChar ch)
{
    return ch.isLower() ? Color::BLACK : Color::RED;
}

Color Pieces::getOtherColor(Color color)
{
    return color == Color::RED ? Color::BLACK : Color::RED;
}

Color Pieces::getColorFromZh(QChar numZh)
{
    return numChars[int(Color::RED)].indexOf(numZh) >= 0 ? Color::RED : Color::BLACK;
}

int Pieces::getIndex(const int seatsLen, const bool isBottom, QChar preChar)
{
    int index = getPreChars_(seatsLen).indexOf(preChar);
    return isBottom ? seatsLen - 1 - index : index;
}

QChar Pieces::getIndexChar(int seatsLen, bool isBottom, int index)
{
    return getPreChars_(seatsLen).at(isBottom ? seatsLen - 1 - index : index);
}

int Pieces::getMovNum(bool isBottom, QChar movChar)
{
    return (movChars.indexOf(movChar) - 1) * (isBottom ? 1 : -1);
}

QChar Pieces::getMovChar(bool isSameRow, bool isBottom, bool isLowToUp)
{
    return movChars.at(isSameRow ? 1 : (isBottom == isLowToUp ? 2 : 0));
}

int Pieces::getNum(Color color, QChar numChar)
{
    return numChars[int(color)].indexOf(numChar) + 1;
}

QChar Pieces::getNumChar(Color color, int num)
{
    return numChars[int(color)].at(num - 1);
}

int Pieces::getCol(bool isBottom, int num)
{
    return isBottom ? SEATCOL - num : num - 1;
}

QChar Pieces::getColChar(Color color, bool isBottom, int col)
{
    return numChars[int(color)].at(isBottom ? SEATCOL - col - 1 : col);
}

PPiece Pieces::getPiece_(Color color, Kind kind)
{
    switch (kind) {
    case Kind::KING:
        return new King(color, kind);
    case Kind::ADVISOR:
        return new Advisor(color, kind);
    case Kind::BISHOP:
        return new Bishop(color, kind);
    case Kind::KNIGHT:
        return new Knight(color, kind);
    case Kind::ROOK:
        return new Rook(color, kind);
    case Kind::CANNON:
        return new Cannon(color, kind);
    default: // Kind::PAWN
        return new Pawn(color, kind);
    }
}

QList<PSeat> Pieces::getLiveSeatList_(const QList<PPiece>& pieceList)
{
    QList<PSeat> seatList;
    for (auto& piece : pieceList)
        if (piece->getSeat())
            seatList.append(piece->getSeat());

    return seatList;
}

const QString Pieces::getPreChars_(int length)
{
    return (length == 2 ? QString(preChars).remove(1, 1) // "前后"
                        : (length == 3 ? preChars // "前中后"
                                       : numChars[int(Color::RED)].left(5))); // "一二三四五";
}

const QList<Color> Pieces::allColorList { Color::RED, Color::BLACK };

const QList<Kind> Pieces::allKindList {
    Kind::KING, Kind::ADVISOR, Kind::BISHOP,
    Kind::KNIGHT, Kind::ROOK, Kind::CANNON, Kind::PAWN
};

const QList<Kind> Pieces::strongeKindList {
    Kind::KNIGHT, Kind::ROOK, Kind::CANNON, Kind::PAWN
};

const QList<Kind> Pieces::lineKindList {
    Kind::KING, Kind::ROOK, Kind::CANNON, Kind::PAWN
};

const QString Pieces::preChars { "前中后" };
const QString Pieces::movChars { "退平进" };
const QStringList Pieces::chars {
    "KABNRCP",
    "kabnrcp"
};
const QStringList Pieces::nameChars {
    "帅仕相马车炮兵",
    "将士象马车炮卒"
};
const QStringList Pieces::numChars {
    "一二三四五六七八九",
    "１２３４５６７８９"
};
const QString Pieces::iccsColChars { "abcdefghi" };
const QString Pieces::iccsRowChars { "0123456789" };
const QString Pieces::FENStr { "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR" };
const QChar Pieces::nullChar { '_' };
const QChar Pieces::FENSplitChar { '/' };
