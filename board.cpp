#include "board.h"
#include "tools.h"

const QVector<QString> fens {
    PieceManager::getFENStr(),
    "5a3/4ak2r/6R2/8p/9/9/9/B4N2B/4K4/3c5"
};

const QString skipExplain { "Save the result to file." };

void Board::clean()
{
    for (int r = 0; r < SEATROW; ++r)
        for (int c = 0; c < SEATCOL; ++c)
            seats_[r][c] = nullptr;

    bottomColor_ = Piece::Color::NOTCOLOR;
}

void Board::reinit()
{
    setFEN(PieceManager::getFENStr());
}

QList<SeatPiece> Board::getSeatPieceList() const
{
    QList<SeatPiece> seatPieceList;
    for (int r = 0; r < SEATROW; ++r)
        for (int c = 0; c < SEATCOL; ++c)
            seatPieceList.append({ { r, c }, seats_[r][c] });

    return seatPieceList;
}

QList<SeatPiece> Board::getColorSeatPieceList(Piece::Color color) const
{
    QList<SeatPiece> seatPieceList;
    for (int r = 0; r < SEATROW; ++r)
        for (int c = 0; c < SEATCOL; ++c) {
            auto& piece = seats_[r][c];
            if (piece && color == piece->color())
                seatPieceList.append({ { r, c }, seats_[r][c] });
        }

    return seatPieceList;
}

PPiece Board::movePiece(const MovSeat& movseat)
{
    auto toPiece = getPiece(movseat.second);
    setPiece_(movseat.second, getPiece(movseat.first));
    setPiece_(movseat.first);

    return toPiece;
}

QList<Seat> Board::canMove(const Seat& seat)
{
    QList<Seat> seatList { canMove_(seat) };
    //  排除将帅对面、被将军的位置

    // 移动棋子后，检测是否会被对方将军
    auto color = getPiece(seat)->color();
    QMutableListIterator<Seat> seatIter(seatList);
    while (seatIter.hasNext()) {
        auto toSeat = seatIter.next();
        auto toPiece = movePiece({ seat, toSeat });
        if (isFace_() || isKilling(color))
            seatIter.remove();

        setPiece_(seat, getPiece(toSeat));
        setPiece_(toSeat, toPiece);
    }

    return seatList;
}

QList<Seat> Board::allCanMove(Piece::Color color)
{
    QList<Seat> seatList;
    auto seatPieceList = getColorSeatPieceList(color);
    for (auto& seatPiece : seatPieceList)
        seatList.append(canMove(seatPiece.first));

    return seatList;
}

bool Board::isCanMove(const Seat& fromSeat, const Seat& toSeat)
{
    return canMove(fromSeat).contains(toSeat);
}

bool Board::isKilling(Piece::Color color) const
{
    auto othColor = PieceManager::getOtherColor(color);
    auto kingSeat = getKingSeat_(color);
    for (auto& seatPiece : getColorSeatPieceList(othColor))
        for (auto& seat : canMove_(seatPiece.first))
            if (seat == kingSeat)
                return true;

    return false;
}

void Board::changeSide(SeatManager::ChangeType ct)
{
    if (ct == SeatManager::ChangeType::NOCHANGE)
        return;

    auto seatPieceList = getSeatPieceList();
    if (ct == SeatManager::ChangeType::EXCHANGE) {
        clean();
        for (auto& seatPiece : seatPieceList) {
            auto ch = seatPiece.second ? seatPiece.second->ch() : PieceManager::nullChar();
            if (ch != PieceManager::nullChar())
                ch = ch.isLower() ? ch.toUpper() : ch.toLower();
            setPiece_(seatPiece.first, getUnUsedPiece_(ch));
        }
        setBottomColor_();
    } else {
        for (auto& seatPiece : seatPieceList)
            SeatManager::changeSeat(seatPiece.first, ct);
        setFromSeatPieceList_(seatPieceList);
    }
}

