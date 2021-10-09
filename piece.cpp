#include "piece.h"
#include <functional>

Piece::Piece(Color color, Kind kind)
    : color_(color)
    , kind_(kind)
{
}

Piece::Color Piece::getColor(QChar ch)
{
    return ch.isLower() ? Color::BLACK : Color::RED;
}

Piece::Kind Piece::getKind(QChar ch)
{
    int k = Chars.at(0).indexOf(ch.toUpper());
    if (k < 0)
        return Kind::NOTKIND;

    return Kind(k);
}

QChar Piece::ch() const
{
    return Chars.at(color_ == Color::RED ? 0 : 1).at(int(kind_));
}

QChar Piece::name() const
{
    bool isRed = color_ == Color::RED;
    switch (kind_) {
    case Kind::KING:
        return isRed ? L'帅' : L'将';
    case Kind::ADVISOR:
        return isRed ? L'仕' : L'士';
    case Kind::BISHOP:
        return isRed ? L'相' : L'象';
    case Kind::KNIGHT:
        return L'马';
    case Kind::ROOK:
        return L'车';
    case Kind::CANNON:
        return L'炮';
    default:
        //    case Kind::PAWN:
        return isRed ? L'兵' : L'卒';
    }
}

QChar Piece::printName() const
{
    if (color_ == Color::BLACK
        && (kind_ == Kind::KNIGHT || kind_ == Kind::ROOK || kind_ == Kind::CANNON))
        return (kind_ == Kind::KNIGHT) ? L'馬' : (kind_ == Kind::ROOK ? L'車' : L'砲');
    else
        return name();
}

QList<Seat> Piece::put(Seatside homeSide) const
{
    QList<Seat> seatList;
    switch (kind_) {
    case Kind::KING:
        for (int r = 0; r < 3; ++r)
            for (int c = 3; c < 6; ++c)
                seatList.append({ r, c });
        break;
    case Kind::ADVISOR:
        for (int r = 0; r < 3; ++r)
            for (int c = 3; c < 6; ++c)
                if (r % 2 != c % 2)
                    seatList.append({ r, c });
        break;
    case Kind::BISHOP:
        for (int r = 0; r < SEATROW / 2; ++r)
            for (int c = 0; c < SEATCOL; ++c)
                if (r % 2 == 0 && c % 2 == 0 && qAbs(c - r) % 4 != 0)
                    seatList.append({ r, c });
        break;
    case Kind::KNIGHT:
    case Kind::ROOK:
    case Kind::CANNON:
        for (int r = 0; r < SEATROW; ++r)
            for (int c = 0; c < SEATCOL; ++c)
                seatList.append({ r, c });
        break;
    default:
        //case Kind::PAWN:
        for (int r = 3; r < SEATROW; ++r)
            for (int c = 0; c < SEATCOL; ++c)
                if (r > 3 || c % 2 == 0)
                    seatList.append({ r, c });
        break;
    }
    if (homeSide == Seatside::THERE)
        for (int i = 0; i < seatList.count(); ++i)
            rotateSeat(seatList[i]);

    return seatList;
}

