#include "piece.h"
#include "seat.h"

Piece::Piece(PieceColor color, PieceKind kind)
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

QList<SeatCoord> Piece::putTo(SeatSide homeSide) const
{
    Q_UNUSED(homeSide)

    return Seats::allSeatCoord();
}

QList<QList<SeatCoord>> Piece::moveTo(const Seats* seats, SeatSide homeSide) const
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
    if (color_ == PieceColor::BLACK) {
        if (kind_ == PieceKind::KNIGHT)
            return L'馬';
        else if (kind_ == PieceKind::ROOK)
            return L'車';
        else if (kind_ == PieceKind::CANNON)
            return L'砲';
    }

    return name();
}

QString Piece::toString() const
{
    return QString("%1%2%3").arg(QChar(color_ == PieceColor::RED ? L'红' : L'黑')).arg(printName()).arg(ch());
}

QList<SeatCoord> King::putTo(SeatSide homeSide) const
{
    return Seats::kingPutTo(homeSide);
}

QList<SeatCoord> King::moveTo_(SeatSide homeSide) const
{
    Q_UNUSED(homeSide)

    return Seats::kingMoveTo(getSeat());
}

QList<SeatCoord> Advisor::putTo(SeatSide homeSide) const
{
    return Seats::advisorPutTo(homeSide);
}

QList<SeatCoord> Advisor::moveTo_(SeatSide homeSide) const
{
    return Seats::advisorMoveTo(getSeat(), homeSide);
}

QList<SeatCoord> Bishop::putTo(SeatSide homeSide) const
{
    return Seats::bishopPutTo(homeSide);
}

QList<SeatCoord> Bishop::moveTo_(SeatSide homeSide) const
{
    Q_UNUSED(homeSide)

    return Seats::bishopMoveTo(getSeat());
}

QList<SeatCoord> Bishop::ruleFiter_(const Seats* seats, QList<SeatCoord>& seatCoordList) const
{
    return seats->bishopRuleFilter(getSeat(), seatCoordList);
}

QList<SeatCoord> Knight::moveTo_(SeatSide homeSide) const
{
    Q_UNUSED(homeSide)

    return Seats::knightMoveTo(getSeat());
}

QList<SeatCoord> Knight::ruleFiter_(const Seats* seats, QList<SeatCoord>& seatCoordList) const
{
    return seats->knightRuleFilter(getSeat(), seatCoordList);
}

QList<SeatCoord> Rook::moveTo_(SeatSide homeSide) const
{
    Q_UNUSED(homeSide)

    return Seats::rookCannonMoveTo(getSeat());
}

QList<SeatCoord> Rook::ruleFiter_(const Seats* seats, QList<SeatCoord>& seatCoordList) const
{
    return seats->rookRuleFilter(seatCoordList);
}

QList<SeatCoord> Cannon::moveTo_(SeatSide homeSide) const
{
    Q_UNUSED(homeSide)

    return Seats::rookCannonMoveTo(getSeat());
}

QList<SeatCoord> Cannon::ruleFiter_(const Seats* seats, QList<SeatCoord>& seatCoordList) const
{
    return seats->cannonRuleFilter(seatCoordList);
}

QList<SeatCoord> Pawn::putTo(SeatSide homeSide) const
{
    return Seats::pawnPutTo(homeSide);
}

QList<SeatCoord> Pawn::moveTo_(SeatSide homeSide) const
{
    return Seats::pawnMoveTo(getSeat(), homeSide);
}

