#include "seat.h"
#include "piece.h"

Seat::Seat(int row, int col)
    : row_(row)
    , col_(col)
    , piece_(nullptr)
{
}

QString Seat::rowcol() const
{
    return Seats::rowcol(row_, col_);
}

void Seat::setPiece(PPiece piece)
{
    if (piece_)
        piece_->setSeat(nullptr);

    if (piece)
        piece->setSeat(this);

    piece_ = piece;
}

PPiece Seat::moveTo(PSeat toSeat, PPiece fillPiece)
{
    PPiece piece = getPiece(),
           toPiece = toSeat->getPiece();

    setPiece(fillPiece); // 清空this与piece的联系
    toSeat->setPiece(piece); // 清空toSeat与toPiece的联系

    return toPiece;
}

QString Seat::toString() const
{
    return QString("<%1,%2 %3>").arg(row_).arg(col_).arg(piece_ ? piece_->name() : '*');
}

Seats::Seats()
{
    for (int row = 0; row < SEATROW; ++row)
        for (int col = 0; col < SEATCOL; ++col)
            seats_[row][col] = new Seat(row, col);
}

Seats::~Seats()
{
    for (int row = 0; row < SEATROW; ++row)
        for (int col = 0; col < SEATCOL; ++col)
            delete seats_[row][col];
}

void Seats::clear()
{
    for (int row = 0; row < SEATROW; ++row)
        for (int col = 0; col < SEATCOL; ++col)
            seats_[row][col]->setPiece(nullptr);
}

PSeat Seats::getSeat(int row, int col) const
{
    return seats_[row][col];
}

PSeat Seats::getSeat(SeatCoord seatCoord) const
{
    return getSeat(seatCoord.first, seatCoord.second);
}

SeatCoord Seats::getChangeSeatCoord(SeatCoord seatCoord, ChangeType ct)
{
    if (ct == ChangeType::SYMMETRY)
        return { seatCoord.first, symmetryCol_(seatCoord.second) };
    else if (ct == ChangeType::ROTATE)
        return { symmetryRow_(seatCoord.first), symmetryCol_(seatCoord.second) };
    else
        //(ct == ChangeType::NOCHANGE || ct == ChangeType::EXCHANGE)
        return seatCoord;
}

PSeat Seats::getChangeSeat(PSeat& seat, ChangeType ct) const
{
    return getSeat(getChangeSeatCoord(seat->seatCoord(), ct));
}

void Seats::changeLayout(const Pieces* pieces, ChangeType ct)
{
    if (ct == ChangeType::SYMMETRY || ct == ChangeType::ROTATE) {
        int maxRow = ct == ChangeType::SYMMETRY ? SEATROW : SEATROW / 2,
            maxCol = ct == ChangeType::SYMMETRY ? SEATCOL / 2 : SEATCOL;
        for (int row = 0; row < maxRow; ++row)
            for (int col = 0; col < maxCol; ++col) {
                PSeat seat = getSeat(row, col),
                      changeSeat = getChangeSeat(seat, ct);
                PPiece piece = seat->getPiece(),
                       changePiece = changeSeat->getPiece();
                if (!piece && !changePiece) // 两个位置都无棋子
                    continue;

                changeSeat->setPiece(nullptr); // 切断changeSeat与changePiece间的联系！
                seat->setPiece(changePiece); // 建立seat与changePiece间的联系，同时切断seat与piece间的联系
                changeSeat->setPiece(piece); // 建立changeSeat与piece间的联系
            }
    } else if (ct == ChangeType::EXCHANGE) {
        QList<QPair<PSeat, PPiece>> seatPieces;
        for (auto color : Pieces::allColorList)
            for (auto& seat : pieces->getLiveSeatList(color)) {
                seatPieces.append({ seat, seat->getPiece() });
                seat->setPiece(nullptr);
            }

        for (auto& seatPiece : seatPieces)
            seatPiece.first->setPiece(pieces->getOtherPiece(seatPiece.second));
    }
}

