#include "piece.h"
#include "seat.h"

Piece::Piece(Color color, Kind kind)
    : color_(color)
    , kind_(kind)
    , seat_(nullptr)
{
}

PPiece Piece::setSeat(PSeat seat)
{
    seat_ = seat;
    return this;
}

QList<QList<SeatCoord>> Piece::canMoveSeatCoord(const Seats* seats, Side homeSide) const
{
    QList<SeatCoord> seatCoords { moveSeatCoord(homeSide) };
    // 排除不符合走棋规则的位置
    QList<SeatCoord> ruleSeatCoords = ruleFilterSeatCoord(seats, seatCoords);
    QList<SeatCoord> colorSeatCoords = colorFilterSeatCoord(seats, seatCoords);

    return { seatCoords, ruleSeatCoords, colorSeatCoords };
}

QList<SeatCoord> Piece::ruleFilterSeatCoord(const Seats* seats, QList<SeatCoord>& seatCoords) const
{
    Q_UNUSED(seats);
    Q_UNUSED(seatCoords);

    return {};
}

QList<SeatCoord> Piece::colorFilterSeatCoord(const Seats* seats, QList<SeatCoord>& seatCoords) const
{
    QList<SeatCoord> colorSeatCoords;
    // 排除目标同色的位置
    QMutableListIterator<SeatCoord> seatCoordIter(seatCoords);
    while (seatCoordIter.hasNext()) {
        auto& seatCoord = seatCoordIter.next();
        auto piece = seats->getSeat(seatCoord)->getPiece();
        if (piece && piece->color() == color_) {
            colorSeatCoords.append(seatCoord);
            seatCoordIter.remove();
        }
    }

    return colorSeatCoords;
}