QString Board::movSeatToStr(const MovSeat& movSeat) const
{
    QString qstr {};
    Seat fseat { movSeat.first }, tseat { movSeat.second };
    PPiece fromPiece { getPiece(fseat) };
    assert(fromPiece);

    Piece::Color color { fromPiece->color() };
    QChar name { fromPiece->name() };
    int fromRow { fseat.first }, fromCol { fseat.second },
        toRow { tseat.first }, toCol { tseat.second };
    bool isSameRow { fromRow == toRow }, isBottom { color == bottomColor_ };
    auto seats = getLiveSeatList_(color, name, fromCol);

    if (seats.size() > 1 && PieceManager::isStronge(name)) {
        if (PieceManager::isPawn(name))
            seats = getSortPawnLiveSeats_(isBottom, color, name);
        qstr.append(PieceManager::getIndexChar(seats.size(), isBottom, seats.indexOf(fseat)))
            .append(name);
    } else { //将帅, 仕(士),相(象): 不用“前”和“后”区别，因为能退的一定在前，能进的一定在后
        qstr.append(name)
            .append(PieceManager::getColChar(color, isBottom, fromCol));
    }
    qstr.append(PieceManager::getMovChar(isSameRow, isBottom, toRow > fromRow))
        .append(PieceManager::isLineMove(name) && !isSameRow
                ? PieceManager::getNumChar(color, abs(fromRow - toRow))
                : PieceManager::getColChar(color, isBottom, toCol));

    auto mvSeats = strToMovSeat(qstr);
    assert(fseat == mvSeats.first && tseat == mvSeats.second);

    return qstr;
}

MovSeat Board::strToMovSeat(const QString& zhStr, bool ignoreError) const
{
    assert(zhStr.size() == 4);
    Seat fseat {}, tseat {};
    QList<Seat> seats {};

    // 根据最后一个字符判断该着法属于哪一方
    Piece::Color color { PieceManager::getColorFromZh(zhStr.back()) };
    bool isBottom { color == bottomColor_ };
    int index {}, movDir { PieceManager::getMovNum(isBottom, zhStr.at(2)) };
    QChar name { zhStr.front() };

    if (PieceManager::isPiece(name)) { // 首字符为棋子名
        seats = getLiveSeatList_(color, name,
            PieceManager::getCol(isBottom, PieceManager::getNum(color, zhStr.at(1))));

        if (ignoreError && seats.size() == 0)
            return { fseat, tseat };
        assert(seats.size() > 0);
        //# 排除：士、象同列时不分前后，以进、退区分棋子。移动方向为退时，修正index
        index = (seats.size() == 2 && movDir == -1) ? 1 : 0; //&& isAdvBish(name)
    } else {
        name = zhStr.at(1);
        seats = (PieceManager::isPawn(name)
                ? getSortPawnLiveSeats_(isBottom, color, name)
                : getLiveSeatList_(color, name));
        if (ignoreError && seats.size() == 0)
            return { fseat, tseat };
        assert(seats.size() > 0);
        index = PieceManager::getIndex(seats.size(), isBottom, zhStr.front());
    }

    assert(index <= seats.length() - 1);
    fseat = seats.at(index);
    int num { PieceManager::getNum(color, zhStr.back()) },
        toCol { PieceManager::getCol(isBottom, num) };
    if (PieceManager::isLineMove(name)) {
        int trow { fseat.first + movDir * num };
        tseat = movDir == 0 ? Seat(fseat.first, toCol) : Seat(trow, fseat.second);
    } else { // 斜线走子：仕、相、马
        int colAway { abs(toCol - fseat.second) }, //  相距1或2列
            trow { fseat.first + movDir * (PieceManager::isAdvBish(name) ? colAway : (colAway == 1 ? 2 : 1)) };
        tseat = Seat(trow, toCol);
    }
    //assert(zhStr == getZh(fseat, tseat));

    return { fseat, tseat };
}

QString Board::getFEN() const
{
    return pieCharsToFEN(getPieChars_(getSeatPieceList()));
}

bool Board::setFEN(const QString& fen)
{
    return setFromSeatPieceList_(getSeatPieceList_pieChars_(FENTopieChars(fen)));
}

