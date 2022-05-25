#include "seatbase.h"
#include "boardpieces.h"
#include "boardseats.h"
#include "piece.h"
#include "piecebase.h"
#include "seat.h"

static const int ROWNUM { 10 };
static const int COLNUM { 9 };

static const int NONROWCOLVALUE { -1 };
static const int MINROWCOL { 0 };
static const int KINGADVLOWMAXROW { 2 };
static const int KINGADVHIGHMINROW { 7 };
static const int KINGADVMINCOL { 3 };
static const int KINGADVMAXCOL { 5 };
static const int BISHOPLOWMAXROW { 4 };
static const QList<int> BISHOPROWS { { 0, 2, 4, 5, 7, 9 } };

static const QChar FENSPLITCHAR { '/' };

QList<Coord> SeatBase::allCoord()
{
    QList<Coord> coords;
    for (int row = MINROWCOL; row < ROWNUM; ++row)
        for (int col = MINROWCOL; col < COLNUM; ++col)
            coords.append({ row, col });

    return coords;
}

int SeatBase::getRowNum()
{
    return ROWNUM;
}

int SeatBase::getColNum()
{
    return COLNUM;
}

int SeatBase::getSeatNum()
{
    return ROWNUM * COLNUM;
}

bool SeatBase::isBottom(const Coord& coord)
{
    return coord.first <= BISHOPLOWMAXROW;
}

Coord SeatBase::getCoord(int index)
{
    return { index / COLNUM, index % COLNUM };
}

int SeatBase::getIndex(const Coord& coord)
{
    return coord.first * COLNUM + coord.second;
}

int SeatBase::symmetryRow(int row)
{
    return ROWNUM - 1 - row;
}

int SeatBase::symmetryCol(int col)
{
    return COLNUM - 1 - col;
}

Coord SeatBase::changeCoord(const Coord& coord, ChangeType ct)
{
    if (ct == ChangeType::SYMMETRY_H)
        return { coord.first, symmetryCol(coord.second) };
    else if (ct == ChangeType::ROTATE)
        return { symmetryRow(coord.first), symmetryCol(coord.second) };
    else if (ct == ChangeType::SYMMETRY_V)
        return { symmetryRow(coord.first), coord.second };
    else
        //(ct == ChangeType::NOCHANGE || ct == ChangeType::EXCHANGE)
        return coord;
}

QString SeatBase::pieCharsToFEN(const QString& pieChars)
{
    QString fen {};
    int seatNum { SeatBase::getSeatNum() };
    if (pieChars.count() != seatNum)
        return fen;

    for (int index = 0; index < seatNum; index += COLNUM) {
        QString line { pieChars.mid(index, COLNUM) }, qstr {};
        int num { 0 };
        for (auto ch : line) {
            if (ch != PieceBase::NULLCHAR) {
                if (num != 0) {
                    qstr.append(QString::number(num));
                    num = 0;
                }
                qstr.append(ch);
            } else
                ++num;
        }
        if (num != 0)
            qstr.append(QString::number(num));
        fen.prepend(qstr).prepend(FENSPLITCHAR);
    }
    fen.remove(0, 1);

    return fen;
}

QString SeatBase::FENToPieChars(const QString& fen)
{
    QStringList strList { fen.split(FENSPLITCHAR) };
    if (strList.count() != ROWNUM)
        return QString();

    QString pieceChars;
    for (auto& line : strList) {
        QString string;
        for (auto ch : line)
            string.append(ch.isDigit() ? QString(ch.digitValue(), PieceBase::NULLCHAR) : ch);
        pieceChars.prepend(string);
    }

    return pieceChars.size() == SeatBase::getSeatNum() ? pieceChars : QString();
}