QList<SeatCoord> Piece::putSeatCoord(Side homeSide) const
{
    Q_UNUSED(homeSide);

    QList<SeatCoord> seatCoords;
    for (int row = 0; row < SEATROW; ++row)
        for (int col = 0; col < SEATCOL; ++col)
            seatCoords.append({ row, col });

    return seatCoords;
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

QList<SeatCoord>& Piece::getValidSeatCoord(QList<SeatCoord>& seatCoords,
    bool (*isValidFunc)(SeatCoord))
{
    QMutableListIterator<SeatCoord> seatCoordIter(seatCoords);
    while (seatCoordIter.hasNext())
        if (!isValidFunc(seatCoordIter.next()))
            seatCoordIter.remove();

    return seatCoords;
}

bool Piece::isValidSeatCoord(SeatCoord seatCoord)
{
    return Seats::isValidRow(seatCoord.first) && Seats::isValidCol(seatCoord.second);
}

bool Piece::isValidKingAdvSeatCoord(SeatCoord seatCoord)
{
    return Seats::isValidKingAdvRow(seatCoord.first) && Seats::isValidKingAdvCol(seatCoord.second);
}

bool Piece::isValidBishopSeatCoord(SeatCoord seatCoord)
{
    return Seats::isValidBishopRow(seatCoord.first) && Seats::isValidCol(seatCoord.second);
}

QList<SeatCoord> Piece::rookCannonMoveSeatCoord() const
{
    int row = getSeat()->row(), col = getSeat()->col();
    QList<SeatCoord> seatCoords;
    // 先行后列，先小后大。顺序固定
    for (int r = row - 1; r >= 0; --r)
        seatCoords.append({ r, col });
    for (int r = row + 1; r < SEATROW; ++r)
        seatCoords.append({ r, col });
    for (int c = col - 1; c >= 0; --c)
        seatCoords.append({ row, c });
    for (int c = col + 1; c < SEATCOL; ++c)
        seatCoords.append({ row, c });

    return seatCoords;
}

QList<SeatCoord> King::putSeatCoord(Side homeSide) const
{
    QList<SeatCoord> seatCoords;
    int rlow = homeSide == Side::HERE ? 0 : 7,
        rhigh = homeSide == Side::HERE ? 3 : SEATROW;
    for (int row = rlow; row < rhigh; ++row)
        for (int col = 3; col < 6; ++col)
            seatCoords.append({ row, col });

    return seatCoords;
}

QList<SeatCoord> King::moveSeatCoord(Side homeSide) const
{
    Q_UNUSED(homeSide);
    PSeat seat = getSeat();
    if (!seat)
        return {};

    int row = seat->row(), col = getSeat()->col();
    QList<SeatCoord> seatCoords {
        { row, col - 1 }, { row, col + 1 },
        { row - 1, col }, { row + 1, col }
    };
    return getValidSeatCoord(seatCoords, isValidKingAdvSeatCoord);
}

QList<SeatCoord> Advisor::putSeatCoord(Side homeSide) const
{
    QList<SeatCoord> seatCoords;
    int rlow = homeSide == Side::HERE ? 0 : 7,
        rhigh = homeSide == Side::HERE ? 3 : SEATROW;
    seatCoords.append({ rlow + 1, 4 });
    for (int row = rlow; row < rhigh; row += 2)
        for (int col = 3; col < 6; col += 2)
            seatCoords.append({ row, col });

    return seatCoords;
}

QList<SeatCoord> Advisor::moveSeatCoord(Side homeSide) const
{
    PSeat seat = getSeat();
    if (!seat)
        return {};

    int row = getSeat()->row(), col = getSeat()->col();
    //    if (col != 4)
    //        return { { homeSide == Side::HERE ? 1 : 8, 4 } };

    //    return {
    //        { row - 1, col - 1 }, { row - 1, col + 1 },
    //        { row + 1, col - 1 }, { row + 1, col + 1 }
    //    };
    Q_UNUSED(homeSide);

    QList<SeatCoord> seatCoords {
        { row - 1, col - 1 }, { row - 1, col + 1 },
        { row + 1, col - 1 }, { row + 1, col + 1 }
    };
    return getValidSeatCoord(seatCoords, isValidKingAdvSeatCoord);
}

QList<SeatCoord> Bishop::putSeatCoord(Side homeSide) const
{
    QList<SeatCoord> seatCoords;
    int rlow = homeSide == Side::HERE ? 0 : 5,
        rhigh = homeSide == Side::HERE ? 5 : SEATROW;
    for (int row = rlow; row < rhigh; row += 4)
        for (int col = 2; col < SEATCOL; col += 4)
            seatCoords.append({ row, col });
    for (int col = 0; col < SEATCOL; col += 4)
        seatCoords.append({ rlow + 2, col });

    return seatCoords;
}

QList<SeatCoord> Bishop::moveSeatCoord(Side homeSide) const
{
    PSeat seat = getSeat();
    if (!seat)
        return {};

    Q_UNUSED(homeSide);

    int row = getSeat()->row(), col = getSeat()->col();
    QList<SeatCoord> seatCoords {
        { row - 2, col - 2 }, { row - 2, col + 2 },
        { row + 2, col - 2 }, { row + 2, col + 2 }
    };
    return getValidSeatCoord(seatCoords, isValidBishopSeatCoord);
}

QList<SeatCoord> Bishop::ruleFilterSeatCoord(const Seats* seats, QList<SeatCoord>& seatCoords) const
{
    QList<SeatCoord> ruleSeatCoords;
    int row = getSeat()->row(), col = getSeat()->col();
    QMutableListIterator<SeatCoord> seatCoordIter(seatCoords);
    while (seatCoordIter.hasNext()) {
        auto& seatCoord = seatCoordIter.next();
        SeatCoord midSeatCoord = {
            (row + seatCoord.first) / 2,
            (col + seatCoord.second) / 2
        };
        if (seats->getSeat(midSeatCoord)->getPiece()) {
            ruleSeatCoords.append({ midSeatCoord, seatCoord });
            seatCoordIter.remove();
        }
    }

    return ruleSeatCoords;
}

QList<SeatCoord> Knight::moveSeatCoord(Side homeSide) const
{
    Q_UNUSED(homeSide);
    PSeat seat = getSeat();
    if (!seat)
        return {};

    int row = getSeat()->row(), col = getSeat()->col();
    QList<SeatCoord> seatCoords {
        { row - 2, col - 1 }, { row - 2, col + 1 },
        { row - 1, col - 2 }, { row - 1, col + 2 },
        { row + 1, col - 2 }, { row + 1, col + 2 },
        { row + 2, col - 1 }, { row + 2, col + 1 }
    };
    return getValidSeatCoord(seatCoords, isValidSeatCoord);
}

QList<SeatCoord> Knight::ruleFilterSeatCoord(const Seats* seats, QList<SeatCoord>& seatCoords) const
{
    QList<SeatCoord> ruleSeatCoords;
    int row = getSeat()->row(), col = getSeat()->col();
    QMutableListIterator<SeatCoord> seatCoordIter(seatCoords);
    while (seatCoordIter.hasNext()) {
        auto& seatCoord = seatCoordIter.next();
        SeatCoord midSeatCoord = {
            row + (seatCoord.first - row) / 2,
            col + (seatCoord.second - col) / 2
        };
        if (seats->getSeat(midSeatCoord)->getPiece()) {
            ruleSeatCoords.append({ midSeatCoord, seatCoord });
            seatCoordIter.remove();
        }
    }

    return ruleSeatCoords;
}

QList<SeatCoord> Rook::moveSeatCoord(Side homeSide) const
{
    Q_UNUSED(homeSide);
    PSeat seat = getSeat();
    if (!seat)
        return {};

    return rookCannonMoveSeatCoord();
}

QList<SeatCoord> Rook::ruleFilterSeatCoord(const Seats* seats, QList<SeatCoord>& seatCoords) const
{
    QList<SeatCoord> ruleSeatCoords;
    // 后前左右按顺序排列的四个方向子序列 [..-row..|..+row..|..-col..|..+col..]
    int curDirection { 0 };
    QVector<bool> stop(4, false);
    int row = getSeat()->row(), col = getSeat()->col();
    QMutableListIterator<SeatCoord> seatCoordIter(seatCoords);
    while (seatCoordIter.hasNext()) {
        auto& seatCoord = seatCoordIter.next();
        int toRow { seatCoord.first }, toCol { seatCoord.second };
        curDirection = (col == toCol
                ? (toRow < row ? 0 : 1)
                : (toCol < col ? 2 : 3));

        if (stop[curDirection]) {
            ruleSeatCoords.append(seatCoord);
            seatCoordIter.remove();
        } else if (seats->getSeat(seatCoord)->getPiece())
            stop[curDirection] = true;
    }

    return ruleSeatCoords;
}

QList<SeatCoord> Cannon::moveSeatCoord(Side homeSide) const
{
    Q_UNUSED(homeSide);
    PSeat seat = getSeat();
    if (!seat)
        return {};

    return rookCannonMoveSeatCoord();
}

QList<SeatCoord> Cannon::ruleFilterSeatCoord(const Seats* seats, QList<SeatCoord>& seatCoords) const
{
    QList<SeatCoord> ruleSeatCoords;
    // 后前左右按顺序排列的四个方向子序列 [..-row..|..+row..|..-col..|..+col..]
    int curDirection { 0 };
    QVector<bool> stop(4, false), skiped(4, false);
    int row = getSeat()->row(), col = getSeat()->col();
    QMutableListIterator<SeatCoord> seatCoordIter(seatCoords);
    while (seatCoordIter.hasNext()) {
        auto& seatCoord = seatCoordIter.next();
        int toRow { seatCoord.first }, toCol { seatCoord.second };
        curDirection = (col == toCol
                ? (toRow < row ? 0 : 1)
                : (toCol < col ? 2 : 3));

        if (stop[curDirection]) {
            ruleSeatCoords.append(seatCoord);
            seatCoordIter.remove();
        } else {
            if (seats->getSeat(seatCoord)->getPiece()) {
                if (!skiped[curDirection]) { // 炮在未跳状态
                    skiped[curDirection] = true;
                    ruleSeatCoords.append(seatCoord);
                    seatCoordIter.remove();
                } else // 炮在已跳状态
                    stop[curDirection] = true;
            } else if (skiped[curDirection]) {
                ruleSeatCoords.append(seatCoord);
                seatCoordIter.remove();
            }
        }
    }

    return ruleSeatCoords;
}

QList<SeatCoord> Pawn::putSeatCoord(Side homeSide) const
{
    QList<SeatCoord> seatCoords;
    int rlow = homeSide == Side::HERE ? 3 : 5,
        rhigh = homeSide == Side::HERE ? 5 : 7;
    for (int row = rlow; row < rhigh; ++row)
        for (int col = 0; col < SEATCOL; col += 2)
            seatCoords.append({ row, col });

    rlow = homeSide == Side::HERE ? 5 : 0;
    rhigh = homeSide == Side::HERE ? SEATROW : 5;
    for (int row = rlow; row < rhigh; ++row)
        for (int col = 0; col < SEATCOL; ++col)
            seatCoords.append({ row, col });

    return seatCoords;
}

QList<SeatCoord> Pawn::moveSeatCoord(Side homeSide) const
{
    PSeat seat = getSeat();
    if (!seat)
        return {};

    int row = getSeat()->row(), col = getSeat()->col();
    QList<SeatCoord> seatCoords { { row + (homeSide == Side::HERE ? 1 : -1), col } };
    // 已过河
    if ((row >= SEATROW / 2) == (homeSide == Side::HERE))
        seatCoords.append({ { row, col - 1 }, { row, col + 1 } });

    return getValidSeatCoord(seatCoords, isValidSeatCoord);
}

Pieces::Pieces()
{
    for (Color color : allColorList)
        for (Kind kind : allKindList) {
            auto& pieceList = pieces_[int(color)][int(kind)];
            switch (kind) {
            case Kind::KING:
                pieceList.append(new King(color, kind));
                break;
            case Kind::ADVISOR:
                for (int i = 0; i < 2; ++i)
                    pieceList.append(new Advisor(color, kind));
                break;
            case Kind::BISHOP:
                for (int i = 0; i < 2; ++i)
                    pieceList.append(new Bishop(color, kind));
                break;
            case Kind::KNIGHT:
                for (int i = 0; i < 2; ++i)
                    pieceList.append(new Knight(color, kind));
                break;
            case Kind::ROOK:
                for (int i = 0; i < 2; ++i)
                    pieceList.append(new Rook(color, kind));
                break;
            case Kind::CANNON:
                for (int i = 0; i < 2; ++i)
                    pieceList.append(new Cannon(color, kind));
                break;
            default: // Kind::PAWN
                for (int i = 0; i < 5; ++i)
                    pieceList.append(new Pawn(color, kind));
                break;
            }
        }
}

Pieces::~Pieces()
{
    for (Color color : allColorList)
        for (Kind kind : allKindList)
            for (auto& piece : pieces_[int(color)][int(kind)])
                delete piece;
}

PPiece Pieces::getNotLivePiece(Color color, Kind kind) const
{
    for (auto& piece : getColorKindPiece(color, kind)) {
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

// QList<PPiece> Pieces::getColorPiece(Color color, bool stronge) const
//{
//     QList<PPiece> pieceList;
//     for (Kind kind : stronge ? strongeKindList : allKindList)
//         pieceList.append(pieces_[int(color)][int(kind)]);

//    return pieceList;
//}

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

// QList<PSeat> Pieces::getLiveSeatList(Color color, bool stronge) const
//{
//     return getLiveSeatList_(getColorPiece(color, stronge));
// }

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

QString printSeatCoord(const SeatCoord& seatCoord)
{
    return QString("<%1,%2>").arg(seatCoord.first).arg(seatCoord.second);
}

QString printSeatCoordList(const QList<SeatCoord>& seatCoordList)
{
    QString qstr {};
    int count = seatCoordList.count();
    if (count > 0) {
        for (int i = 0; i < count; ++i) {
            auto seatCoord = seatCoordList[i];
            qstr.append(printSeatCoord(seatCoord));
            // 每行SEATCOL个数据
            if ((i % SEATCOL == SEATCOL - 1) && i != count - 1)
                qstr.append("\n");
        }
        qstr.append(QString("【%1】").arg(count));
    }

    return qstr;
}

QString printSeatList(const QList<PSeat>& seatList)
{
    QString qstr {};
    int count = seatList.count();
    if (count > 0) {
        for (int i = 0; i < count; ++i) {
            qstr.append(seatList[i]->toString());
            // 每行SEATCOL个数据
            if ((i % SEATCOL == SEATCOL - 1) && i != count - 1)
                qstr.append("\n");
        }
        qstr.append(QString("【%1】").arg(count));
    }

    return qstr;
}