QString Seats::getFEN() const
{
    QString pieChars;
    for (int row = 0; row < SEATROW; ++row)
        for (int col = 0; col < SEATCOL; ++col) {
            auto piece = getSeat(row, col)->getPiece();
            pieChars.append(piece ? piece->ch() : Pieces::nullChar);
        }

    return pieCharsToFEN(pieChars);
}

bool Seats::setFEN(const Pieces* pieces, const QString& fen)
{
    QString pieChars { FENToPieChars(fen) };
    if (pieChars.isEmpty())
        return false;

    clear();
    int index = 0;
    for (int row = 0; row < SEATROW; ++row)
        for (int col = 0; col < SEATCOL; ++col) {
            QChar ch = pieChars[index++];
            getSeat(row, col)->setPiece(pieces->getNotLivePiece(ch));
        }

    return true;
}

QString Seats::toString() const
{
    // 文本空棋盘
    QString textBlankBoard {
        "┏━┯━┯━┯━┯━┯━┯━┯━┓\n"
        "┃　│　│　│╲│╱│　│　│　┃\n"
        "┠─┼─┼─┼─╳─┼─┼─┼─┨\n"
        "┃　│　│　│╱│╲│　│　│　┃\n"
        "┠─╬─┼─┼─┼─┼─┼─╬─┨\n"
        "┃　│　│　│　│　│　│　│　┃\n"
        "┠─┼─╬─┼─╬─┼─╬─┼─┨\n"
        "┃　│　│　│　│　│　│　│　┃\n"
        "┠─┴─┴─┴─┴─┴─┴─┴─┨\n"
        "┃　　　　　　　　　　　　　　　┃\n"
        "┠─┬─┬─┬─┬─┬─┬─┬─┨\n"
        "┃　│　│　│　│　│　│　│　┃\n"
        "┠─┼─╬─┼─╬─┼─╬─┼─┨\n"
        "┃　│　│　│　│　│　│　│　┃\n"
        "┠─╬─┼─┼─┼─┼─┼─╬─┨\n"
        "┃　│　│　│╲│╱│　│　│　┃\n"
        "┠─┼─┼─┼─╳─┼─┼─┼─┨\n"
        "┃　│　│　│╱│╲│　│　│　┃\n"
        "┗━┷━┷━┷━┷━┷━┷━┷━┛\n"
    }; // 边框粗线

    for (int row = 0; row < SEATROW; ++row)
        for (int col = 0; col < SEATCOL; ++col) {
            auto seat = getSeat(row, col);
            if (!seat->getPiece())
                continue;

            int index = (SEATROW - 1 - seat->row()) * 2 * (SEATCOL * 2) + seat->col() * 2;
            textBlankBoard[index] = seat->getPiece()->printName();
        }

    return textBlankBoard;
}

