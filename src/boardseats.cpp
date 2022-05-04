#include "boardseats.h"
#include "boardpieces.h"
#include "piece.h"
#include "piecebase.h"
#include "seat.h"
#include "seatbase.h"

BoardSeats::BoardSeats()
    : seats(Seat::creatSeats())
{
}

BoardSeats::~BoardSeats()
{
    for (auto& seat : seats)
        delete seat;
}

void BoardSeats::clear()
{
    for (auto& seat : seats)
        seat->setPiece(Q_NULLPTR);
}

Seat* BoardSeats::getSeat(int index) const
{
    return seats.at(index); // 如用value(index), 则可能返回空指针
}

Seat* BoardSeats::getSeat(const Coord& coord) const
{
    return getSeat(SeatBase::getIndex(coord));
}

Seat* BoardSeats::changeSeat(const Seat* seat, ChangeType ct) const
{
    return getSeat(SeatBase::changeCoord(seat->coord(), ct));
}

void BoardSeats::changeLayout(const BoardPieces* boardPieces, ChangeType ct)
{
    if (ct == ChangeType::SYMMETRY_H || ct == ChangeType::ROTATE) {
        int rowNum { SeatBase::getRowNum() }, colNum { SeatBase::getColNum() };
        int maxRow = ct == ChangeType::SYMMETRY_H ? rowNum : rowNum / 2,
            maxCol = ct == ChangeType::SYMMETRY_H ? colNum / 2 : colNum;
        for (int row = 0; row < maxRow; ++row)
            for (int col = 0; col < maxCol; ++col) {
                Seat* seat = getSeat({ row, col });
                Seat* changedSeat = changeSeat(seat, ct);
                Piece* piece = seat->piece();
                Piece* changePiece = changedSeat->piece();
                if (!piece && !changePiece) // 两个位置都无棋子
                    continue;

                changedSeat->setPiece(Q_NULLPTR); // 切断changeSeat与changePiece间的联系！
                seat->setPiece(changePiece); // 建立seat与changePiece间的联系，同时切断seat与piece间的联系
                changedSeat->setPiece(piece); // 建立changeSeat与piece间的联系
            }
    } else if (ct == ChangeType::EXCHANGE) {
        QList<QPair<Seat*, Piece*>> seatPieces;
        for (auto& seat : seats)
            if (seat->hasPiece()) {
                seatPieces.append({ seat, boardPieces->getOtherPiece(seat->piece()) });
                seat->setPiece(Q_NULLPTR);
            }

        for (auto& seatPiece : seatPieces)
            seatPiece.first->setPiece(seatPiece.second);
    }
}

QString BoardSeats::getPieceChars() const
{
    QString pieChars;
    for (auto& seat : seats)
        pieChars.append(seat->hasPiece() ? seat->piece()->ch() : PieceBase::NULLCHAR);

    return pieChars;
}

bool BoardSeats::setPieceChars(const BoardPieces* boardPieces, const QString& pieceChars)
{
    if (pieceChars.size() != seats.size())
        return false;

    clear();
    int index = 0;
    for (auto& seat : seats) {
        QChar ch = pieceChars.at(index++);
        if (ch != PieceBase::NULLCHAR)
            seat->setPiece(boardPieces->getNonLivePiece(PieceBase::getColor(ch), PieceBase::getKind(ch)));
    }

    //    Q_ASSERT(pieceChars == getPieceChars());
    return true;
}

QString BoardSeats::getFEN() const
{
    return SeatBase::pieCharsToFEN(getPieceChars());
}

bool BoardSeats::setFEN(const BoardPieces* boardPieces, const QString& fen)
{
    return setPieceChars(boardPieces, SeatBase::FENToPieChars(fen));
}

bool BoardSeats::isFace(Seat* redSeat, Seat* blackSeat) const
{
    int col { redSeat->col() };
    if (col != blackSeat->col())
        return false;

    int redRow = redSeat->row(), blackRow = blackSeat->row();
    int lowRow { qMin(redRow, blackRow) },
        upRow { qMax(redRow, blackRow) };
    for (int row = lowRow + 1; row < upRow; ++row)
        if (getSeat({ row, col })->hasPiece())
            return false;

    return true;
}

QString BoardSeats::toString(PieceColor bottomColor, bool hasEdge) const
{
    // 棋盘上边标识字符串
    const QString preStrR[] = {
        "　　　　　　　黑　方　　　　　　　\n１　２　３　４　５　６　７　８　９\n",
        "　　　　　　　红　方　　　　　　　\n一　二　三　四　五　六　七　八　九\n"
    };

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

    // 棋盘下边标识字符串
    const QString sufStr[] = {
        "九　八　七　六　五　四　三　二　一\n　　　　　　　红　方　　　　　　　\n",
        "９　８　７　６　５　４　３　２　１\n　　　　　　　黑　方　　　　　　　\n"
    };

    int colNum { SeatBase::getColNum() };
    for (auto& seat : seats)
        if (seat->hasPiece()) {
            int index = SeatBase::symmetryRow(seat->row()) * 2 * (colNum * 2) + seat->col() * 2;
            textBlankBoard[index] = seat->piece()->printName();
        }

    if (!hasEdge)
        return textBlankBoard;

    int index = int(bottomColor);
    return preStrR[index] + textBlankBoard + sufStr[index];
}
