#include "seat.h"
#include "piece.h"

Seat::Seat(int row, int col)
    : row_(row)
    , col_(col)
{
}

PSeat Seat::setPiece(PPiece piece)
{
    if (piece_)
        piece_->setSeat(nullptr);
    if (piece)
        piece->setSeat(this);
    piece_ = piece;

    return this;
}

PPiece Seat::moveTo(PSeat toSeat, PPiece fillPiece)
{
    PPiece piece = getPiece(),
           toPiece = toSeat->getPiece();
    setPiece(fillPiece); // 先执行，清空与piece的联系
    toSeat->setPiece(piece);

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

QList<PSeat> Seats::getSeatList(const QList<SeatCoord>& seatCoords) const
{
    QList<PSeat> seatList;
    for (auto& seatCoord : seatCoords)
        seatList.append(getSeat(seatCoord));

    return seatList;
}

PSeat Seats::getChangeSeat(PSeat& seat, ChangeType ct) const
{
    if (ct == ChangeType::SYMMETRY)
        return getSeat(seat->row(), symmetryCol(seat->col()));
    else if (ct == ChangeType::ROTATE)
        return getSeat(symmetryRow(seat->row()), symmetryCol(seat->col()));
    else
        //(ct == ChangeType::NOCHANGE || ct == ChangeType::EXCHANGE)
        return seat;
}

void Seats::changeLayout(const Pieces* pieces, ChangeType ct)
{
    if (ct == ChangeType::NOCHANGE)
        return;

    if (ct == ChangeType::SYMMETRY || ct == ChangeType::ROTATE) {
        int topRow = ct == ChangeType::SYMMETRY ? SEATROW : SEATROW / 2,
            topCol = ct == ChangeType::SYMMETRY ? SEATCOL / 2 : SEATCOL;
        for (int row = 0; row < topRow; ++row)
            for (int col = 0; col < topCol; ++col) {
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
            for (auto& seat : pieces->getLiveSeatList(color)) {
                seatPieces.append({ seat, seat->getPiece() });
                seat->setPiece(nullptr);
            }
        for (auto& seatPiece : seatPieces)
            seatPiece.first->setPiece(pieces->getOtherPiece(seatPiece.second));
    }
}

QString Seats::getPieChars() const
{
    QString qstr;
    for (int row = 0; row < SEATROW; ++row)
        for (int col = 0; col < SEATCOL; ++col) {
            auto piece = getSeat(row, col)->getPiece();
            qstr.append(piece ? piece->ch() : Pieces::nullChar);
        }

    return qstr;
}

bool Seats::setPieChars(const Pieces* pieces, const QString& pieChars)
{
    if (pieChars.count() != SEATNUM)
        return false;

    int index = 0;
    for (int row = 0; row < SEATROW; ++row)
        for (int col = 0; col < SEATCOL; ++col) {
            QChar ch = pieChars[index++];
            PPiece piece = (ch == Pieces::nullChar
                    ? nullptr
                    : pieces->getNotLivePiece(pieces->getColor(ch), pieces->getKind(ch)));
            getSeat(row, col)->setPiece(piece);
        }

    return true;
}

QString Seats::getFEN() const
{
    return pieCharsToFEN(getPieChars());
}

bool Seats::setFEN(const Pieces* pieces, const QString& fen)
{
    return setPieChars(pieces, FENToPieChars(fen));
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

            int index = (SEATCOL - seat->row()) * 2 * (SEATCOL * 2) + seat->col() * 2;
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
    QString pieceChars {};
    QStringList strList { fen.split(Pieces::FENSplitChar) };
    if (strList.count() != SEATROW)
        return pieceChars;

    for (auto& line : strList) {
        QString qstr {};
        for (auto ch : line)
            qstr.append(ch.isDigit() ? QString(ch.digitValue(), Pieces::nullChar) : ch);
        pieceChars.prepend(qstr);
    }

    return pieceChars;
}