const QList<QPair<Coord, QPair<PieceColor, PieceKind>>>& SeatBase::getInitCoordColorKinds()
{
    static const QList<QPair<Coord, QPair<PieceColor, PieceKind>>> initCoordColorKinds {
        { { 0, 0 }, { PieceColor::RED, PieceKind::ROOK } },
        { { 0, 1 }, { PieceColor::RED, PieceKind::KNIGHT } },
        { { 0, 2 }, { PieceColor::RED, PieceKind::BISHOP } },
        { { 0, 3 }, { PieceColor::RED, PieceKind::ADVISOR } },
        { { 0, 4 }, { PieceColor::RED, PieceKind::KING } },
        { { 0, 5 }, { PieceColor::RED, PieceKind::ADVISOR } },
        { { 0, 6 }, { PieceColor::RED, PieceKind::BISHOP } },
        { { 0, 7 }, { PieceColor::RED, PieceKind::KNIGHT } },
        { { 0, 8 }, { PieceColor::RED, PieceKind::ROOK } },
        { { 2, 1 }, { PieceColor::RED, PieceKind::CANNON } },
        { { 2, 7 }, { PieceColor::RED, PieceKind::CANNON } },
        { { 3, 0 }, { PieceColor::RED, PieceKind::PAWN } },
        { { 3, 2 }, { PieceColor::RED, PieceKind::PAWN } },
        { { 3, 4 }, { PieceColor::RED, PieceKind::PAWN } },
        { { 3, 6 }, { PieceColor::RED, PieceKind::PAWN } },
        { { 3, 8 }, { PieceColor::RED, PieceKind::PAWN } },
        { { 9, 0 }, { PieceColor::BLACK, PieceKind::ROOK } },
        { { 9, 1 }, { PieceColor::BLACK, PieceKind::KNIGHT } },
        { { 9, 2 }, { PieceColor::BLACK, PieceKind::BISHOP } },
        { { 9, 3 }, { PieceColor::BLACK, PieceKind::ADVISOR } },
        { { 9, 4 }, { PieceColor::BLACK, PieceKind::KING } },
        { { 9, 5 }, { PieceColor::BLACK, PieceKind::ADVISOR } },
        { { 9, 6 }, { PieceColor::BLACK, PieceKind::BISHOP } },
        { { 9, 7 }, { PieceColor::BLACK, PieceKind::KNIGHT } },
        { { 9, 8 }, { PieceColor::BLACK, PieceKind::ROOK } },
        { { 7, 1 }, { PieceColor::BLACK, PieceKind::CANNON } },
        { { 7, 7 }, { PieceColor::BLACK, PieceKind::CANNON } },
        { { 6, 0 }, { PieceColor::BLACK, PieceKind::PAWN } },
        { { 6, 2 }, { PieceColor::BLACK, PieceKind::PAWN } },
        { { 6, 4 }, { PieceColor::BLACK, PieceKind::PAWN } },
        { { 6, 6 }, { PieceColor::BLACK, PieceKind::PAWN } },
        { { 6, 8 }, { PieceColor::BLACK, PieceKind::PAWN } }
    };

    return initCoordColorKinds;
}

QPair<Coord, Coord> SeatBase::coordPair(const QString& rowcols)
{
    return { { rowcols.at(0).digitValue(), rowcols.at(1).digitValue() },
        { rowcols.at(2).digitValue(), rowcols.at(3).digitValue() } };
}

bool SeatBase::less(Seat* first, Seat* last)
{
    return first->row() < last->row();
}

QList<Coord> SeatBase::canPut(PieceKind kind, SeatSide homeSide)
{
    switch (kind) {
    case PieceKind::KING:
        return kingCanPut(homeSide);
    case PieceKind::ADVISOR:
        return advisorCanPut(homeSide);
    case PieceKind::BISHOP:
        return bishopCanPut(homeSide);
    case PieceKind::PAWN:
        return pawnCanPut(homeSide);
    default:
        return allCoord();
    }
}

QList<QList<Coord>> SeatBase::canMove(Piece* piece, const Coord& coord,
    const BoardSeats* boardSeats, SeatSide homeSide)
{
    QList<Coord> moveCoords { SeatBase::getCanMove(piece->kind(), coord, homeSide) };

    // 排除不符合走棋规则的位置
    QList<Coord> ruleCoords { SeatBase::filterMoveRule(boardSeats, piece->kind(), coord, moveCoords) };

    // 排除同色棋子的位置
    QList<Coord> colorCoords { SeatBase::filterColorRule(boardSeats, piece->color(), moveCoords) };

    return { moveCoords, ruleCoords, colorCoords };
}

QList<Coord> SeatBase::getCanMove(PieceKind kind, const Coord& coord, SeatSide homeSide)
{
    int row { coord.first }, col { coord.second };
    switch (kind) {
    case PieceKind::KING:
        return kingCanMove(row, col);
    case PieceKind::ADVISOR:
        return advisorCanMove(row, col, homeSide);
    case PieceKind::BISHOP:
        return bishopCanMove(row, col);
    case PieceKind::KNIGHT:
        return knightCanMove(row, col);
    case PieceKind::PAWN:
        return pawnCanMove(row, col, homeSide);
    default:
        return rookCannonCanMove(row, col);
    }
}

