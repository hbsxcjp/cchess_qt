#include "piece.h"
#include <functional>

int SeatManager::rotateRow(int row)
{
    return SEATROW - row - 1;
}

int SeatManager::rotateCol(int col)
{
    return SEATCOL - col - 1;
}

int SeatManager::index(const Seat& seat)
{
    return seat.first * SEATCOL + seat.second;
}

int SeatManager::rowcol(const Seat& seat)
{
    return seat.first * 10 + seat.second;
}

int SeatManager::rowcols(const MovSeat& movseat)
{
    return rowcol(movseat.first) * 100 + rowcol(movseat.second);
}

Seat SeatManager::seat(int rowcol)
{
    return { rowcol / 10, rowcol % 10 };
}

MovSeat SeatManager::movseat(int rowcols)
{
    return { seat(rowcols / 100), seat(rowcols % 100) };
}

QList<Seat> SeatManager::allSeats()
{
    QList<Seat> seatList;
    for (int r = 0; r < SEATROW; ++r)
        for (int c = 0; c < SEATCOL; ++c)
            seatList.append({ r, c });

    return seatList;
}

bool SeatManager::movSeatIsInvalid(const MovSeat& movseat)
{
    return movseat.first == movseat.second;
}

void SeatManager::changeSeat(Seat& seat, SeatManager::ChangeType ct)
{
    if (ct == SeatManager::ChangeType::NOCHANGE || ct == SeatManager::ChangeType::EXCHANGE)
        return;

    if (ct == SeatManager::ChangeType::SYMMETRY || ct == SeatManager::ChangeType::ROTATE)
        seat.second = SeatManager::rotateCol(seat.second);

    if (ct == SeatManager::ChangeType::ROTATE)
        seat.first = SeatManager::rotateRow(seat.first);
}

QString SeatManager::printSeat(const Seat& seat)
{
    return QString("<%1,%2>").arg(seat.first).arg(seat.second);
}

QString SeatManager::printSeatList(const QList<Seat>& seatList)
{
    QString qstr {};
    int count = seatList.count();
    if (count > 0) {
        for (int i = 0; i < count; ++i) {
            qstr.append(SeatManager::printSeat(seatList[i]));
            // 每行SEATCOL个数据
            if ((i % SEATCOL == SEATCOL - 1) && i != count - 1)
                qstr.append("\n");
        }
        qstr.append(QString("【%1】").arg(count));
    }

    return qstr;
}

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
    int k = PieceManager::getChChars().indexOf(ch.toUpper());
    if (k < 0)
        return Kind::NOTKIND;

    return Kind(k);
}

QChar Piece::ch() const
{
    return PieceManager::getChChars().at(int(color_) * KINDNUM + int(kind_));
}

QChar Piece::name() const
{
    switch (kind_) {
    case Kind::KING:
    case Kind::ADVISOR:
    case Kind::BISHOP:
        return PieceManager::getNameChars().at(kind_ * 2 + color_);
    case Kind::KNIGHT:
    case Kind::ROOK:
    case Kind::CANNON:
        return PieceManager::getNameChars().at(kind_ + 3);
    default: //    case Kind::PAWN:
        return PieceManager::getNameChars().at(color_ + 9);
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

QList<Seat> Piece::put(SeatManager::Seatside homeSide) const
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
        seatList = SeatManager::allSeats();
        break;
    default:
        //case Kind::PAWN:
        for (int r = 3; r < SEATROW; ++r)
            for (int c = 0; c < SEATCOL; ++c)
                if (r > 3 || c % 2 == 0)
                    seatList.append({ r, c });
        break;
    }
    if (homeSide == SeatManager::Seatside::THERE)
        for (int i = 0; i < seatList.count(); ++i)
            SeatManager::changeSeat(seatList[i], SeatManager::ChangeType::ROTATE);

    return seatList;
}