QList<Seat> Piece::move(Seat seat, Seatside homeSide) const
{
    std::function<void(QList<Seat> & seatList, QVector<QPair<bool, Seat>>)>
        getSeats_ = [](QList<Seat>& seatList, QVector<QPair<bool, Seat>> keepSeats) {
            for (int i = 0; i < keepSeats.count(); ++i)
                if (keepSeats[i].first)
                    seatList.append(keepSeats[i].second);
        };

    QList<Seat> seatList;
    int row = seat.first, col = seat.second;
    switch (kind_) {
    case Kind::KING: {
        // 前后左右的位置
        QVector<QPair<bool, Seat>> keepSeats {
            { true, { row + 1, col } }, { true, { row - 1, col } },
            { true, { row, col - 1 } }, { true, { row, col + 1 } }
        };
        if (row == 0 || row == 7)
            keepSeats[1].first = false;
        else if (row == 2 || row == 9)
            keepSeats[0].first = false;
        if (col == 3)
            keepSeats[2].first = false;
        else if (col == 5)
            keepSeats[3].first = false;
        getSeats_(seatList, keepSeats);
    } break;
    case Kind::ADVISOR:
        if (col == 4)
            seatList = { { row - 1, col - 1 }, { row - 1, col + 1 }, { row + 1, col - 1 }, { row + 1, col + 1 } };
        else
            seatList = { { (row < SEATROW / 2) ? 1 : 8, 4 } };
        break;
    case Kind::BISHOP: {
        QVector<QPair<bool, Seat>> keepSeats {
            { true, { row + 2, col + 2 } }, { true, { row + 2, col - 2 } },
            { true, { row - 2, col + 2 } }, { true, { row - 2, col - 2 } }
        };
        if (row == 0 || row == 5) {
            keepSeats[2].first = false;
            keepSeats[3].first = false;
        } else if (row == 4 || row == 9) {
            keepSeats[0].first = false;
            keepSeats[1].first = false;
        }
        if (col == 0) {
            keepSeats[1].first = false;
            keepSeats[3].first = false;
        } else if (col == SEATCOL - 1) {
            keepSeats[0].first = false;
            keepSeats[2].first = false;
        }
        getSeats_(seatList, keepSeats);
    } break;
    case Kind::KNIGHT: {
        QVector<QPair<bool, Seat>> keepSeats {
            { true, { row - 2, col - 1 } }, { true, { row - 2, col + 1 } },
            { true, { row - 1, col - 2 } }, { true, { row - 1, col + 2 } },
            { true, { row + 1, col - 2 } }, { true, { row + 1, col + 2 } },
            { true, { row + 2, col - 1 } }, { true, { row + 2, col + 1 } }
        };
        if (row < 2) {
            keepSeats[0].first = false;
            keepSeats[1].first = false;
            if (row == 0) {
                keepSeats[2].first = false;
                keepSeats[3].first = false;
            }
        } else if (row > SEATROW - 3) {
            keepSeats[6].first = false;
            keepSeats[7].first = false;
            if (row == SEATROW - 1) {
                keepSeats[4].first = false;
                keepSeats[5].first = false;
            }
        }
        if (col < 2) {
            keepSeats[2].first = false;
            keepSeats[4].first = false;
            if (col == 0) {
                keepSeats[0].first = false;
                keepSeats[6].first = false;
            }
        } else if (col > SEATCOL - 3) {
            keepSeats[3].first = false;
            keepSeats[5].first = false;
            if (col == SEATCOL - 1) {
                keepSeats[1].first = false;
                keepSeats[7].first = false;
            }
        }
        getSeats_(seatList, keepSeats);
    } break;
    case Kind::ROOK:
    case Kind::CANNON:
        for (int r = 0; r < SEATROW; ++r)
            if (r != row)
                seatList.append({ r, col });
        for (int c = 0; c < SEATCOL; ++c)
            if (c != col)
                seatList.append({ row, c });
        break;
    case Kind::PAWN: {
        QVector<QPair<bool, Seat>> keepSeats {
            { true, { row - 1, col } }, { true, { row + 1, col } },
            { true, { row, col - 1 } }, { true, { row, col + 1 } }
        };
        if (homeSide == Seatside::HERE)
            keepSeats[0].first = false;
        else
            keepSeats[1].first = false;
        if (col == SEATCOL - 1)
            keepSeats[3].first = false;
        else if (col == 0)
            keepSeats[2].first = false;

        // 已过河
        if ((row >= SEATROW / 2) == (homeSide == Seatside::HERE)) {
            if (row == SEATROW - 1)
                keepSeats[1].first = false;
            else if (row == 0)
                keepSeats[0].first = false;
        } else {
            keepSeats[2].first = false;
            keepSeats[3].first = false;
            keepSeats[homeSide == Seatside::HERE ? 0 : 1].first = false;
        }
        getSeats_(seatList, keepSeats);
    } break;
    default:
        break;
    }
    return seatList;
}

const QString Piece::toString() const
{
    return QString().append(color_ == Color::RED ? L'红' : L'黑').append(printName()).append(ch());
}