QList<Coord> SeatBase::filterMoveRule(const BoardSeats* boardSeats, PieceKind kind,
    const Coord& coord, QList<Coord>& coords)
{
    switch (kind) {
    case PieceKind::BISHOP:
        return filterBishopMoveRule(boardSeats, coord, coords);
    case PieceKind::KNIGHT:
        return filterKnightMoveRule(boardSeats, coord, coords);
    case PieceKind::ROOK:
        return filterRookMoveRule(boardSeats, coords);
    case PieceKind::CANNON:
        return filterCannonMoveRule(boardSeats, coords);
    default:
        return {};
    }
}

QList<Coord> SeatBase::filterColorRule(const BoardSeats* boardSeats, PieceColor color, QList<Coord>& coords)
{
    QList<Coord> colorCoords;

    // 排除目标同色的位置
    QMutableListIterator<Coord> coordIter(coords);
    while (coordIter.hasNext()) {
        Coord& coord = coordIter.next();
        Seat* seat = boardSeats->getSeat(coord);
        if (seat->hasPiece() && seat->piece()->color() == color) {
            colorCoords.append(coord);
            coordIter.remove();
        }
    }

    return colorCoords;
}

QList<Coord> SeatBase::filterBishopMoveRule(const BoardSeats* boardSeats,
    const Coord& coord, QList<Coord>& coords)
{
    int row { coord.first }, col { coord.second };
    QList<Coord> ruleCoords;
    QMutableListIterator<Coord> coordIter(coords);
    while (coordIter.hasNext()) {
        auto& coord = coordIter.next();
        Coord midCoord = {
            (row + coord.first) / 2,
            (col + coord.second) / 2
        };
        if (boardSeats->getSeat(midCoord)->hasPiece()) {
            ruleCoords.append({ midCoord, coord });
            coordIter.remove();
        }
    }

    return ruleCoords;
}

QList<Coord> SeatBase::filterKnightMoveRule(const BoardSeats* boardSeats,
    const Coord& coord, QList<Coord>& coords)
{
    int row { coord.first }, col { coord.second };
    QList<Coord> ruleCoords;
    QMutableListIterator<Coord> coordIter(coords);
    while (coordIter.hasNext()) {
        auto& coord = coordIter.next();
        Coord midCoord = {
            row + (coord.first - row) / 2,
            col + (coord.second - col) / 2
        };
        if (boardSeats->getSeat(midCoord)->hasPiece()) {
            ruleCoords.append({ midCoord, coord });
            coordIter.remove();
        }
    }

    return ruleCoords;
}

QList<Coord> SeatBase::filterRookMoveRule(const BoardSeats* boardSeats, QList<Coord>& coords)
{
    QList<Coord> ruleCoords;
    // 后前左右按顺序排列的四个方向子序列
    int curDirection { 0 };
    QVector<bool> stop(4, false);
    QMutableListIterator<Coord> coordIter(coords);
    while (coordIter.hasNext()) {
        auto& coord = coordIter.next();
        if (coord.first == NONROWCOLVALUE) {
            coordIter.remove();
            curDirection++;
            continue;
        }

        if (stop[curDirection]) {
            ruleCoords.append(coord);
            coordIter.remove();
        } else if (boardSeats->getSeat(coord)->hasPiece())
            stop[curDirection] = true;
    }

    return ruleCoords;
}

QList<Coord> SeatBase::filterCannonMoveRule(const BoardSeats* boardSeats, QList<Coord>& coords)
{
    QList<Coord> ruleCoords;
    // 后前左右按顺序排列的四个方向子序列
    int curDirection { 0 };
    QVector<bool> stop(4, false), skiped(4, false);
    QMutableListIterator<Coord> coordIter(coords);
    while (coordIter.hasNext()) {
        auto& coord = coordIter.next();
        if (coord.first == NONROWCOLVALUE) {
            coordIter.remove(); // 删除设置的哨卡
            curDirection++;
            continue;
        }

        if (stop[curDirection]) {
            ruleCoords.append(coord);
            coordIter.remove();
        } else {
            if (boardSeats->getSeat(coord)->hasPiece()) {
                if (!skiped[curDirection]) { // 炮在未跳状态
                    skiped[curDirection] = true;
                    ruleCoords.append(coord);
                    coordIter.remove();
                } else // 炮在已跳状态
                    stop[curDirection] = true;
            } else if (skiped[curDirection]) {
                ruleCoords.append(coord);
                coordIter.remove();
            }
        }
    }

    return ruleCoords;
}