QString Board::pieCharsToFEN(const QString& pieChars)
{
    QString fen {};
    assert(pieChars.count() == SEATNUM);
    if (pieChars.count() != SEATNUM)
        return fen;

    for (int index = 0; index < SEATNUM; index += SEATCOL) {
        QString line { pieChars.mid(index, SEATCOL) }, qstr {};
        int num { 0 };
        for (auto ch : line) {
            if (ch != PieceManager::nullChar()) {
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
        fen.prepend(qstr).prepend(PieceManager::getFENSplitChar());
    }
    fen.remove(0, 1);

    return fen;
}

QString Board::FENTopieChars(const QString& fen)
{
    QString pieceChars {};
    QStringList strList { fen.split(PieceManager::getFENSplitChar()) };
    if (strList.count() != SEATROW)
        return pieceChars;

    for (auto& line : strList) {
        QString qstr {};
        for (auto ch : line)
            qstr.append(ch.isDigit() ? QString(ch.digitValue(), PieceManager::nullChar()) : ch);
        pieceChars.prepend(qstr);
    }

    return pieceChars;
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

    auto seatPieceList = getSeatPieceList();
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

QList<Seat> Board::canMove_(const Seat& seat) const
{
    PPiece piece = getPiece(seat);
    assert(piece);
    Piece::Color color = piece->color();
    Piece::Kind kind = piece->kind();
    auto homeSide = getSeatside_(color);
    int row { seat.first }, col { seat.second };

    // 排除目标同色的位置（Java风格迭代器）
    QList<Seat> seatList { piece->move(seat, homeSide) };
    QMutableListIterator<Seat> seatIter(seatList);
    // 排除行棋规则不允许走的位置
    switch (kind) {
    case Piece::Kind::KING:
    case Piece::Kind::ADVISOR:
    case Piece::Kind::PAWN:
        break;
    case Piece::Kind::BISHOP:
        while (seatIter.hasNext()) {
            auto& toSeat = seatIter.next();
            if (getPiece({ (row + toSeat.first) / 2,
                    (col + toSeat.second) / 2 }))
                seatIter.remove();
        }
        break;
    case Piece::Kind::KNIGHT:
        while (seatIter.hasNext()) {
            auto& toSeat = seatIter.next();
            if (getPiece({ row + (toSeat.first - row) / 2,
                    col + (toSeat.second - col) / 2 }))
                seatIter.remove();
        }
        break;
    case Piece::Kind::ROOK: {
        // 后前左右按顺序排列的四个方向子序列 [..-row..|..+row..|..-col..|..+col..]
        int curDirection { 0 };
        QVector<bool> stop(4, false), skiped(4, false);
        while (seatIter.hasNext()) {
            auto& toSeat = seatIter.next();
            int toRow { toSeat.first }, toCol { toSeat.second };
            curDirection = (col == toCol
                    ? (toRow < row ? 0 : 1)
                    : (toCol < col ? 2 : 3));

            if (stop[curDirection])
                seatIter.remove();
            else if (getPiece(toSeat))
                stop[curDirection] = true;
        }
    } break;
    case Piece::Kind::CANNON: {
        // 后前左右按顺序排列的四个方向子序列 [..-row..|..+row..|..-col..|..+col..]
        int curDirection { 0 };
        QVector<bool> stop(4, false), skiped(4, false);
        while (seatIter.hasNext()) {
            auto& toSeat = seatIter.next();
            int toRow { toSeat.first }, toCol { toSeat.second };
            curDirection = (col == toCol
                    ? (toRow < row ? 0 : 1)
                    : (toCol < col ? 2 : 3));

            if (stop[curDirection])
                seatIter.remove();
            else {
                if (getPiece(toSeat)) {
                    if (!skiped[curDirection]) { // 炮在未跳状态
                        skiped[curDirection] = true;
                        seatIter.remove();
                    } else // 炮在已跳状态
                        stop[curDirection] = true;
                } else if (skiped[curDirection])
                    seatIter.remove();
            }
        }
    } break;
    default:
        break;
    }

    seatIter = QMutableListIterator<Seat>(seatList);
    while (seatIter.hasNext()) {
        auto toPiece = getPiece(seatIter.next());
        if (toPiece && toPiece->color() == color)
            seatIter.remove();
    }

    return seatList;
}

Seat Board::getKingSeat_(Piece::Color color) const
{
    auto kpie = pieces_.getColorKindPiece(color, Piece::Kind::KING).at(0);
    for (auto seatSide : { SeatManager::Seatside::HERE, SeatManager::Seatside::THERE })
        for (auto& seat : kpie->put(seatSide))
            if (getPiece(seat) == kpie)
                return seat;

    return { -1, -1 };
}

QList<Seat> Board::getLiveSeatList_(Piece::Color color, QChar name) const
{
    QList<Seat> seatList;
    for (auto& seatPiece : getColorSeatPieceList(color))
        if (name == seatPiece.second->name())
            seatList.append(seatPiece.first);

    return seatList;
}

QList<Seat> Board::getLiveSeatList_(Piece::Color color, QChar name, int col) const
{
    QList<Seat> seatList { getLiveSeatList_(color, name) };
    QMutableListIterator<Seat> seatIter(seatList);
    while (seatIter.hasNext())
        if (seatIter.next().second != col)
            seatIter.remove();

    return seatList;
}

QList<Seat> Board::getLiveSeatList_(Piece::Color color, QChar name, int col, bool getStronge) const
{
    QList<Seat> seatList { getLiveSeatList_(color, name, col) };
    if (!getStronge)
        return seatList;

    QMutableListIterator<Seat> seatIter(seatList);
    while (seatIter.hasNext())
        if (!PieceManager::isStronge(getPiece(seatIter.next())->name()))
            seatIter.remove();

    return seatList;
}

QList<Seat> Board::getSortPawnLiveSeats_(bool isBottom, Piece::Color color, QChar name) const
{
    // 最多5个兵, 按列建立字典，按列排序
    QList<Seat> pawnSeats { getLiveSeatList_(color, name) }, seats {};
    QMap<int, QList<Seat>> colSeats {};

    for (auto& seat : pawnSeats)
        // 底边则列倒序,每列位置顺序
        colSeats[isBottom ? -seat.second : seat.second].append(seat);

    // 整合成一个数组
    for (auto col : colSeats.keys())
        if (colSeats[col].size() > 1) //  选取多于一个位置的列
            for (auto& seat : colSeats[col])
                seats.prepend(seat); // 按列存入
    return seats;
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
        if (getPiece({ r, col }))
            return false;

    return true;
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
    assert(ch == PieceManager::nullChar() || PieceManager::getChChars().contains(ch));
    if (ch == PieceManager::nullChar())
        return nullptr;

    QList<PPiece> ckpies = pieces_.getColorKindPiece(Piece::getColor(ch), Piece::getKind(ch));
    for (int i = 0; i < ckpies.count(); ++i) {
        auto piece = ckpies.at(i);
        if (!inSeat_(piece))
            return piece;
    }

    return nullptr;
}

void Board::setBottomColor_()
{
    bottomColor_ = (getKingSeat_(Piece::Color::RED).first < SEATROW / 2
            ? Piece::Color::RED
            : Piece::Color::BLACK);
}

bool Board::setFromSeatPieceList_(const QList<SeatPiece>& seatPieceList)
{
    assert(seatPieceList.count() == SEATNUM);
    if (seatPieceList.count() != SEATNUM)
        return false;

    for (int i = 0; i < SEATNUM; ++i)
        setPiece(seatPieceList.at(i));

    setBottomColor_();
    return true;
}

QString Board::getPieChars_(const QList<SeatPiece>& seatPieceList) const
{
    if (seatPieceList.count() != SEATNUM)
        return QString();

    QString qstr;
    for (int i = 0; i < SEATNUM; ++i) {
        auto piece = seatPieceList.at(i).second;
        qstr.append(piece ? piece->ch() : PieceManager::nullChar());
    }

    return qstr;
}

QList<SeatPiece> Board::getSeatPieceList_pieChars_(const QString& pieChars) const
{
    QList<SeatPiece> seatPieceList;
    assert(pieChars.count() == SEATNUM);
    if (pieChars.count() != SEATNUM)
        return seatPieceList;

    int index = 0;
    for (int r = 0; r < SEATROW; ++r)
        for (int c = 0; c < SEATCOL; ++c)
            seatPieceList.append({ { r, c }, getUnUsedPiece_(pieChars[index++]) });

    return seatPieceList;
}
