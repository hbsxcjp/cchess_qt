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
    bool res = setFromSeatPieceList_(getSeatPieceList_pieChars_(FENTopieChars_(fen)));
    setBottomColor_();
    return res;
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

    if (!full)
        return textBlankBoard;

    int index = int(bottomColor_);
    return PRESTR[index] + textBlankBoard + SUFSTR[index];
}

bool Board::isFace_() const
{
    Seat rkseat { getKingSeat_(Piece::Color::RED) },
        bkseat { getKingSeat_(Piece::Color::BLACK) };
    int col { rkseat.second };
    if (col != bkseat.second)
        return false;

    bool redIsBottom { bottomColor_ == Piece::Color::RED };
    int lrow { redIsBottom ? rkseat.first : bkseat.first },
        urow { redIsBottom ? bkseat.first : rkseat.first };
    for (int r = lrow + 1; r < urow; ++r)
        if (getPiece_({ r, col }))
            return false;

    return true;
}

Seat Board::getKingSeat_(Piece::Color color) const
{
    auto kpies = pieces_.getColorKindPiece(color, Piece::Kind::KING);
    assert(kpies.count() == 1);

    auto kpie = kpies.at(0);
    auto seatList = kpie->put(Seatside::HERE);
    seatList.append(kpie->put(Seatside::THERE));
    for (auto& seat : seatList) {
        if (getPiece_(seat) == kpie)
            return seat;
    }

    return { -1, -1 };
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
    if (ch == NullChar
        || !(Chars.at(0).contains(ch) || Chars.at(1).contains(ch)))
        return nullptr;

    QList<PPiece> ckpies = pieces_.getColorKindPiece(Piece::getColor(ch), Piece::getKind(ch));
    for (int i = 0; i < ckpies.count(); ++i) {
        auto piece = ckpies.at(i);
        if (!inSeat_(piece))
            return piece;
    }

    return nullptr;
}

void Board::setBottomColor_(Piece::Color color)
{
    if (color != Piece::Color::NOTCOLOR
        || getKingSeat_(color).first < (SEATROW / 2))
        bottomColor_ = color;
    else
        bottomColor_ = (color == Piece::Color::BLACK)
            ? Piece::Color::RED
            : Piece::Color::BLACK;
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

    for (int index = 0; index < SEATNUM; index += SEATCOL) {
        QString line { pieChars.mid(index, SEATCOL) }, qstr {};
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

    return fen;
}

QString Board::FENTopieChars_(const QString& fen) const
{
    QString pieceChars {};
    QStringList strList { fen.split(FENSplitChar) };
    if (strList.count() != SEATROW)
        return pieceChars;

    for (auto& line : strList) {
        QString qstr {};
        for (auto ch : line)
            qstr.append(ch.isDigit() ? QString(ch.digitValue(), NullChar) : ch);
        pieceChars.prepend(qstr);
    }

    return pieceChars;
}