QList<Coord> SeatBase::kingCanPut(SeatSide homeSide)
{
    QList<Coord> coords;
    int minRow = homeSide == SeatSide::BOTTOM ? MINROWCOL : KINGADVHIGHMINROW,
        maxRow = homeSide == SeatSide::BOTTOM ? KINGADVLOWMAXROW : ROWNUM - 1;
    for (int row = minRow; row <= maxRow; ++row)
        for (int col = KINGADVMINCOL; col <= KINGADVMAXCOL; ++col)
            coords.append({ row, col });

    return coords;
}

QList<Coord> SeatBase::advisorCanPut(SeatSide homeSide)
{
    QList<Coord> coords;
    int minRow = homeSide == SeatSide::BOTTOM ? MINROWCOL : KINGADVHIGHMINROW,
        maxRow = homeSide == SeatSide::BOTTOM ? KINGADVLOWMAXROW : ROWNUM - 1;
    coords.append({ minRow + 1, KINGADVMINCOL + 1 });
    for (int row = minRow; row <= maxRow; row += 2)
        for (int col = KINGADVMINCOL; col <= KINGADVMAXCOL; col += 2)
            coords.append({ row, col });

    return coords;
}

QList<Coord> SeatBase::bishopCanPut(SeatSide homeSide)
{
    QList<Coord> coords;
    int minRow = homeSide == SeatSide::BOTTOM ? MINROWCOL : BISHOPLOWMAXROW + 1,
        maxRow = homeSide == SeatSide::BOTTOM ? BISHOPLOWMAXROW : ROWNUM - 1,
        midRow = homeSide == SeatSide::BOTTOM ? KINGADVLOWMAXROW : KINGADVHIGHMINROW;
    for (int row = minRow; row <= maxRow; row += 4)
        for (int col = MINROWCOL + 2; col < COLNUM; col += 4)
            coords.append({ row, col });
    for (int col = MINROWCOL; col < COLNUM; col += 4)
        coords.append({ midRow, col });

    return coords;
}

QList<Coord> SeatBase::pawnCanPut(SeatSide homeSide)
{
    QList<Coord> coords;
    int minRow = homeSide == SeatSide::BOTTOM ? KINGADVLOWMAXROW + 1 : BISHOPLOWMAXROW + 1,
        maxRow = homeSide == SeatSide::BOTTOM ? BISHOPLOWMAXROW : BISHOPLOWMAXROW + 2;
    for (int row = minRow; row <= maxRow; ++row)
        for (int col = MINROWCOL; col < COLNUM; col += 2)
            coords.append({ row, col });

    minRow = homeSide == SeatSide::BOTTOM ? BISHOPLOWMAXROW + 1 : MINROWCOL;
    maxRow = homeSide == SeatSide::BOTTOM ? ROWNUM - 1 : BISHOPLOWMAXROW;
    for (int row = minRow; row <= maxRow; ++row)
        for (int col = MINROWCOL; col < COLNUM; ++col)
            coords.append({ row, col });

    return coords;
}

QList<Coord> SeatBase::kingCanMove(int row, int col)
{
    QList<Coord> coords {
        { row, col - 1 }, { row, col + 1 },
        { row - 1, col }, { row + 1, col }
    };

    return filterValid(coords, isKingAdvCoord);
}

QList<Coord> SeatBase::advisorCanMove(int row, int col, SeatSide homeSide)
{
    if (col != KINGADVMINCOL + 1)
        return { { (homeSide == SeatSide::BOTTOM ? MINROWCOL : KINGADVHIGHMINROW) + 1,
            KINGADVMINCOL + 1 } };

    return {
        { row - 1, col - 1 }, { row - 1, col + 1 },
        { row + 1, col - 1 }, { row + 1, col + 1 }
    };
}