QList<Seat> Piece::move(Seat seat, SeatManager::Seatside homeSide) const
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
        // 先行后列，先小后大。顺序固定，为Board::canMove()分析走棋规则打下基础
        for (int r = row - 1; r >= 0; --r)
            seatList.append({ r, col });
        for (int r = row + 1; r < SEATROW; ++r)
            seatList.append({ r, col });
        for (int c = col - 1; c >= 0; --c)
            seatList.append({ row, c });
        for (int c = col + 1; c < SEATCOL; ++c)
            seatList.append({ row, c });
        break;
    case Kind::PAWN: {
        QVector<QPair<bool, Seat>> keepSeats {
            { true, { row - 1, col } }, { true, { row + 1, col } },
            { true, { row, col - 1 } }, { true, { row, col + 1 } }
        };
        if (homeSide == SeatManager::Seatside::HERE)
            keepSeats[0].first = false;
        else
            keepSeats[1].first = false;
        if (col == SEATCOL - 1)
            keepSeats[3].first = false;
        else if (col == 0)
            keepSeats[2].first = false;

        // 已过河
        if ((row >= SEATROW / 2) == (homeSide == SeatManager::Seatside::HERE)) {
            if (row == SEATROW - 1)
                keepSeats[1].first = false;
            else if (row == 0)
                keepSeats[0].first = false;
        } else {
            keepSeats[2].first = false;
            keepSeats[3].first = false;
            keepSeats[homeSide == SeatManager::Seatside::HERE ? 0 : 1].first = false;
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

QList<PPiece> Pieces::getColorPiece(Piece::Color color) const
{
    QList<PPiece> pieceList;
    for (int k = 0; k < KINDNUM; ++k)
        pieceList.append(pieces_[color][k]);

    return pieceList;
}

Piece::Color PieceManager::getOtherColor(Piece::Color color)
{
    return color == Piece::Color::RED ? Piece::Color::BLACK : Piece::Color::RED;
}

const QString PieceManager::getZhChars()
{
    return (preChars_ + nameChars_ + movChars_
        + numChars_[Piece::Color::RED] + numChars_[Piece::Color::BLACK]);
}

const QString PieceManager::getICCSChars() { return ICCS_ColChars_ + ICCS_RowChars_; }

const QString PieceManager::getFENStr() { return FENStr_; }

const QString PieceManager::getChChars() { return chChars_; }

const QString PieceManager::getNameChars() { return nameChars_; }

const QChar PieceManager::getFENSplitChar() { return FENSplitChar_; }

bool PieceManager::redIsBottom(const QString& fen)
{
    return fen.indexOf(chChars_[0]) < SEATNUM / 2;
}

int PieceManager::getRowFromICCSChar(QChar ch) { return ICCS_RowChars_.indexOf(ch); }

int PieceManager::getColFromICCSChar(QChar ch) { return ICCS_ColChars_.indexOf(ch); }

QChar PieceManager::getOtherChar(QChar ch)
{
    return ch.isLetter() ? (ch.isUpper() ? ch.toLower() : ch.toUpper()) : ch;
}

QChar PieceManager::getColICCSChar(int col) { return ICCS_ColChars_.at(col); }

QChar PieceManager::getName(QChar ch)
{
    //    int chIndex_nameIndex[][2] {
    //        { 0, 0 }, { 1, 2 }, { 2, 4 }, { 3, 6 }, { 4, 7 }, { 5, 8 }, { 6, 9 },
    //        { 7, 1 }, { 8, 3 }, { 9, 5 }, { 10, 6 }, { 11, 7 }, { 12, 8 }, { 13, 10 }
    //    };
    //    return nameChars_.at(chIndex_nameIndex[chChars_.indexOf(ch)][1]);
    int index = chChars_.indexOf(ch), kind = index % KINDNUM, color = index / KINDNUM;
    if (kind < Piece::Kind::BISHOP)
        return nameChars_.at(kind * 2 + color);
    else if (kind == Piece::Kind::PAWN)
        return nameChars_.at(color + 9);
    else
        return nameChars_.at(kind + 3);
}

Piece::Color PieceManager::getColor(QChar ch)
{
    return ch.isLower() ? Piece::Color::BLACK : Piece::Color::RED;
}

Piece::Color PieceManager::getColorFromZh(QChar numZh)
{
    return numChars_[Piece::Color::RED].indexOf(numZh) >= 0 ? Piece::Color::RED : Piece::Color::BLACK;
}

int PieceManager::getIndex(const int seatsLen, const bool isBottom, QChar preChar)
{
    int index = getPreChars__(seatsLen).indexOf(preChar);
    return isBottom ? seatsLen - 1 - index : index;
}

QChar PieceManager::getIndexChar(const int seatsLen, const bool isBottom, const int index)
{
    return getPreChars__(seatsLen).at(isBottom ? seatsLen - 1 - index : index);
}

QChar PieceManager::nullChar() { return nullChar_; }

QChar PieceManager::redKingChar() { return chChars_[0]; };

int PieceManager::getMovNum(bool isBottom, QChar movChar)
{
    return (movChars_.indexOf(movChar) - 1) * (isBottom ? 1 : -1);
}

QChar PieceManager::getMovChar(bool isSameRow, bool isBottom, bool isLowToUp)
{
    return movChars_.at(isSameRow ? 1 : (isBottom == isLowToUp ? 2 : 0));
}

int PieceManager::getNum(Piece::Color color, QChar numChar)
{
    return numChars_[color].indexOf(numChar) + 1;
}

QChar PieceManager::getNumChar(Piece::Color color, int num)
{
    return numChars_[color].at(num - 1);
}

int PieceManager::getCol(bool isBottom, const int num)
{
    return isBottom ? SEATCOL - num : num - 1;
}

QChar PieceManager::getColChar(const Piece::Color color, bool isBottom, const int col)
{
    return numChars_[color].at(isBottom ? SEATCOL - col - 1 : col);
}

bool PieceManager::isKing(QChar name)
{
    return nameChars_.leftRef(2).indexOf(name) >= 0;
}

bool PieceManager::isAdvBish(QChar name)
{
    return nameChars_.midRef(2, 4).indexOf(name) >= 0;
}

bool PieceManager::isStronge(QChar name)
{
    return nameChars_.midRef(6, 5).indexOf(name) >= 0;
}

bool PieceManager::isLineMove(QChar name)
{
    return isKing(name) || nameChars_.midRef(7, 4).indexOf(name) >= 0;
}

bool PieceManager::isPawn(QChar name)
{
    return nameChars_.rightRef(2).indexOf(name) >= 0;
}

bool PieceManager::isPiece(QChar name)
{
    return nameChars_.indexOf(name) >= 0;
}

const QString PieceManager::getPreChars__(int length)
{
    return (length == 2 ? QString(preChars_).remove(1, 1) // "前后"
                        : (length == 3 ? preChars_ // "前中后"
                                       : numChars_[Piece::Color::RED].left(5))); // "一二三四五";
}

const QString PieceManager::chChars_ { "KABNRCPkabnrcp" };
const QString PieceManager::preChars_ { "前中后" };
const QString PieceManager::nameChars_ { "帅将仕士相象马车炮兵卒" };
const QString PieceManager::movChars_ { "退平进" };
const QMap<Piece::Color, QString> PieceManager::numChars_ {
    { Piece::Color::RED, "一二三四五六七八九" },
    { Piece::Color::BLACK, "１２３４５６７８９" }
};
const QString PieceManager::ICCS_ColChars_ { "abcdefghi" };
const QString PieceManager::ICCS_RowChars_ { "0123456789" };
const QString PieceManager::FENStr_ { "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR" };
const QChar PieceManager::nullChar_ { '_' };
const QChar PieceManager::FENSplitChar_ { '/' };