QString Seats::pieCharsToFEN(const QString& pieChars)
{
    QString fen {};
    if (pieChars.count() != SEATNUM)
        return fen;

    for (int index = 0; index < SEATNUM; index += SEATCOL) {
        QString line { pieChars.mid(index, SEATCOL) }, qstr {};
        int num { 0 };
        for (auto ch : line) {
            if (ch != Pieces::nullChar) {
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
        fen.prepend(qstr).prepend(Pieces::FENSplitChar);
    }
    fen.remove(0, 1);

    return fen;
}

QString Seats::FENToPieChars(const QString& fen)
{
    QStringList strList { fen.split(Pieces::FENSplitChar) };
    if (strList.count() != SEATROW)
        return QString();

    QString pieceChars {};
    for (auto& line : strList) {
        QString qstr {};
        for (auto ch : line)
            qstr.append(ch.isDigit() ? QString(ch.digitValue(), Pieces::nullChar) : ch);
        pieceChars.prepend(qstr);
    }

    return pieceChars.size() == SEATNUM ? pieceChars : QString();
}

QString Seats::rowcol(int row, int col)
{
    return QString("%1%2").arg(row).arg(col);
}

QString Seats::rowcols(const QString& frowcol, const QString& trowcol)
{
    return frowcol + trowcol;
}

QPair<SeatCoord, SeatCoord> Seats::seatCoordPair(const QString& rowcols)
{
    return { { rowcols.at(0).digitValue(), rowcols.at(1).digitValue() },
        { rowcols.at(2).digitValue(), rowcols.at(3).digitValue() } };
}

bool Seats::less(PSeat first, PSeat last)
{
    return (first->row() < last->row()
        || (first->row() == last->row() && first->col() < last->col()));
}

bool Seats::isBottom(PSeat seat)
{
    return seat->row() < SEATROW / 2;
}

QList<SeatCoord> Seats::getSeatCoordList(const QList<PSeat>& seatList)
{
    QList<SeatCoord> seatCoordList;
    for (auto& seat : seatList)
        seatCoordList.append(seat->seatCoord());

    return seatCoordList;
}

QList<SeatCoord> Seats::allSeatCoord()
{
    QList<SeatCoord> seatCoordList;
    for (int row = 0; row < SEATROW; ++row)
        for (int col = 0; col < SEATCOL; ++col)
            seatCoordList.append({ row, col });

    return seatCoordList;
}

QList<SeatCoord> Seats::kingPutTo(Side homeSide)
{
    QList<SeatCoord> seatCoordList;
    int rlow = homeSide == Side::HERE ? 0 : 7,
        rhigh = homeSide == Side::HERE ? 3 : SEATROW;
    for (int row = rlow; row < rhigh; ++row)
        for (int col = 3; col < 6; ++col)
            seatCoordList.append({ row, col });

    return seatCoordList;
}

QList<SeatCoord> Seats::advisorPutTo(Side homeSide)
{
    QList<SeatCoord> seatCoordList;
    int rlow = homeSide == Side::HERE ? 0 : 7,
        rhigh = homeSide == Side::HERE ? 3 : SEATROW;
    seatCoordList.append({ rlow + 1, 4 });
    for (int row = rlow; row < rhigh; row += 2)
        for (int col = 3; col < 6; col += 2)
            seatCoordList.append({ row, col });

    return seatCoordList;
}

QList<SeatCoord> Seats::bishopPutTo(Side homeSide)
{
    QList<SeatCoord> seatCoordList;
    int rlow = homeSide == Side::HERE ? 0 : 5,
        rhigh = homeSide == Side::HERE ? 5 : SEATROW;
    for (int row = rlow; row < rhigh; row += 4)
        for (int col = 2; col < SEATCOL; col += 4)
            seatCoordList.append({ row, col });
    for (int col = 0; col < SEATCOL; col += 4)
        seatCoordList.append({ rlow + 2, col });

    return seatCoordList;
}

QList<SeatCoord> Seats::pawnPutTo(Side homeSide)
{
    QList<SeatCoord> seatCoordList;
    int rlow = homeSide == Side::HERE ? 3 : 5,
        rhigh = homeSide == Side::HERE ? 5 : 7;
    for (int row = rlow; row < rhigh; ++row)
        for (int col = 0; col < SEATCOL; col += 2)
            seatCoordList.append({ row, col });

    rlow = homeSide == Side::HERE ? 5 : 0;
    rhigh = homeSide == Side::HERE ? SEATROW : 5;
    for (int row = rlow; row < rhigh; ++row)
        for (int col = 0; col < SEATCOL; ++col)
            seatCoordList.append({ row, col });

    return seatCoordList;
}

QList<SeatCoord> Seats::kingMoveTo(PSeat seat)
{
    int row = seat->row(), col = seat->col();
    QList<SeatCoord> seatCoordList {
        { row, col - 1 }, { row, col + 1 },
        { row - 1, col }, { row + 1, col }
    };

    return getValidSeatCoord_(seatCoordList, isValidKingAdvSeatCoord_);
}

QList<SeatCoord> Seats::advisorMoveTo(PSeat seat, Side homeSide)
{
    int row = seat->row(), col = seat->col();
    if (col != 4)
        return { { homeSide == Side::HERE ? 1 : 8, 4 } };

    return {
        { row - 1, col - 1 }, { row - 1, col + 1 },
        { row + 1, col - 1 }, { row + 1, col + 1 }
    };
}

QList<SeatCoord> Seats::bishopMoveTo(PSeat seat)
{
    int row = seat->row(), col = seat->col();
    QList<SeatCoord> seatCoordList {
        { row - 2, col - 2 }, { row - 2, col + 2 },
        { row + 2, col - 2 }, { row + 2, col + 2 }
    };

    return getValidSeatCoord_(seatCoordList, isValidBishopSeatCoord_);
}

QList<SeatCoord> Seats::knightMoveTo(PSeat seat)
{
    int row = seat->row(), col = seat->col();
    QList<SeatCoord> seatCoordList {
        { row - 2, col - 1 }, { row - 2, col + 1 },
        { row - 1, col - 2 }, { row - 1, col + 2 },
        { row + 1, col - 2 }, { row + 1, col + 2 },
        { row + 2, col - 1 }, { row + 2, col + 1 }
    };

    return getValidSeatCoord_(seatCoordList, isValidSeatCoord_);
}

QList<SeatCoord> Seats::rookCannonMoveTo(PSeat seat)
{
    int row = seat->row(), col = seat->col();
    QList<SeatCoord> seatCoordList;
    for (int r = row - 1; r >= 0; --r)
        seatCoordList.append({ r, col });

    seatCoordList.append({ -1, -1 }); // 更换方向时设置的哨卡
    for (int r = row + 1; r < SEATROW; ++r)
        seatCoordList.append({ r, col });

    seatCoordList.append({ -1, -1 }); // 更换方向时设置的哨卡
    for (int c = col - 1; c >= 0; --c)
        seatCoordList.append({ row, c });

    seatCoordList.append({ -1, -1 }); // 更换方向时设置的哨卡
    for (int c = col + 1; c < SEATCOL; ++c)
        seatCoordList.append({ row, c });

    return seatCoordList;
}

QList<SeatCoord> Seats::pawnMoveTo(PSeat seat, Side homeSide)
{
    int row = seat->row(), col = seat->col();
    QList<SeatCoord> seatCoordList { { row + (homeSide == Side::HERE ? 1 : -1), col } };

    // 已过河
    if ((row >= SEATROW / 2) == (homeSide == Side::HERE))
        seatCoordList.append({ { row, col - 1 }, { row, col + 1 } });

    return getValidSeatCoord_(seatCoordList, isValidSeatCoord_);
}

QList<SeatCoord> Seats::bishopRuleFilter(PSeat seat, QList<SeatCoord>& seatCoordList) const
{
    QList<SeatCoord> ruleSeatCoords;
    int row = seat->row(), col = seat->col();
    QMutableListIterator<SeatCoord> seatCoordIter(seatCoordList);
    while (seatCoordIter.hasNext()) {
        auto& seatCoord = seatCoordIter.next();
        SeatCoord midSeatCoord = {
            (row + seatCoord.first) / 2,
            (col + seatCoord.second) / 2
        };
        if (getSeat(midSeatCoord)->getPiece()) {
            ruleSeatCoords.append({ midSeatCoord, seatCoord });
            seatCoordIter.remove();
        }
    }

    return ruleSeatCoords;
}

QList<SeatCoord> Seats::knightRuleFilter(PSeat seat, QList<SeatCoord>& seatCoordList) const
{
    QList<SeatCoord> ruleSeatCoords;
    int row = seat->row(), col = seat->col();
    QMutableListIterator<SeatCoord> seatCoordIter(seatCoordList);
    while (seatCoordIter.hasNext()) {
        auto& seatCoord = seatCoordIter.next();
        SeatCoord midSeatCoord = {
            row + (seatCoord.first - row) / 2,
            col + (seatCoord.second - col) / 2
        };
        if (getSeat(midSeatCoord)->getPiece()) {
            ruleSeatCoords.append({ midSeatCoord, seatCoord });
            seatCoordIter.remove();
        }
    }

    return ruleSeatCoords;
}

QList<SeatCoord> Seats::rookRuleFilter(QList<SeatCoord>& seatCoordList) const
{
    QList<SeatCoord> ruleSeatCoords;
    // 后前左右按顺序排列的四个方向子序列
    int curDirection { 0 };
    QVector<bool> stop(4, false);
    QMutableListIterator<SeatCoord> seatCoordIter(seatCoordList);
    while (seatCoordIter.hasNext()) {
        auto& seatCoord = seatCoordIter.next();
        if (!isValidSeatCoord_(seatCoord)) {
            seatCoordIter.remove();
            curDirection++;
            continue;
        }

        if (stop[curDirection]) {
            ruleSeatCoords.append(seatCoord);
            seatCoordIter.remove();
        } else if (getSeat(seatCoord)->getPiece())
            stop[curDirection] = true;
    }

    return ruleSeatCoords;
}

QList<SeatCoord> Seats::cannonRuleFilter(QList<SeatCoord>& seatCoordList) const
{
    QList<SeatCoord> ruleSeatCoords;
    // 后前左右按顺序排列的四个方向子序列
    int curDirection { 0 };
    QVector<bool> stop(4, false), skiped(4, false);
    QMutableListIterator<SeatCoord> seatCoordIter(seatCoordList);
    while (seatCoordIter.hasNext()) {
        auto& seatCoord = seatCoordIter.next();
        if (!isValidSeatCoord_(seatCoord)) {
            seatCoordIter.remove(); // 删除设置的哨卡
            curDirection++;
            continue;
        }

        if (stop[curDirection]) {
            ruleSeatCoords.append(seatCoord);
            seatCoordIter.remove();
        } else {
            if (getSeat(seatCoord)->getPiece()) {
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

QList<SeatCoord>& Seats::getValidSeatCoord_(QList<SeatCoord>& seatCoordList,
    bool (*isValidFunc)(SeatCoord))
{
    QMutableListIterator<SeatCoord> seatCoordIter(seatCoordList);
    while (seatCoordIter.hasNext())
        if (!isValidFunc(seatCoordIter.next()))
            seatCoordIter.remove();

    return seatCoordList;
}

bool Seats::isValidSeatCoord_(SeatCoord seatCoord)
{
    return isValidRow_(seatCoord.first) && isValidCol_(seatCoord.second);
}

bool Seats::isValidKingAdvSeatCoord_(SeatCoord seatCoord)
{
    return isValidKingAdvRow_(seatCoord.first) && isValidKingAdvCol_(seatCoord.second);
}

bool Seats::isValidBishopSeatCoord_(SeatCoord seatCoord)
{
    return isValidBishopRow_(seatCoord.first) && isValidCol_(seatCoord.second);
}

bool Seats::isValidRow_(int row)
{
    return row >= 0 && row < SEATROW;
}

bool Seats::isValidCol_(int col)
{
    return col >= 0 && col < SEATCOL;
}

bool Seats::isValidKingAdvRow_(int row)
{
    return (row >= 0 && row < 3) || (row >= 7 && row < SEATROW);
}

bool Seats::isValidKingAdvCol_(int col)
{
    return col >= 3 && col < 6;
}

bool Seats::isValidBishopRow_(int row)
{
    return QList<int>({ 0, 2, 4, 5, 7, 9 }).contains(row);
}

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
    return printSeatCoordList(Seats::getSeatCoordList(seatList));
}