QList<Coord> SeatBase::bishopCanMove(int row, int col)
{
    QList<Coord> coords {
        { row - 2, col - 2 }, { row - 2, col + 2 },
        { row + 2, col - 2 }, { row + 2, col + 2 }
    };

    return filterValid(coords, isBishopCoord);
}

QList<Coord> SeatBase::knightCanMove(int row, int col)
{
    QList<Coord> coords {
        { row - 2, col - 1 }, { row - 2, col + 1 },
        { row - 1, col - 2 }, { row - 1, col + 2 },
        { row + 1, col - 2 }, { row + 1, col + 2 },
        { row + 2, col - 1 }, { row + 2, col + 1 }
    };

    return filterValid(coords, isValid);
}

QList<Coord> SeatBase::rookCannonCanMove(int fromRow, int fromCol)
{
    QList<Coord> coords;
    for (int row = fromRow - 1; row >= MINROWCOL; --row)
        coords.append({ row, fromCol });

    coords.append({ NONROWCOLVALUE, NONROWCOLVALUE }); // 更换方向时设置的哨卡
    for (int row = fromRow + 1; row < ROWNUM; ++row)
        coords.append({ row, fromCol });

    coords.append({ NONROWCOLVALUE, NONROWCOLVALUE }); // 更换方向时设置的哨卡
    for (int col = fromCol - 1; col >= MINROWCOL; --col)
        coords.append({ fromRow, col });

    coords.append({ NONROWCOLVALUE, NONROWCOLVALUE }); // 更换方向时设置的哨卡
    for (int col = fromCol + 1; col < COLNUM; ++col)
        coords.append({ fromRow, col });

    return coords;
}

QList<Coord> SeatBase::pawnCanMove(int row, int col, SeatSide homeSide)
{
    QList<Coord> coords { { row + (homeSide == SeatSide::BOTTOM ? 1 : -1), col } };

    // 已过河
    if ((row > BISHOPLOWMAXROW) == (homeSide == SeatSide::BOTTOM))
        coords.append({ { row, col - 1 }, { row, col + 1 } });

    return filterValid(coords, isValid);
}

QList<Coord>& SeatBase::filterValid(QList<Coord>& coords, bool (*isFunc)(const Coord&))
{
    QMutableListIterator<Coord> coordIter(coords);
    while (coordIter.hasNext())
        if (!isFunc(coordIter.next()))
            coordIter.remove();

    return coords;
}

bool SeatBase::isKingAdvCoord(const Coord& coord)
{
    return isKingAdvRow(coord.first) && isKingAdvCol(coord.second);
}

bool SeatBase::isBishopCoord(const Coord& coord)
{
    return isBishopRow(coord.first) && isCol(coord.second);
}

bool SeatBase::isValid(const Coord& coord)
{
    return isRow(coord.first) && isCol(coord.second);
}

bool SeatBase::isRow(int row)
{
    return row >= MINROWCOL && row < ROWNUM;
}

bool SeatBase::isCol(int col)
{
    return col >= MINROWCOL && col < COLNUM;
}

bool SeatBase::isKingAdvRow(int row)
{
    return ((row >= MINROWCOL && row <= KINGADVLOWMAXROW)
        || (row >= KINGADVHIGHMINROW && row < ROWNUM));
}

bool SeatBase::isKingAdvCol(int col)
{
    return col >= KINGADVMINCOL && col <= KINGADVMAXCOL;
}

bool SeatBase::isBishopRow(int row)
{
    return BISHOPROWS.contains(row);
}

QList<Coord> getCoordList(const QList<Seat*>& seats)
{
    QList<Coord> coords;
    for (auto& seat : seats)
        coords.append(seat->coord());

    return coords;
}

QString getCoordString(const Coord& coord)
{
    return QString("<%1,%2>").arg(coord.first).arg(coord.second);
}

QString getCoordListString(const QList<Coord>& coords)
{
    QString str;
    int colNum { SeatBase::getColNum() };
    int count = coords.count();
    if (count > 0) {
        for (int i = 0; i < count; ++i) {
            auto coord = coords[i];
            str.append(getCoordString(coord));
            // 每行SEATCOL个数据
            if ((i % colNum == colNum - 1) && i != count - 1)
                str.append("\n");
        }
        str.append(QString("【%1】").arg(count));
    }

    return str;
}

QString getSeatListString(const QList<Seat*>& seats)
{
    return getCoordListString(getCoordList(seats));
}
