#include "seat.h"
#include "piece.h"

Seat::Seat(int row, int col)
    : row_(row)
    , col_(col)
{
}

int Seat::rowcol() const
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
    return QString("<%1,%2 %3>").arg(row_).arg(col_).arg(piece_ ? piece_->name() : '-');
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

PSeat Seats::getChangeSeat(PSeat& seat, ChangeType ct) const
{
    if (ct == ChangeType::SYMMETRY)
        return getSeat(seat->row(), symmetryCol_(seat->col()));
    else if (ct == ChangeType::ROTATE)
        return getSeat(symmetryRow_(seat->row()), symmetryCol_(seat->col()));
    else
        //(ct == ChangeType::NOCHANGE || ct == ChangeType::EXCHANGE)
        return seat;
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
                seat->setPiece(nullptr); // 切断seat与piece间的联系
                changeSeat->setPiece(nullptr); // 切断changeSeat与changePiece间的联系
                seat->setPiece(changePiece);
                changeSeat->setPiece(piece);
            }
    } else if (ct == ChangeType::EXCHANGE) {
        QList<QPair<PSeat, PPiece>> seatPieces;
        for (auto color : Pieces::allColorList)
            for (auto& seat : pieces->getLiveSeatList(color))
                if (seat) {
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
    assert(pieChars.count() == SEATNUM);
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

QList<SeatCoord> Seats::allSeatCoord()
{
    QList<SeatCoord> seatCoordList;
    for (int row = 0; row < SEATROW; ++row)
        for (int col = 0; col < SEATCOL; ++col)
            seatCoordList.append({ row, col });

    return seatCoordList;
}

QList<SeatCoord> Seats::kingSeatCoord(Side homeSide)
{
    QList<SeatCoord> seatCoordList;
    int rlow = homeSide == Side::HERE ? 0 : 7,
        rhigh = homeSide == Side::HERE ? 3 : SEATROW;
    for (int row = rlow; row < rhigh; ++row)
        for (int col = 3; col < 6; ++col)
            seatCoordList.append({ row, col });

    return seatCoordList;
}

QList<SeatCoord> Seats::advisorSeatCoord(Side homeSide)
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

QList<SeatCoord> Seats::bishopSeatCoord(Side homeSide)
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

QList<SeatCoord> Seats::pawnSeatCoord(Side homeSide)
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

QList<SeatCoord> Seats::kingMoveSeatCoord(PSeat seat)
{
    int row = seat->row(), col = seat->col();
    QList<SeatCoord> seatCoordList {
        { row, col - 1 }, { row, col + 1 },
        { row - 1, col }, { row + 1, col }
    };

    return getValidSeatCoord(seatCoordList, isValidKingAdvSeatCoord);
}

QList<SeatCoord> Seats::advisorMoveSeatCoord(PSeat seat, Side homeSide)
{
    int row = seat->row(), col = seat->col();
    if (col != 4)
        return { { homeSide == Side::HERE ? 1 : 8, 4 } };

    return {
        { row - 1, col - 1 }, { row - 1, col + 1 },
        { row + 1, col - 1 }, { row + 1, col + 1 }
    };
}

QList<SeatCoord> Seats::bishopMoveSeatCoord(PSeat seat)
{
    int row = seat->row(), col = seat->col();
    QList<SeatCoord> seatCoordList {
        { row - 2, col - 2 }, { row - 2, col + 2 },
        { row + 2, col - 2 }, { row + 2, col + 2 }
    };

    return Seats::getValidSeatCoord(seatCoordList, Seats::isValidBishopSeatCoord);
}

QList<SeatCoord> Seats::knightMoveSeatCoord(PSeat seat)
{
    int row = seat->row(), col = seat->col();
    QList<SeatCoord> seatCoordList {
        { row - 2, col - 1 }, { row - 2, col + 1 },
        { row - 1, col - 2 }, { row - 1, col + 2 },
        { row + 1, col - 2 }, { row + 1, col + 2 },
        { row + 2, col - 1 }, { row + 2, col + 1 }
    };

    return Seats::getValidSeatCoord(seatCoordList, Seats::isValidSeatCoord);
}

QList<SeatCoord> Seats::rookCannonMoveSeatCoord(PSeat seat)
{
    int row = seat->row(), col = seat->col();
    QList<SeatCoord> seatCoordList;
    // 先行后列，先小后大。顺序固定
    for (int r = row - 1; r >= 0; --r)
        seatCoordList.append({ r, col });
    for (int r = row + 1; r < SEATROW; ++r)
        seatCoordList.append({ r, col });
    for (int c = col - 1; c >= 0; --c)
        seatCoordList.append({ row, c });
    for (int c = col + 1; c < SEATCOL; ++c)
        seatCoordList.append({ row, c });

    return seatCoordList;
}

QList<SeatCoord> Seats::pawnMoveSeatCoord(PSeat seat, Side homeSide)
{
    int row = seat->row(), col = seat->col();
    QList<SeatCoord> seatCoordList { { row + (homeSide == Side::HERE ? 1 : -1), col } };
    // 已过河
    if ((row >= SEATROW / 2) == (homeSide == Side::HERE))
        seatCoordList.append({ { row, col - 1 }, { row, col + 1 } });

    return Seats::getValidSeatCoord(seatCoordList, Seats::isValidSeatCoord);
}

QList<SeatCoord> Seats::bishopFilterRuleSeatCoord(PSeat seat, QList<SeatCoord>& seatCoordList) const
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

QList<SeatCoord> Seats::knightFilterRuleSeatCoord(PSeat seat, QList<SeatCoord>& seatCoordList) const
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

QList<SeatCoord> Seats::rookFilterRuleSeatCoord(PSeat seat, QList<SeatCoord>& seatCoordList) const
{
    QList<SeatCoord> ruleSeatCoords;
    // 后前左右按顺序排列的四个方向子序列 [..-row..|..+row..|..-col..|..+col..]
    int curDirection { 0 };
    QVector<bool> stop(4, false);
    int row = seat->row(), col = seat->col();
    QMutableListIterator<SeatCoord> seatCoordIter(seatCoordList);
    while (seatCoordIter.hasNext()) {
        auto& seatCoord = seatCoordIter.next();
        int toRow { seatCoord.first }, toCol { seatCoord.second };
        curDirection = (col == toCol
                ? (toRow < row ? 0 : 1)
                : (toCol < col ? 2 : 3));

        if (stop[curDirection]) {
            ruleSeatCoords.append(seatCoord);
            seatCoordIter.remove();
        } else if (getSeat(seatCoord)->getPiece())
            stop[curDirection] = true;
    }

    return ruleSeatCoords;
}

QList<SeatCoord> Seats::cannonFilterRuleSeatCoord(PSeat seat, QList<SeatCoord>& seatCoordList) const
{
    QList<SeatCoord> ruleSeatCoords;
    // 后前左右按顺序排列的四个方向子序列 [..-row..|..+row..|..-col..|..+col..]
    int curDirection { 0 };
    QVector<bool> stop(4, false), skiped(4, false);
    int row = seat->row(), col = seat->col();
    QMutableListIterator<SeatCoord> seatCoordIter(seatCoordList);
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

QList<SeatCoord>& Seats::getValidSeatCoord(QList<SeatCoord>& seatCoordList,
    bool (*isValidFunc)(SeatCoord))
{
    QMutableListIterator<SeatCoord> seatCoordIter(seatCoordList);
    while (seatCoordIter.hasNext())
        if (!isValidFunc(seatCoordIter.next()))
            seatCoordIter.remove();

    return seatCoordList;
}

bool Seats::isValidSeatCoord(SeatCoord seatCoord)
{
    return isValidRow_(seatCoord.first) && isValidCol_(seatCoord.second);
}

bool Seats::isValidKingAdvSeatCoord(SeatCoord seatCoord)
{
    return isValidKingAdvRow_(seatCoord.first) && isValidKingAdvCol_(seatCoord.second);
}

bool Seats::isValidBishopSeatCoord(SeatCoord seatCoord)
{
    return isValidBishopRow_(seatCoord.first) && isValidCol_(seatCoord.second);
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