Pieces::Pieces()
{
    QVector<int> kindNum { 1, 2, 2, 2, 2, 2, 5 };
    for (PieceColor color : allColorList)
        for (PieceKind kind : allKindList) {
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

    PieceColor color = piece->color();
    PieceKind kind = piece->kind();
    int index = getColorKindPiece(color, kind).indexOf(piece);
    return getColorKindPiece(getOtherColor(color), kind).at(index);
}

QList<PPiece> Pieces::getColorPiece(PieceColor color) const
{
    QList<PPiece> pieceList;
    for (PieceKind kind : allKindList)
        pieceList.append(pieces_[int(color)][int(kind)]);

    return pieceList;
}

QList<PPiece> Pieces::getColorKindPiece(PieceColor color, PieceKind kind) const
{
    return pieces_[int(color)][int(kind)];
}

PSeat Pieces::getKingSeat(PieceColor color) const
{
    return getColorKindPiece(color, PieceKind::KING)[0]->getSeat();
}

QList<PSeat> Pieces::getLiveSeatList(PieceColor color) const
{
    return getLiveSeatList_(getColorPiece(color));
}

QList<PSeat> Pieces::getLiveSeatList(PieceColor color, PieceKind kind) const
{
    return getLiveSeatList_(getColorKindPiece(color, kind));
}

QList<PSeat> Pieces::getLiveSeatList(PieceColor color, QChar name) const
{
    int index = nameChars[int(color)].indexOf(name);
    Q_ASSERT(index > -1);

    return getLiveSeatList(color, PieceKind(index));
}

QList<PSeat> Pieces::getLiveSeatList(PieceColor color, QChar name, int col) const
{
    QList<PSeat> seatList { getLiveSeatList(color, name) };
    QMutableListIterator<PSeat> seatIter(seatList);
    while (seatIter.hasNext())
        if (seatIter.next()->col() != col)
            seatIter.remove();

    return seatList;
}

QList<PSeat> Pieces::getSortPawnLiveSeatList(PieceColor color, bool isBottom) const
{
    // 最多5个兵, 按列、行建立映射，按列、行排序
    QMap<int, QMap<int, PSeat>> colRowSeatList;
    for (auto& seat : getLiveSeatList(color, PieceKind::PAWN)) {
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

PieceKind Pieces::getKind(QChar ch)
{
    int index = chars.join("").indexOf(ch);
    Q_ASSERT(index > -1);

    return PieceKind(index % KINDNUM);
}

bool Pieces::isKindName(QChar name, QList<PieceKind> kinds)
{
    for (auto kind : kinds)
        if (nameChars[int(PieceColor::RED)][int(kind)] == name
            || nameChars[int(PieceColor::BLACK)][int(kind)] == name)
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

PieceColor Pieces::getColor(QChar ch)
{
    return ch.isLower() ? PieceColor::BLACK : PieceColor::RED;
}

PieceColor Pieces::getOtherColor(PieceColor color)
{
    return color == PieceColor::RED ? PieceColor::BLACK : PieceColor::RED;
}

PieceColor Pieces::getColorFromZh(QChar numZh)
{
    return numChars[int(PieceColor::RED)].indexOf(numZh) >= 0 ? PieceColor::RED : PieceColor::BLACK;
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

int Pieces::getNum(PieceColor color, QChar numChar)
{
    return numChars[int(color)].indexOf(numChar) + 1;
}

QChar Pieces::getNumChar(PieceColor color, int num)
{
    return numChars[int(color)].at(num - 1);
}

int Pieces::getCol(bool isBottom, int num)
{
    return isBottom ? SEATCOL - num : num - 1;
}

QChar Pieces::getColChar(PieceColor color, bool isBottom, int col)
{
    return numChars[int(color)].at(isBottom ? SEATCOL - col - 1 : col);
}

PPiece Pieces::getPiece_(PieceColor color, PieceKind kind)
{
    switch (kind) {
    case PieceKind::KING:
        return new King(color, kind);
    case PieceKind::ADVISOR:
        return new Advisor(color, kind);
    case PieceKind::BISHOP:
        return new Bishop(color, kind);
    case PieceKind::KNIGHT:
        return new Knight(color, kind);
    case PieceKind::ROOK:
        return new Rook(color, kind);
    case PieceKind::CANNON:
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
                                       : numChars[int(PieceColor::RED)].left(5))); // "一二三四五";
}

const QList<PieceColor> Pieces::allColorList { PieceColor::RED, PieceColor::BLACK };

const QList<PieceKind> Pieces::allKindList {
    PieceKind::KING, PieceKind::ADVISOR, PieceKind::BISHOP,
    PieceKind::KNIGHT, PieceKind::ROOK, PieceKind::CANNON, PieceKind::PAWN
};

const QList<PieceKind> Pieces::strongeKindList {
    PieceKind::KNIGHT, PieceKind::ROOK, PieceKind::CANNON, PieceKind::PAWN
};

const QList<PieceKind> Pieces::lineKindList {
    PieceKind::KING, PieceKind::ROOK, PieceKind::CANNON, PieceKind::PAWN
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
const QString Pieces::allChars { "KAABBNNRRCCPPPPPkaabbnnrrccppppp" };
const QChar Pieces::nullChar { '_' };
const QChar Pieces::FENSplitChar { '/' };
