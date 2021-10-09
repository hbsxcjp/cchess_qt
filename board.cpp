#include "board.h"
#include "tools.h"

PPiece Board::movePiece(Seat fromSeat, Seat toSeat)
{
    auto toPiece = getPiece_(toSeat);
    setPiece_(toSeat, getPiece_(fromSeat));
    setPiece_(fromSeat);

    return toPiece;
}

QList<Seat> Board::allCanMoveSeat(Piece::Color color) const
{
    QList<Seat> seatList;
    auto homeSide = getSeatside_(color);
    auto seatPieceList = getSeatPieceList_(color);
    for (int i = 0; i < seatPieceList.count(); ++i) {
        auto& seatPiece = seatPieceList.at(i);
        auto moveSeatList = seatPiece.second->move(seatPiece.first, homeSide);
        //筛选排除？

        seatList.append(moveSeatList);
    }
    return seatList;
}

QString Board::getFEN() const
{
    return pieCharsToFEN_(getPieChars_(getSeatPieceList_()));
}

bool Board::setFEN(const QString& fen)
{
    return setFromSeatPieceList_(getSeatPieceList_pieChars_(FENTopieChars_(fen)));
}

const QString Board::toString(bool full) const
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

    // 棋盘上下边标识字符串
    const QString PRESTR[] = {
        "　　　　　　　黑　方　　　　　　　\n１　２　３　４　５　６　７　８　９\n",
        "　　　　　　　红　方　　　　　　　\n一　二　三　四　五　六　七　八　九\n"
    };
    const QString SUFSTR[] = {
        "九　八　七　六　五　四　三　二　一\n　　　　　　　红　方　　　　　　　\n",
        "９　８　７　６　５　４　３　２　１\n　　　　　　　黑　方　　　　　　　\n"
    };

    auto seatPieceList = getSeatPieceList_();
    for (int i = 0; i < seatPieceList.count(); ++i) {
        auto seatPiece = seatPieceList.at(i);
        if (!seatPiece.second)
            continue;

        auto seat = seatPiece.first;
        textBlankBoard[(SEATCOL - seat.first) * 2 * (SEATCOL * 2) + seat.second * 2] = seatPiece.second->printName();
    }

    //    for (auto color : { Piece::Color::BLACK, Piece::Color::RED })
    //        for (auto& seat : getLiveSeats__(color))
    //            textBlankBoard[(SeatManager::ColNum() - seat->row())
    //                    * 2 * (SeatManager::ColNum() * 2)
    //                + seat->col() * 2]
    //                = PieceManager::getPrintName(*seat->piece());

    if (!full)
        return textBlankBoard;

    int index = int(bottomColor_);
    return PRESTR[index] + textBlankBoard + SUFSTR[index];
}

bool Board::inSeat_(PPiece piece) const
{
    for (int r = 0; r < SEATROW; ++r)
        for (int c = 0; c < SEATCOL; ++c)
            if (piece == seats_[r][c])
                return true;

    return false;
}

PPiece Board::getUnUsedPiece_(QChar ch) const
{
    if (ch == NullChar || !Chars.contains(ch))
        return nullptr;

    QList<PPiece> ckpies = pieces_.getColorKindPiece(Piece::getColor(ch), Piece::getKind(ch));
    for (int i = 0; i < ckpies.count(); ++i) {
        auto piece = ckpies.at(i);
        if (!inSeat_(piece))
            return piece;
    }

    return nullptr;
}

QList<SeatPiece> Board::getSeatPieceList_(Piece::Color color) const
{
    QList<SeatPiece> seatPieceList;
    for (int r = 0; r < SEATROW; ++r)
        for (int c = 0; c < SEATCOL; ++c)
            if (color == Piece::Color::NOTCOLOR || color == seats_[r][c]->color())
                seatPieceList.append({ { r, c }, seats_[r][c] });

    return seatPieceList;
}

bool Board::setFromSeatPieceList_(const QList<SeatPiece>& seatPieceList)
{
    if (seatPieceList.count() != SEATNUM)
        return false;

    for (int i = 0; i < seatPieceList.count(); ++i)
        setPiece(seatPieceList.at(i));
    return true;
}

QString Board::getPieChars_(const QList<SeatPiece>& seatPieceList) const
{
    if (seatPieceList.count() != SEATNUM)
        return QString();

    QString qstr(SEATNUM, NullChar);
    for (int i = 0; i < qstr.count(); ++i) {
        auto piece = seatPieceList.at(i).second;
        if (piece)
            qstr[i] = piece->ch();
    }

    return qstr;
}

QList<SeatPiece> Board::getSeatPieceList_pieChars_(const QString& pieChars) const
{
    QList<SeatPiece> seatPieceList;
    if (pieChars.length() != SEATNUM)
        return seatPieceList;

    int index = 0;
    for (int r = 0; r < SEATROW; ++r)
        for (int c = 0; c < SEATCOL; ++c)
            seatPieceList.append({ { r, c }, getUnUsedPiece_(pieChars[index++]) });

    return seatPieceList;
}

QString Board::pieCharsToFEN_(const QString& pieChars) const
{
    QString fen {};
    if (pieChars.length() != SEATNUM)
        return fen;

    for (int index = 0; index < pieChars.length(); index += 9) {
        QString line { pieChars.mid(index, 9) }, qstr {};
        int num { 0 };
        for (auto ch : line) {
            if (ch != NullChar) {
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
        fen.prepend(qstr).prepend(FENSplitChar);
    }
    fen.remove(0, 1);

    //assert(FENTopieChars(fen) == pieceChars);
    return fen;
}

QString Board::FENTopieChars_(const QString& fen) const
{
    QString pieceChars {};
    QStringList strList { fen.split(FENSplitChar) };
    for (auto& line : strList) {
        QString qstr {};
        for (auto ch : line)
            if (ch.isDigit())
                qstr.append(QString(ch.digitValue(), NullChar));
            else
                qstr.append(ch);
        pieceChars.prepend(qstr);
    }

    assert(fen == pieCharsToFEN_(pieceChars));
    return pieceChars;
}
