#include "piece.h"
#include "seat.h"

Piece::Piece(Color color, Kind kind)
    : color_(color)
    , kind_(kind)
    , seat_(nullptr)
{
}

QList<SeatCoord> Piece::putSeatCoord(Side homeSide) const
{
    Q_UNUSED(homeSide);

    return Seats::allSeatCoord();
}

QList<QList<SeatCoord>> Piece::canMoveSeatCoord(const Seats* seats, Side homeSide) const
{
    QList<SeatCoord> seatCoordList { moveSeatCoord(homeSide) };
    // 排除不符合走棋规则的位置
    QList<SeatCoord> ruleSeatCoords { filterRuleSeatCoord(seats, seatCoordList) };
    // 排除同色棋子的位置
    QList<SeatCoord> colorSeatCoords { filterColorSeatCoord(seats, seatCoordList) };

    return { seatCoordList, ruleSeatCoords, colorSeatCoords };
}

QList<SeatCoord> Piece::filterRuleSeatCoord(const Seats* seats, QList<SeatCoord>& seatCoordList) const
{
    Q_UNUSED(seats);
    Q_UNUSED(seatCoordList);

    return {};
}

QList<SeatCoord> Piece::filterColorSeatCoord(const Seats* seats, QList<SeatCoord>& seatCoordList) const
{
    QList<SeatCoord> colorSeatCoords;
    // 排除目标同色的位置
    QMutableListIterator<SeatCoord> seatCoordIter(seatCoordList);
    while (seatCoordIter.hasNext()) {
        auto& seatCoord = seatCoordIter.next();
        if (isSameColor(seats->getSeat(seatCoord)->getPiece())) {
            colorSeatCoords.append(seatCoord);
            seatCoordIter.remove();
        }
    }

    return colorSeatCoords;
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

QList<SeatCoord> King::putSeatCoord(Side homeSide) const
{
    return Seats::kingSeatCoord(homeSide);
}

QList<SeatCoord> King::moveSeatCoord(Side homeSide) const
{
    Q_UNUSED(homeSide);
    return Seats::kingMoveSeatCoord(getSeat());
}

QList<SeatCoord> Advisor::putSeatCoord(Side homeSide) const
{
    return Seats::advisorSeatCoord(homeSide);
}

QList<SeatCoord> Advisor::moveSeatCoord(Side homeSide) const
{
    return Seats::advisorMoveSeatCoord(getSeat(), homeSide);
}

QList<SeatCoord> Bishop::putSeatCoord(Side homeSide) const
{
    return Seats::bishopSeatCoord(homeSide);
}

QList<SeatCoord> Bishop::moveSeatCoord(Side homeSide) const
{
    Q_UNUSED(homeSide);
    return Seats::bishopMoveSeatCoord(getSeat());
}

QList<SeatCoord> Bishop::filterRuleSeatCoord(const Seats* seats, QList<SeatCoord>& seatCoordList) const
{
    return seats->bishopFilterRuleSeatCoord(getSeat(), seatCoordList);
}

QList<SeatCoord> Knight::moveSeatCoord(Side homeSide) const
{
    Q_UNUSED(homeSide);
    return Seats::knightMoveSeatCoord(getSeat());
}

QList<SeatCoord> Knight::filterRuleSeatCoord(const Seats* seats, QList<SeatCoord>& seatCoordList) const
{
    return seats->knightFilterRuleSeatCoord(getSeat(), seatCoordList);
}

QList<SeatCoord> Rook::moveSeatCoord(Side homeSide) const
{
    Q_UNUSED(homeSide);
    return Seats::rookCannonMoveSeatCoord(getSeat());
}

QList<SeatCoord> Rook::filterRuleSeatCoord(const Seats* seats, QList<SeatCoord>& seatCoordList) const
{
    return seats->rookFilterRuleSeatCoord(getSeat(), seatCoordList);
}

QList<SeatCoord> Cannon::moveSeatCoord(Side homeSide) const
{
    Q_UNUSED(homeSide);
    return Seats::rookCannonMoveSeatCoord(getSeat());
}

QList<SeatCoord> Cannon::filterRuleSeatCoord(const Seats* seats, QList<SeatCoord>& seatCoordList) const
{
    return seats->cannonFilterRuleSeatCoord(getSeat(), seatCoordList);
}

QList<SeatCoord> Pawn::putSeatCoord(Side homeSide) const
{
    return Seats::pawnSeatCoord(homeSide);
}

QList<SeatCoord> Pawn::moveSeatCoord(Side homeSide) const
{
    return Seats::pawnMoveSeatCoord(getSeat(), homeSide);
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

    for (auto& piece : getColorKindPiece(getColor(ch), getKind(ch))) {
        if (!piece->getSeat())
            return piece;
    }

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

QList<PPiece> Pieces::getAllPiece(bool onlyKind) const
{
    QList<PPiece> pieceList;
    for (Color color : allColorList) {
        if (onlyKind) {
            for (Kind kind : allKindList)
                pieceList.append(getColorKindPiece(color, kind)[0]);
        } else
            pieceList.append(getColorPiece(color));
    }

    return pieceList;
}

QList<PPiece> Pieces::getColorPiece(Color color) const
{
    QList<PPiece> pieceList;
    for (Kind kind : allKindList)
        pieceList.append(pieces_[int(color)][int(kind)]);

    return pieceList;
}

PSeat Pieces::getKingSeat(Color color) const
{
    auto pieceList = getColorKindPiece(color, Kind::KING);
    Q_ASSERT(pieceList.size() == 1);

    return pieceList[0]->getSeat();
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
    int index = getNameChars(color).indexOf(name);
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
        auto& colSeatList = colRowSeatList[isBottom ? -seat->col() : seat->col()];
        colSeatList[isBottom ? -seat->row() : seat->row()] = seat;
    }

    QList<PSeat> seatList;
    for (auto& colSeatList : colRowSeatList.values()) {
        // 舍弃少于2个的QMap
        if (colSeatList.size() > 1)
            for (auto& seat : colSeatList.values())
                seatList.append(seat);
    }

    return seatList;
}

QString Pieces::getNameChars() const
{
    return getNameChars(Color::RED) + getNameChars(Color::BLACK);
}

QString Pieces::getNameChars(Color color) const
{
    QString names;
    for (Kind kind : allKindList)
        names.append(getColorKindPiece(color, kind)[0]->name());

    return names;
}

QString Pieces::getNameChars(QList<Kind> kinds) const
{
    QString names;
    for (Color color : allColorList)
        for (Kind kind : kinds)
            names.append(getColorKindPiece(color, kind)[0]->name());

    return names;
}

QString Pieces::getZhChars() const
{
    return (preChars + getNameChars() + movChars
        + numChars[int(Color::RED)] + numChars[int(Color::BLACK)]);
}

QString Pieces::getChChars() const
{
    QString chChars;
    for (auto& piece : getAllPiece(true))
        chChars.append(piece->ch());

    return chChars;
}

Kind Pieces::getKind(QChar ch) const
{
    int index = getChChars().indexOf(ch);
    Q_ASSERT(index > -1);

    return Kind(index % KINDNUM);
}

bool Pieces::isKindName(QChar name, QList<Kind> kinds) const
{
    return getNameChars(kinds).contains(name);
}

QString Pieces::toString() const
{
    QString qstr;
    for (auto& piece : getAllPiece())
        qstr.append(piece->toString());

    return qstr;
}

int Pieces::getIndex(const int seatsLen, const bool isBottom, QChar preChar)
{
    int index = getPreChars_(seatsLen).indexOf(preChar);
    return isBottom ? seatsLen - 1 - index : index;
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
const QStringList Pieces::numChars {
    "一二三四五六七八九",
    "１２３４５６７８９"
};
const QString Pieces::ICCS_ColChars { "abcdefghi" };
const QString Pieces::ICCS_RowChars { "0123456789" };
const QString Pieces::FENStr { "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR" };
const QChar Pieces::nullChar { '_' };
const QChar Pieces::FENSplitChar { '/' };