const QString Piece::putString(Seatside homeSide) const
{
    QString qstr;
#ifdef CREATE_TESTPIECE_TEXT
    qstr.append("\"");
#endif
    qstr.append(QString("(%1).put(%2):").arg(toString()).arg(homeSide));
#ifdef CREATE_TESTPIECE_TEXT
    qstr.append("\"\n");
#endif

    auto seatList = put(homeSide);
    qstr.append(printSeatList(seatList));
#ifdef CREATE_TESTPIECE_TEXT
    qstr.append(",");
#endif

    return qstr;
}

const QString Piece::moveString(Seat seat, Seatside homeSide) const
{
    QString qstr;
#ifdef CREATE_TESTPIECE_TEXT
    qstr.append("\"");
#endif
    qstr.append(QString("(%1).move(%2,%3):").arg(toString()).arg(printSeat(seat)).arg(homeSide));
#ifdef CREATE_TESTPIECE_TEXT
    qstr.append("\"\n");
#endif

    auto seatList = move(seat, homeSide);
    qstr.append(printSeatList(seatList));
#ifdef CREATE_TESTPIECE_TEXT
    qstr.append(",");
#endif

    return qstr;
}

Pieces::Pieces()
{
    int kindNums[] { 1, 2, 2, 2, 2, 2, 5 };
    for (int c = 0; c < COLORNUM; ++c)
        for (int k = 0; k < KINDNUM; ++k)
            for (int i = 0; i < kindNums[k]; ++i)
                pieces_[c][k].append(new Piece(Piece::Color(c), Piece::Kind(k)));
}

Pieces::~Pieces()
{
    for (int c = 0; c < COLORNUM; ++c)
        for (int k = 0; k < KINDNUM; ++k) {
            auto& kpies = pieces_[c][k];
            for (int i = 0; i < kpies.count(); ++i)
                delete kpies.at(i);
        }
}

QList<PPiece> Pieces::getColorPiece(Piece::Color color) const
{
    QList<PPiece> pieceList;
    for (int k = 0; k < KINDNUM; ++k) {
        auto& kpies = pieces_[color][k];
        for (int i = 0; i < kpies.count(); ++i)
            pieceList.append(kpies.at(i));
    }

    return pieceList;
}

QList<PPiece> Pieces::getAllPiece(bool onlyKind) const
{
    QList<PPiece> pieceList;
    for (int c = 0; c < COLORNUM; ++c) {
        for (int k = 0; k < KINDNUM; ++k) {
            auto& kpies = pieces_[c][k];
            int num = onlyKind ? 1 : kpies.count();
            for (int i = 0; i < num; ++i)
                pieceList.append(kpies.at(i));
        }
    }

    return pieceList;
}

QList<Seat> Pieces::getAllSeat() const
{
    return pieces_[Piece::Color::RED][Piece::Kind::ROOK].at(0)->put(Seatside::HERE);
}

int rotateRow(int row)
{
    return SEATROW - row - 1;
}

int rotateCol(int col)
{
    return SEATCOL - col - 1;
}

Seat& rotateSeat(Seat& seat)
{
    seat.first = rotateRow(seat.first);
    seat.second = rotateCol(seat.second);

    return seat;
}

QString printSeat(const Seat& seat)
{
    return QString("<%1,%2>").arg(seat.first).arg(seat.second);
}

QString printSeatList(const QList<Seat>& seatList)
{
    QString qstr {};
    int count = seatList.count();
    if (count > 0) {
#ifdef CREATE_TESTPIECE_TEXT
        qstr.append("\"");
#endif

        for (int i = 0; i < count; ++i) {
            qstr.append(printSeat(seatList[i]));

#ifdef CREATE_TESTPIECE_TEXT
            // 每行SEATCOL个数据
            if ((i % SEATCOL == SEATCOL - 1) && i != count - 1)
                qstr.append("\"\n\"");
#endif
        }
        qstr.append(QString("【%1】").arg(count));

#ifdef CREATE_TESTPIECE_TEXT
        qstr.append("\"\n");
#endif
    }

    return qstr;
}

constexpr QChar NullChar { '_' };
constexpr QChar FENSplitChar { '/' };
const QVector<QString> Chars { "KABNRCP", "kabnrcp" };
const QString FEN { "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w - - 0 1" };
