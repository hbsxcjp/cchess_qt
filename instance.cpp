#include "instance.h"
#include "board.h"
#include "piece.h"
#include "tools.h"
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QTextCodec>
#include <cassert>
#include <cmath>
#include <functional>

Record::Record(int weight, bool killing, bool willKill, bool isCatch, bool fail)
    : count { 1 }
    , weight { weight }
    , killing { killing }
    , willKill { willKill }
    , isCatch { isCatch }
    , isFail { fail }
{
}

QDataStream& operator<<(QDataStream& out, const Record& record)
{
    out << record.count << record.weight << record.killing
        << record.willKill << record.isCatch << record.isFail;
    return out;
}

QDataStream& operator>>(QDataStream& in, Record& record)
{
    in >> record.count >> record.weight >> record.killing
        >> record.willKill >> record.isCatch >> record.isFail;
    return in;
}

MoveRec::MoveRec(const QString& fen, Piece::Color color, int rowcols, const Record& record)
    : fen { fen }
    , color { color }
    , rowcols { rowcols }
    , record { record }
{
}

Instance::Instance()
    : board_ { new Board }
    , rootMove_ { new Move }
    , curMove_ { rootMove_ }
    , info_ { getInitInfoMap() }
{
    board_->setFEN(fen__());
}

Instance::Instance(const QString& fileName)
    : Instance()
{
    if (!fileName.isEmpty())
        read__(fileName);
}

Instance::~Instance()
{
    delMove__(rootMove_); // 驱动函数
    delete board_;
}

bool Instance::write(const QString& fileName) const
{
    RecFormat fmt = getRecFormat(QFileInfo(fileName).suffix());
    switch (fmt) {
    case RecFormat::NOTFMT:
        break;
    case RecFormat::XQF:
        break;
    case RecFormat::BIN:
        return writeBIN__(fileName);
    case RecFormat::JSON:
        return writeJSON__(fileName);
    default:
        return writePGN__(fileName, fmt);
    }
    return false;
}

Instance::PMove Instance::appendMove_rc(int frow, int fcol, int trow, int tcol, const QString& remark, bool isOther)
{
    return appendMove_seats__({ { frow, fcol }, { trow, tcol } }, remark, isOther);
}

Instance::PMove Instance::appendMove_iccszh(QString iccszhStr, RecFormat fmt, const QString& remark, bool isOther)
{
    if (fmt != RecFormat::PGN_ICCS) {
        if (isOther)
            undo__();
        auto movseat = board_->strToMovSeat(iccszhStr);
        assert(!SeatManager::movSeatIsInvalid(movseat));
        if (isOther)
            done__();
        return appendMove_seats__(movseat, remark, isOther);
    }

    int frow { PieceManager::getRowFromICCSChar(iccszhStr[1]) },
        fcol { PieceManager::getColFromICCSChar(iccszhStr[0]) },
        trow { PieceManager::getRowFromICCSChar(iccszhStr[3]) },
        tcol { PieceManager::getColFromICCSChar(iccszhStr[2]) };
    return appendMove_rc(frow, fcol, trow, tcol, remark, isOther);
}

Instance::PMove Instance::appendMove_zh_tolerateError(QString zhStr, bool isOther)
{
    if (isOther)
        undo__();
    //    QPair<PSeat, PSeat> seats = board_->fromToSeats(zhStr, true);
    auto movseat = board_->strToMovSeat(zhStr, true);
    if (isOther)
        done__();

    if (SeatManager::movSeatIsInvalid(movseat))
        return Q_NULLPTR;
    return appendMove_seats__(movseat, "", isOther);
}

bool Instance::go()
{
    if (curMove_->next_)
        return go__(curMove_->next_);

    return false;
}

bool Instance::goOther()
{
    if (curMove_->other_) {
        undo__();
        return go__(curMove_->other_);
    }
    return false;
}

int Instance::goEnd()
{
    int count = 0;
    while (curMove_->next_) {
        go__(curMove_->next_);
        ++count;
    }
    return count;
}

int Instance::goTo(Instance::PMove& move)
{
    assert(move);
    QList<PMove> preMoves { move->getPrevMoves() };
    for (auto& move : preMoves)
        go__(move);
    return preMoves.length();
}

bool Instance::back()
{
    if (curMove_->isOther())
        return backOther();
    else if (curMove_->prev_)
        return back__();

    return false;
}

bool Instance::backNext()
{
    if (curMove_->prev_ && !curMove_->isOther())
        return back__();

    return false;
}

bool Instance::backOther()
{
    if (curMove_->isOther()) {
        back__(); // 变着回退
        done__(); // 前变执行
        return true;
    }
    return false;
}

int Instance::backToPre()
{
    int count = 0;
    while (curMove_->isOther()) {
        backOther();
        ++count;
    }
    backNext();
    return ++count;
}

int Instance::backStart()
{
    int count = 0;
    while (curMove_->prev_) {
        back();
        ++count;
    }
    return count;
}

int Instance::backTo(Instance::PMove& move)
{
    int count = 0;
    while (curMove_->prev_ && curMove_ != move) {
        back();
        ++count;
    }
    return count;
}

int Instance::goInc(int inc)
{
    int incCount { abs(inc) }, count { 0 };
    //std::function<void(Instance*)> fbward = inc > 0 ? &Instance::go : &Instance::back;
    auto fbward = std::mem_fn(inc > 0 ? &Instance::go : &Instance::back);
    while (incCount-- > 0)
        if (fbward(this))
            ++count;

    return count;
}

void Instance::changeSide(SeatManager::ChangeType ct)
{
    //    QList<PMove> prevMoves {};
    //    if (curMove_ != rootMove_)
    //        prevMoves = curMove_->getPrevMoves();

    backStart();
    board_->changeSide(ct);
    if (rootMove_->next_)
        rootMove_->next_->changeSide(board_, ct);

    PMove firstMove { rootMove_->next_ ? rootMove_->next_ : nullptr };
    setFEN__(board_->getFEN(), firstMove ? board_->getPiece(firstMove->movseatPiece_.first.first)->color() : Piece::Color::RED);

    //    for (auto& move : prevMoves)
    //        move->done();
    while (curMove_)
        done__();
}

const QString& Instance::remark() const { return rootMove_->remark_; }

const QString Instance::toString()
{
    QString qstr {};
    QTextStream stream(&qstr);
    backStart();
    writeInfo_PGN__(stream);
    stream << board_->toString(true);

    writeMove_PGN_CC__(stream);
    stream << moveInfo__() << '\n';
    return qstr;
}

const QString Instance::toFullString()
{
    QString qstr {};
    QTextStream stream(&qstr);
    stream << toString();

    std::function<void(const PMove&, bool)>
        __printMoveBoard = [&](const PMove& move, bool isOther) {
            stream << board_->toString() << move->toString() << "\n\n";

            isOther ? goOther() : go();
            if (move->next_)
                __printMoveBoard(move->next_, false);
            if (move->other_)
                __printMoveBoard(move->other_, true);

            isOther ? backOther() : backNext();
        };

    if (rootMove_->next_)
        __printMoveBoard(rootMove_->next_, false);
    return qstr;
}

const QString Instance::getExtName(const RecFormat fmt)
{
    switch (fmt) {
    case RecFormat::XQF:
        return ".xqf";
    case RecFormat::BIN:
        return ".bin";
    case RecFormat::JSON:
        return ".json";
    case RecFormat::PGN_ICCS:
        return ".pgn_iccs";
    case RecFormat::PGN_ZH:
        return ".pgn_zh";
    case RecFormat::PGN_CC:
        return ".pgn_cc";
    default:
        return QString {};
    }
}

RecFormat Instance::getRecFormat(const QString& ext_)
{
    QString ext { ext_.toLower() };
    if (ext == "xqf")
        return RecFormat::XQF;
    else if (ext == "bin")
        return RecFormat::BIN;
    else if (ext == "json")
        return RecFormat::JSON;
    else if (ext == "pgn_iccs")
        return RecFormat::PGN_ICCS;
    else if (ext == "pgn_zh")
        return RecFormat::PGN_ZH;
    else if (ext == "pgn_cc")
        return RecFormat::PGN_CC;
    else
        return RecFormat::NOTFMT;
}

InfoMap Instance::getInitInfoMap()
{
    return InfoMap {
        { "TITLE", "" }, { "EVENT", "" }, { "DATE", "" },
        { "SITE", "" }, { "BLACK", "" }, { "RED", "" },
        { "OPENING", "" }, { "WRITER", "" }, { "AUTHOR", "" },
        { "TYPE", "" }, { "RESULT", "" }, { "VERSION", "" },
        { "SOURCE", "" }, { "FEN", PieceManager::getFENStr() },
        { "ICCSSTR", "" }, { "ECCOSN", "" }, { "ECCONAME", "" }, { "MOVESTR", "" }
    };
}

void Instance::writeInfoMap(QTextStream& stream, const InfoMap& info)
{
    for (auto& key : info.keys())
        stream << '[' << key << " \"" << info[key] << "\"]\n";
    stream << '\n';
}

//int Instance::changeRowcols(int rowcols, SeatManager::ChangeType ct)
//{
//    if (ct == SeatManager::ChangeType::NOCHANGE)
//        return rowcols;

//    int frowcol { Move::getFRowcol(rowcols) }, trowcol { Move::getTRowcol(rowcols) };
//    return Move::getRowcols(SeatManager::getChangeRowcol(frowcol, ct), SeatManager::getChangeRowcol(trowcol, ct));
//}

QList<MoveRec> Instance::getMoveReces()
{
    QList<MoveRec> moveReces {};
    PMove curMove = curMove_;
    backStart();
    std::function<void(const PMove&)>
        __appendMoveRec = [&](const PMove& move) {
            // 待补充棋局状态变量的取值
            auto& movseat = move->movseatPiece_.first;
            moveReces.append(MoveRec(board_->getFEN(),
                board_->getPiece(movseat.first)->color(),
                SeatManager::rowcols(movseat),
                Record(0, false, false, false, false)));

            //            move->done();
            done__();
            if (move->next_)
                __appendMoveRec(move->next_);
            //            move->undo();
            undo__();

            if (move->other_)
                __appendMoveRec(move->other_);
        };

    if (rootMove_->next_)
        __appendMoveRec(rootMove_->next_); // 驱动函数
    goTo(curMove);

    return moveReces;
}

bool Instance::read__(const QString& fileName)
{
    RecFormat fmt = getRecFormat(QFileInfo(fileName).suffix());
    switch (fmt) {
    case RecFormat::NOTFMT:
        break;
    case RecFormat::XQF:
        return readXQF__(fileName);
    case RecFormat::BIN:
        return readBIN__(fileName);
    case RecFormat::JSON:
        return readJSON__(fileName);
    default:
        return readPGN__(fileName, fmt);
    }
    return false;
}

bool Instance::readXQF__(const QString& fileName)
{
    QFile file(fileName);
    if (!file.exists() || !(file.open(QIODevice::ReadOnly)))
        return false;

    QDataStream stream(&file);
    //stream.setByteOrder(QDataStream::LittleEndian);

    //文件标记'XQ'=$5158/版本/加密掩码/ProductId[4], 产品(厂商的产品号)
    char Signature[3] {}, Version {}, headKeyMask {}, ProductId[4] {},
        headKeyOrA {}, headKeyOrB {}, headKeyOrC {}, headKeyOrD {},
        // 加密的钥匙和/棋子布局位置钥匙/棋谱起点钥匙/棋谱终点钥匙
        headKeysSum {}, headKeyXY {}, headKeyXYf {}, headKeyXYt {},
        headQiziXY[PieceManager::pieceNum] {}, // 32个棋子的原始位置
        // 用单字节坐标表示, 将字节变为十进制, 十位数为X(0-8)个位数为Y(0-9),
        // 棋盘的左下角为原点(0, 0). 32个棋子的位置从1到32依次为:
        // 红: 车马相士帅士相马车炮炮兵兵兵兵兵 (位置从右到左, 从下到上)
        // 黑: 车马象士将士象马车炮炮卒卒卒卒卒 (位置从右到左, 从下到上)PlayStepNo[2],
        PlayStepNo[2] {},
        headWhoPlay {}, headPlayResult {}, PlayNodes[4] {}, PTreePos[4] {}, Reserved1[4] {},
        // 该谁下 0-红先, 1-黑先/最终结果 0-未知, 1-红胜 2-黑胜, 3-和棋
        headCodeA_H[16] {}, TitleA[65] {}, TitleB[65] {}, //对局类型(开,中,残等)
        Event[65] {}, Date[17] {}, Site[17] {}, Red[17] {}, Black[17] {},
        Opening[65] {}, Redtime[17] {}, Blktime[17] {}, Reservedh[33] {},
        RMKWriter[17] {}, Author[17] {}; //, Other[528]{}; // 棋谱评论员/文件的作者

    stream.readRawData(Signature, 2);
    stream.readRawData(&Version, 1);
    stream.readRawData(&headKeyMask, 1);
    stream.readRawData(ProductId, 4); // = 8 bytes
    stream.readRawData(&headKeyOrA, 1);
    stream.readRawData(&headKeyOrB, 1);
    stream.readRawData(&headKeyOrC, 1);
    stream.readRawData(&headKeyOrD, 1);
    stream.readRawData(&headKeysSum, 1);
    stream.readRawData(&headKeyXY, 1);
    stream.readRawData(&headKeyXYf, 1);
    stream.readRawData(&headKeyXYt, 1); // = 16 bytes
    stream.readRawData(headQiziXY, PieceManager::pieceNum); // = 48 bytes
    stream.readRawData(PlayStepNo, 2);
    stream.readRawData(&headWhoPlay, 1);
    stream.readRawData(&headPlayResult, 1);
    stream.readRawData(PlayNodes, 4);
    stream.readRawData(PTreePos, 4);
    stream.readRawData(Reserved1, 4); // = 64 bytes
    stream.readRawData(headCodeA_H, 16);
    stream.readRawData(TitleA, 64);
    stream.readRawData(TitleB, 64);
    stream.readRawData(Event, 64);
    stream.readRawData(Date, 16);
    stream.readRawData(Site, 16);
    stream.readRawData(Red, 16); // = 320 bytes
    stream.readRawData(Black, 16);
    stream.readRawData(Opening, 64);
    stream.readRawData(Redtime, 16);
    stream.readRawData(Blktime, 16);
    stream.readRawData(Reservedh, 32);
    stream.readRawData(RMKWriter, 16); // = 480 bytes
    stream.readRawData(Author, 16); // = 496 bytes

    assert(Signature[0] == 0x58 || Signature[1] == 0x51);
    // L" 检查密码校验和不对，不等于0。\n";
    assert((headKeysSum + headKeyXY + headKeyXYf + headKeyXYt) % 256 == 0);
    // L" 这是一个高版本的XQF文件，您需要更高版本的XQStudio来读取这个文件。\n";
    assert(Version <= 18);

    unsigned char KeyXY {}, KeyXYf {}, KeyXYt {}, F32Keys[PieceManager::pieceNum],
        *head_QiziXY { (unsigned char*)headQiziXY };
    int KeyRMKSize {};
    if (Version <= 10) { // version <= 10 兼容1.0以前的版本
        KeyRMKSize = KeyXYf = KeyXYt = 0;
    } else {
        std::function<unsigned char(unsigned char, unsigned char)>
            __calkey = [](unsigned char bKey, unsigned char cKey) {
                // % 256; // 保持为<256
                return (((((bKey * bKey) * 3 + 9) * 3 + 8) * 2 + 1) * 3 + 8) * cKey;
            };
        KeyXY = __calkey(headKeyXY, headKeyXY);
        KeyXYf = __calkey(headKeyXYf, KeyXY);
        KeyXYt = __calkey(headKeyXYt, KeyXYf);
        KeyRMKSize = (((static_cast<unsigned char>(headKeysSum) * 256 + static_cast<unsigned char>(headKeyXY)) % 32000) + 767); // % 65536
        if (Version >= 12) { // 棋子位置循环移动
            QList<unsigned char> Qixy(std::begin(headQiziXY), std::end(headQiziXY)); // 数组不能拷贝
            for (int i = 0; i != PieceManager::pieceNum; ++i)
                head_QiziXY[(i + KeyXY + 1) % PieceManager::pieceNum] = Qixy[i];
        }
        for (int i = 0; i != PieceManager::pieceNum; ++i)
            head_QiziXY[i] -= KeyXY; // 保持为8位无符号整数，<256
    }
    int KeyBytes[4] {
        (headKeysSum & headKeyMask) | headKeyOrA,
        (headKeyXY & headKeyMask) | headKeyOrB,
        (headKeyXYf & headKeyMask) | headKeyOrC,
        (headKeyXYt & headKeyMask) | headKeyOrD
    };
    const char copyright[] { "[(C) Copyright Mr. Dong Shiwei.]" };
    for (int i = 0; i != PieceManager::pieceNum; ++i)
        F32Keys[i] = copyright[i] & KeyBytes[i % 4]; // ord(c)

    // 取得棋子字符串
    QString pieceChars(90, PieceManager::nullChar());
    char pieChars[] { "RNBAKABNRCCPPPPPrnbakabnrccppppp" }; // QiziXY设定的棋子顺序
    for (int i = 0; i != PieceManager::pieceNum; ++i) {
        int xy = head_QiziXY[i];
        if (xy <= 89)
            // 用单字节坐标表示, 将字节变为十进制,  十位数为X(0-8),个位数为Y(0-9),棋盘的左下角为原点(0, 0)
            pieceChars[xy % 10 * 9 + xy / 10] = pieChars[i];
    }

    QTextCodec* codec = QTextCodec::codecForName("gbk");
    info_["VERSION"] = QString::number(Version);
    info_["RESULT"] = (QMap<unsigned char, QString> { { 0, "未知" }, { 1, "红胜" }, { 2, "黑胜" }, { 3, "和棋" } })[headPlayResult];
    info_["TYPE"] = (QMap<unsigned char, QString> { { 0, "全局" }, { 1, "开局" }, { 2, "中局" }, { 3, "残局" } })[headCodeA_H[0]];
    info_["TITLE"] = codec->toUnicode(TitleA);
    info_["EVENT"] = codec->toUnicode(Event);
    info_["DATE"] = codec->toUnicode(Date);
    info_["SITE"] = codec->toUnicode(Site);
    info_["RED"] = codec->toUnicode(Red);
    info_["BLACK"] = codec->toUnicode(Black);
    info_["OPENING"] = codec->toUnicode(Opening);
    info_["WRITER"] = codec->toUnicode(RMKWriter);
    info_["AUTHOR"] = codec->toUnicode(Author);
    info_["FEN"] = Board::pieCharsToFEN(pieceChars); // 可能存在不是红棋先走的情况？

    std::function<unsigned char(unsigned char, unsigned char)>
        __sub = [](unsigned char a, unsigned char b) {
            return a - b;
        }; // 保持为<256

    auto __readBytes = [&](char* bytes, int size) {
        int pos = file.pos();
        stream.readRawData(bytes, size);
        if (Version > 10) // '字节解密'
            for (int i = 0; i != size; ++i)
                bytes[i] = __sub(bytes[i], F32Keys[(pos + i) % 32]);
    };

    auto __getRemarksize = [&]() {
        char clen[4] {};
        __readBytes(clen, 4);
        return *(int*)clen - KeyRMKSize;
    };

    char data[4] {}, &frc { data[0] }, &trc { data[1] }, &tag { data[2] };
    std::function<QString()> __readDataAndGetRemark = [&]() {
        __readBytes(data, 4);
        int RemarkSize {};
        if (Version <= 10) {
            tag = ((tag & 0xF0) ? 0x80 : 0) | ((tag & 0x0F) ? 0x40 : 0);
            RemarkSize = __getRemarksize();
        } else {
            tag &= 0xE0;
            if (tag & 0x20)
                RemarkSize = __getRemarksize();
        }
        if (RemarkSize > 0) { // # 如果有注解
            char rem[2048] {};
            __readBytes(rem, RemarkSize);
            return codec->toUnicode(rem);
        } else
            return QString {};
    };

    std::function<void(bool)> __readMove = [&](bool isOther) {
        auto remark = __readDataAndGetRemark();
        //# 一步棋的起点和终点有简单的加密计算，读入时需要还原
        int fcolrow = __sub(frc, 0X18 + KeyXYf), tcolrow = __sub(trc, 0X20 + KeyXYt);
        assert(fcolrow <= 89 && tcolrow <= 89);

        int frow = fcolrow % 10, fcol = fcolrow / 10, trow = tcolrow % 10, tcol = tcolrow / 10;

        auto& movseat = curMove_->movseatPiece_.first;
        if (movseat == MovSeat({ { frow, fcol }, { trow, tcol } })) {
            if (!remark.isEmpty())
                curMove_->remark_ = remark;
        } else
            appendMove_rc(frow, fcol, trow, tcol, remark, isOther);

        char ntag { tag };
        if (ntag & 0x80) //# 有左子树
            __readMove(false);

        if (ntag & 0x40) // # 有右子树
            __readMove(true);

        isOther ? backOther() : backNext();
    };

    file.seek(1024);
    rootMove_->remark_ = __readDataAndGetRemark();
    board_->reinit();
    if (tag & 0x80) //# 有左子树
        __readMove(false);

    setMoveNums__();
    file.close();

    return true;
}

static const char FILETAG[] = "learnchess";

bool Instance::readBIN__(const QString& fileName)
{
    QFile file(fileName);
    if (!file.exists() || !(file.open(QIODevice::ReadOnly)))
        return false;

    QDataStream stream(&file);
    //stream.setByteOrder(QDataStream::LittleEndian);
    char* fileTag;
    uint len;
    stream.readBytes(fileTag, len);
    if (strcmp(fileTag, FILETAG) != 0) // 文件标志不对
        return false;

    std::function<void(bool)> __readMove = [&](bool isOther) {
        int rowcols;
        char tag;
        QString remark {};
        stream >> rowcols;
        stream.readRawData(&tag, 1);
        if (tag & 0x20)
            stream >> remark;
        appendMove_seats__(SeatManager::movseat(rowcols), remark, isOther);

        if (tag & 0x80)
            __readMove(false);

        if (tag & 0x40)
            __readMove(true);

        isOther ? backOther() : backNext();
    };

    char tag;
    stream.readRawData(&tag, 1);
    if (tag & 0x80) {
        char infoNum;
        stream.readRawData(&infoNum, 1);
        QString key {}, value {};
        for (int i = 0; i < infoNum; ++i) {
            stream >> key >> value;
            info_[key] = value;
        }
    }
    board_->reinit();

    if (tag & 0x40)
        stream >> rootMove_->remark_;
    if (tag & 0x20)
        __readMove(false);

    setMoveNums__();
    file.close();
    return true;
}

bool Instance::writeBIN__(const QString& fileName) const
{
    QFile file(fileName);
    if (!(file.open(QIODevice::WriteOnly)))
        return false;

    QDataStream stream(&file);
    //stream.setByteOrder(QDataStream::LittleEndian);
    stream.writeBytes(FILETAG, sizeof(FILETAG));

    std::function<void(const PMove&)> __writeMove = [&](const PMove& move) {
        char tag = ((move->next_ ? 0x80 : 0x00)
            | (move->other_ ? 0x40 : 0x00)
            | (!move->remark_.isEmpty() ? 0x20 : 0x00));
        int rowcols = SeatManager::rowcols(move->movseatPiece_.first);
        stream << rowcols;
        stream.writeRawData(&tag, 1);
        if (tag & 0x20)
            stream << move->remark_;
        if (tag & 0x80)
            __writeMove(move->next_);
        if (tag & 0x40)
            __writeMove(move->other_);
    };

    char tag = ((!info_.isEmpty() ? 0x80 : 0x00)
        | (!rootMove_->remark_.isEmpty() ? 0x40 : 0x00)
        | (rootMove_->next_ ? 0x20 : 0x00));
    stream.writeRawData(&tag, 1);
    if (tag & 0x80) {
        char infoNum = info_.size();
        stream.writeRawData(&infoNum, 1);
        for (auto& key : info_.keys())
            stream << key << info_[key];
    }
    if (tag & 0x40)
        stream << rootMove_->remark_;
    if (tag & 0x20)
        __writeMove(rootMove_->next_);
    file.close();
    return true;
}

bool Instance::readJSON__(const QString& fileName)
{
    QFile file(fileName);
    if (!file.exists() || !(file.open(QIODevice::ReadOnly)))
        return false;

    QByteArray byteArray = file.readAll();
    QJsonDocument document = QJsonDocument::fromJson(byteArray);
    if (document.isNull()) {
        file.close();
        return false;
    }

    QJsonObject jsonRoot { document.object() },
        jsonInfo = jsonRoot.find("info")->toObject(),
        jsonRootMove = jsonRoot.find("rootMove")->toObject();
    QJsonValue jsonRemark = jsonRoot.find("remark")->toObject();
    for (auto iter = jsonInfo.constBegin(); iter != jsonInfo.constEnd(); ++iter)
        info_[iter.key()] = iter.value().toString();
    board_->reinit();

    std::function<void(bool, QJsonObject)>
        __readMove = [&](bool isOther, QJsonObject item) {
            QJsonValue mitem { item.value("m") };
            if (!mitem.isUndefined()) {
                QString mvstr { mitem.toString() };
                int pos { mvstr.indexOf(' ') }, rowcols { mvstr.left(pos).toInt() };
                QString remark { mvstr.mid(pos + 1) };
                appendMove_seats__(SeatManager::movseat(rowcols), remark, isOther);
            }

            QJsonValue nitem { item.value("n") }, oitem { item.value("o") };
            if (!nitem.isUndefined())
                __readMove(false, nitem.toObject());

            if (!oitem.isUndefined())
                __readMove(true, oitem.toObject());

            isOther ? backOther() : backNext();
        };

    rootMove_->remark_ = jsonRemark.toString();
    __readMove(false, jsonRootMove);

    setMoveNums__();
    file.close();
    return true;
}

bool Instance::writeJSON__(const QString& fileName) const
{
    QFile file(fileName);
    if (!(file.open(QIODevice::WriteOnly)))
        return false;

    QJsonObject jsonRoot, jsonInfo;
    for (auto& key : info_.keys())
        jsonInfo.insert(key, info_[key]);
    jsonRoot.insert("info", jsonInfo);

    std::function<QJsonObject(const PMove&)>
        __getJsonMove = [&](const PMove& move) {
            QJsonObject item {};
            if (move != rootMove_) {
                QString mvstr;
                QTextStream(&mvstr) << SeatManager::rowcols(move->movseatPiece_.first) << ' ' << move->remark_;
                item.insert("m", mvstr);
            }

            if (move->next_)
                item.insert("n", __getJsonMove(move->next_));

            if (move->other_)
                item.insert("o", __getJsonMove(move->other_));

            return item;
        };

    jsonRoot.insert("remark", rootMove_->remark_);
    jsonRoot.insert("rootMove", __getJsonMove(rootMove_));

    QJsonDocument document;
    document.setObject(jsonRoot);
    file.write(document.toJson());
    return true;
}

bool Instance::readPGN__(const QString& fileName, RecFormat fmt)
{
    QFile file(fileName);
    if (!file.exists() || !(file.open(QIODevice::ReadOnly)))
        return false;

    QTextStream stream(&file);
    readInfo_PGN__(stream);
    if (fmt != RecFormat::PGN_CC)
        readMove_PGN_ICCSZH__(stream, fmt);
    else
        readMove_PGN_CC__(stream);

    setMoveNums__();
    file.close();
    return true;
}

bool Instance::writePGN__(const QString& fileName, RecFormat fmt) const
{
    QFile file(fileName);
    if (!(file.open(QIODevice::WriteOnly)))
        return false;

    QTextStream stream(&file);
    writeInfo_PGN__(stream);
    if (fmt != RecFormat::PGN_CC)
        writeMove_PGN_ICCSZH__(stream, fmt);
    else
        writeMove_PGN_CC__(stream);

    file.close();
    return true;
}

Instance::PMove Instance::appendMove_seats__(const MovSeat& movseat, const QString& remark, bool isOther)
{
    //    assert(fseat && tseat);
    if (isOther) {
        undo__();
        /*/ 当前着法方与待添加着法方不相同，则不应该是变着
        if (fseat->piece()->color() != curMove_->fseat_->piece()->color()) {
            Tools::writeTxtFile("test.txt",
                curMove_->toString() + board_->toString() + fseat->toString() + tseat->toString() + (isOther ? " isOther.\n\n" : "\n\n"),
                QIODevice::Append);
            isOther = false;
            curMove_->done();
        }
        //assert(fseat->piece()->color() == curMove_->fseat_->piece()->color());
        //*/
    }
    bool can { board_->isCanMove(movseat.first, movseat.second) };
    QString zhStr {};
    if (can)
        zhStr = board_->movSeatToStr(movseat);
    /*/ 观察棋盘局面与着法
    Tools::writeTxtFile("test.txt",
        curMove_->toString() + board_->toString() + fseat->toString() + tseat->toString() + zhStr + (isOther ? " isOther.\n\n" : "\n\n"),
        QIODevice::Append);
    //*/

    if (isOther)
        done__();
    if (!can) {
        /*
        Tools::writeTxtFile("test.txt",
            curMove_->toString() + board_->toString() + fseat->toString() + tseat->toString() + (isOther ? " isOther.\n\n" : "\n\n"),
            QIODevice::Append);
        qDebug("appendMove_seats__ ?");
        //*/
        return Q_NULLPTR; //PMove {}
    }

    PMove move = curMove_->appendMove(movseat, zhStr, remark, isOther);
    isOther ? goOther() : go();
    return move;
}

void Instance::readInfo_PGN__(QTextStream& stream)
{
    QString line {};
    QRegularExpression infoReg(R"(\[(\w+)\s+\"([\s\S]*?)\"\])", QRegularExpression::UseUnicodePropertiesOption);
    QRegularExpressionMatch match;
    while (!(line = stream.readLine()).isEmpty()) { // 以空行为终止特征
        match = infoReg.match(line);
        if (match.hasMatch())
            info_[match.captured(1)] = match.captured(2);
    }
    board_->reinit();
}

void Instance::readMove_PGN_ICCSZH__(QTextStream& stream, RecFormat fmt)
{
    QString moveStr { stream.readAll() };
    bool isPGN_ZH { fmt == RecFormat::PGN_ZH };
    QString otherBeginStr { R"((\()?)" }; // 1:( 变着起始标志
    QString boutStr { R"((\d+\.)?[\s.]*\b)" }; // 2: 回合着法起始标志
    QString ICCSZhStr { R"(([)"
        + (isPGN_ZH ? PieceManager::getZhChars() : PieceManager::getICCSChars())
        + R"(]{4})\b)" }; // 3: 回合着法
    QString remarkStr { R"((?:\s*\{([\s\S]*?)\})?)" }; // 4: 注解
    QString otherEndStr { R"(\s*(\)+)?)" }; // 5:) 变着结束标志，可能存在多个右括号
    QString movePat { otherBeginStr + boutStr + ICCSZhStr + remarkStr + otherEndStr };
    QRegularExpression moveReg(movePat, QRegularExpression::UseUnicodePropertiesOption),
        remReg(remarkStr + R"(1\.)", QRegularExpression::UseUnicodePropertiesOption);

    int index = 0;
    QRegularExpressionMatch match = remReg.match(moveStr);
    if (match.hasMatch()) {
        rootMove_->remark_ = match.captured(1);
        index = match.capturedEnd();
    }

    QList<PMove> preOtherMoves {};
    auto matchIter = moveReg.globalMatch(moveStr, index);
    while (matchIter.hasNext()) {
        match = matchIter.next();
        bool isOther = !match.captured(1).isEmpty();
        if (isOther)
            preOtherMoves.append(curMove_);

        QString iccszhStr { match.captured(3) }, remark { match.captured(4) };
        appendMove_iccszh(iccszhStr, fmt, remark, isOther);
        //appendMove_iccszh(match.captured(3), fmt, match.captured(4), isOther);
        int num = match.captured(5).length();
        while (num-- && !preOtherMoves.isEmpty()) {
            auto otherMove = preOtherMoves.takeLast();
            backTo(otherMove);
        }
    }
    backStart();
}

void Instance::writeInfo_PGN__(QTextStream& stream) const
{
    writeInfoMap(stream, info_);
}

void Instance::writeMove_PGN_ICCSZH__(QTextStream& stream, RecFormat fmt) const
{
    bool isPGN_ZH { fmt == RecFormat::PGN_ZH };
    auto __getRemarkStr = [&](const PMove& move) {
        return (move->remark_.isEmpty()) ? "" : (" \n{" + move->remark_ + "}\n ");
    };

    std::function<void(const PMove&, bool)>
        __writeMove = [&](const PMove& move, bool isOther) {
            QString boutStr { QString::number((move->nextNo_ + 1) / 2) + ". " };
            bool isEven { move->nextNo_ % 2 == 0 };
            stream << (isOther ? "(" + boutStr + (isEven ? "... " : "")
                               : (isEven ? " " : boutStr))
                   << (isPGN_ZH ? move->zhStr_ : move->iccs()) << ' '
                   << __getRemarkStr(move);

            if (move->other_) {
                __writeMove(move->other_, true);
                stream << ")";
            }
            if (move->next_)
                __writeMove(move->next_, false);
        };

    stream << __getRemarkStr(rootMove_);
    if (rootMove_->next_)
        __writeMove(rootMove_->next_, false);
}

QString Instance::remarkNo__(int nextNo, int colNo) const
{
    return QString::asprintf("(%d,%d)", nextNo, colNo);
}

void Instance::readMove_PGN_CC__(QTextStream& stream)
{
    QString move_remStr { stream.readAll() };
    auto pos0 = move_remStr.indexOf("\n(");
    QString moveStr { move_remStr.left(pos0) },
        remStr { move_remStr.mid(pos0) };
    QRegularExpression movReg(R"([^…　]{4}[…　])", QRegularExpression::UseUnicodePropertiesOption),
        remReg(R"(\s*(\(\d+,\d+\)): \{([\s\S]*?)\})", QRegularExpression::UseUnicodePropertiesOption);

    InfoMap rems {};
    QRegularExpressionMatch match;
    auto matchIter = remReg.globalMatch(remStr);
    while (matchIter.hasNext()) {
        match = matchIter.next();
        //int offset = 0;
        //while (offset < remStr.length()) {
        //  match = remReg.match(remStr, offset);
        // if (!match.hasMatch())
        //   break;

        //offset = match.capturedEnd(); // 前进到下一个偏移位置
        rems[match.captured(1)] = match.captured(2);
    }

    // 读取每行列表
    QList<QStringList> moveLines {};
    //offset = 0;
    for (auto& line : moveStr.split('\n')) {
        if (line.indexOf(L'↓') != -1)
            continue;

        // 读取每行的着法列表
        int offs = 0;
        QStringList lineList {};
        while (offs < line.length() - 4) {
            lineList.append(line.mid(offs, 5));
            offs += 5;
        }
        moveLines.append(lineList);
    }

    std::function<void(bool, int, int)> __readMove = [&](bool isOther, int row, int col) {
        if (moveLines[row][col][0] == L'　')
            return;

        while (moveLines[row][col][0] == L'…')
            ++col;

        QString zhStr { moveLines[row][col] };
        match = movReg.match(zhStr);
        if (!match.hasMatch())
            return;

        QString qstr = zhStr.left(4), remark { rems[remarkNo__(row, col)] };
        appendMove_iccszh(qstr, RecFormat::PGN_CC, remark, isOther);

        if (zhStr.back() == L'…')
            __readMove(true, row, col + 1);

        if (row < moveLines.size() - 1)
            __readMove(false, row + 1, col);

        isOther ? backOther() : backNext();
    };

    rootMove_->remark_ = rems[remarkNo__(0, 0)];
    if (!moveLines.isEmpty())
        __readMove(false, 1, 0);
}

void Instance::writeMove_PGN_CC__(QTextStream& stream) const
{
    QString blankStr((getMaxCol() + 1) * 5, L'　');
    QVector<QString> lineStr((getMaxRow() + 1) * 2, blankStr);
    std::function<void(const PMove&)>
        __setMovePGN_CC = [&](const PMove& move) {
            int firstcol { move->CC_ColNo_ * 5 }, row { move->nextNo_ * 2 };
            lineStr[row].replace(firstcol, 4, move->zhStr_);

            if (move->next_) {
                lineStr[row + 1][firstcol + 2] = L'↓';
                __setMovePGN_CC(move->next_);
            }
            if (move->other_) {
                int fcol { firstcol + 4 }, num { move->other_->CC_ColNo_ * 5 - fcol };
                lineStr[row].replace(fcol, num, QString(num, L'…'));
                __setMovePGN_CC(move->other_);
            }
        };

    lineStr.front().replace(0, 3, "　开始");
    lineStr[1][2] = L'↓';
    if (rootMove_->next_)
        __setMovePGN_CC(rootMove_->next_);
    for (auto& line : lineStr)
        stream << line << '\n';

    std::function<void(const PMove&)>
        __setRemarkPGN_CC = [&](const PMove& move) {
            if (!move->remark_.isEmpty())
                stream << remarkNo__(move->nextNo_, move->CC_ColNo_) << ": {"
                       << move->remark_ << "}\n";

            if (move->next_)
                __setRemarkPGN_CC(move->next_);
            if (move->other_)
                __setRemarkPGN_CC(move->other_);
        };
    __setRemarkPGN_CC(rootMove_);
}

void Instance::setMoveNums__()
{
    curMove_ = rootMove_;
    std::function<void(const PMove&)>
        __setNums = [&](const PMove& move) {
            ++movCount_;
            maxCol_ = std::max(maxCol_, move->otherNo_);
            maxRow_ = std::max(maxRow_, move->nextNo_);
            move->CC_ColNo_ = maxCol_; // # 本着在视图中的列数
            if (!move->remark_.isEmpty()) {
                ++remCount_;
                remLenMax_ = std::max(remLenMax_, move->remark_.length());
            }

            //            move->done();
            if (move->next_)
                __setNums(move->next_);
            //            move->undo();

            if (move->other_) {
                ++maxCol_;
                __setNums(move->other_);
            }
        };

    movCount_ = remCount_ = remLenMax_ = maxRow_ = maxCol_ = 0;
    if (rootMove_->next_)
        __setNums(rootMove_->next_); // 驱动函数
}

bool Instance::done__() const
{
    if (!curMove_ || curMove_ == rootMove_)
        return false;

    auto& movseat = curMove_->movseatPiece_.first;
    curMove_->movseatPiece_.second = board_->movePiece(movseat);
    board_->setPiece({ movseat.first, nullptr });
    return true;
}

bool Instance::undo__() const
{
    if (!curMove_ || curMove_ == rootMove_)
        return false;

    auto& movseat = curMove_->movseatPiece_.first;
    board_->movePiece({ movseat.second, movseat.first });
    board_->setPiece({ movseat.second, curMove_->movseatPiece_.second });
    return true;
}

bool Instance::go__(PMove& curMove)
{
    //curMove->done();
    curMove_ = curMove;
    return done__();
    //    return true;
}

bool Instance::back__()
{
    //    curMove_->undo();
    curMove_ = curMove_->prev_;
    return undo__();
    //    return true;
}

void Instance::delMove__(Instance::PMove& move)
{
    PMove nmove = move->next_,
          omove = move->other_;
    delete move;

    if (nmove)
        delMove__(nmove);

    if (omove)
        delMove__(omove);
}

void Instance::setFEN__(const QString& fen, Piece::Color color)
{
    info_["FEN"] = (fen + " "
        + (color == Piece::Color::RED ? "r" : "b") + " - - 0 1");
}

const QString Instance::fen__() const
{
    return info_["FEN"].left(info_["FEN"].indexOf(' '));
}

const QString Instance::moveInfo__() const
{
    return QString::asprintf(
        "【着法深度：%d, 视图宽度：%d, 着法数量：%d, 注解数量：%d, 注解最长：%d】\n",
        maxRow_, maxCol_, movCount_, remCount_, remLenMax_);
}

QString Instance::Move::iccs() const
{
    Seat fseat { movseatPiece_.first.first }, tseat { movseatPiece_.first.second };
    return ((fseat != tseat)
            ? (PieceManager::getColICCSChar(fseat.second) + QString::number(fseat.first)
                + PieceManager::getColICCSChar(tseat.second) + QString::number(tseat.first))
            : QString {});
}

Instance::PMove Instance::Move::appendMove(const MovSeat& movseat, const QString& zhStr, const QString& remark, bool isOther)
{
    PMove pmove = new Move;
    pmove->prev_ = this;
    movseatPiece_.first = movseat;
    pmove->zhStr_ = zhStr;
    pmove->remark_ = remark;
    if (isOther) {
        pmove->nextNo_ = nextNo_;
        pmove->otherNo_ = otherNo_ + 1;
        other_ = pmove;
    } else {
        pmove->nextNo_ = nextNo_ + 1;
        pmove->otherNo_ = otherNo_;
        next_ = pmove;
    }
    return pmove;
}

void Instance::Move::changeSide(PBoard& board, SeatManager::ChangeType ct)
{
    auto& movseat = movseatPiece_.first;
    SeatManager::changeSeat(movseat.first, ct);
    SeatManager::changeSeat(movseat.second, ct);
    zhStr_ = board->movSeatToStr(movseat);

    //    done();
    if (next_)
        next_->changeSide(board, ct);
    //    undo();

    if (other_)
        other_->changeSide(board, ct);
}

bool Instance::Move::isOther()
{
    return this->prev_ && this->prev_->other_ == this;
}

Instance::PMove Instance::Move::getPrevMove()
{
    PMove move { this };
    while (move->prev_ && (move->prev_->other_ == move))
        move = move->prev_;
    return move->prev_;
}

QList<Instance::PMove> Instance::Move::getPrevMoves()
{
    QList<PMove> moves {};
    if (this->prev_) {
        PMove move { this };
        moves.append(move);
        while ((move = move->getPrevMove()))
            moves.prepend(move);
    }

    return moves;
}

QString Instance::Move::toString() const
{
    QString qstr {};
    auto& movseat = movseatPiece_.first;
    Seat fseat { movseat.first }, tseat { movseat.second };
    if (fseat != tseat) {
        QTextStream stream(&qstr);
        stream << SeatManager::printSeat(fseat) << '_' << SeatManager::printSeat(fseat)
               << '-' << iccs() << ':' << zhStr_ << '{' << remark_ << "}\n";
    }
    return qstr;
}

struct OperateDirData {
    int fcount {}, dcount {}, movCount {}, remCount {}, remLenMax {};
    QString dirName;
    RecFormat fromfmt, tofmt;
};

static void transFile__(const QString& fileName, void* odata)
{
    if (!QFileInfo::exists(fileName) || Instance::getRecFormat(QFileInfo(fileName).suffix()) == RecFormat::NOTFMT)
        return;

    Tools::writeTxtFile("test.txt", fileName + '\n', QIODevice::Append);
    Instance ins(fileName);

    OperateDirData* data { (OperateDirData*)odata };
    QString toFileName { fileName };
    toFileName.replace(Instance::getExtName(data->fromfmt), Instance::getExtName(data->tofmt), Qt::CaseInsensitive); // 目录名和文件名的扩展名都替换
    QString toDirName { QFileInfo(toFileName).absolutePath() };
    QDir dir(toDirName);
    if (!dir.exists())
        dir.mkpath(toDirName);

    //Tools::writeTxtFile("test.txt", fileName + '\n' + toFileName + '\n', QIODevice::Append);
    ins.write(toFileName);

    if (data->dirName != toDirName) {
        data->dirName = toDirName;
        ++data->dcount;
    }
    ++data->fcount;
    data->movCount += ins.getMovCount();
    data->remCount += ins.getRemCount();
    if (data->remLenMax < ins.getRemLenMax())
        data->remLenMax = ins.getRemLenMax();
}

void transDir(const QString& dirName, RecFormat fromfmt, RecFormat tofmt, bool isPrint)
{
    QDir fdir(dirName);
    QString fromDirName { fdir.absolutePath() }, toDirName { fromDirName };
    toDirName.replace(Instance::getExtName(fromfmt), Instance::getExtName(tofmt)); // 扩展名替换
    QDir tdir(toDirName);
    if (!tdir.exists())
        tdir.mkpath(toDirName);

    OperateDirData data {};
    data.dirName = toDirName;
    data.fromfmt = fromfmt;
    data.tofmt = tofmt;

    //Tools::writeTxtFile("test.txt", fromDirName + '\n', QIODevice::Append);
    Tools::operateDir(fromDirName, transFile__, &data, true);

    if (isPrint) {
        QString qstr {};
        QTextStream(&qstr) << dirName << " =>" << tdir.canonicalPath() << ": "
                           << data.fcount << " files, " << data.dcount << " dirs.\n   movCount: "
                           << data.movCount << ", remCount: " << data.remCount << ", remLenMax: " << data.remLenMax << "\n\n";
        Tools::writeTxtFile("test.txt", qstr, QIODevice::Append);
        qDebug() << qstr;
    }
}

static void testTransDir__(int fd, int td, int ff, int ft, int tf, int tt)
{
    QList<QString> dirfroms {
        "chessManual/示例文件",
        "chessManual/象棋杀着大全",
        "chessManual/疑难文件",
        "chessManual/中国象棋棋谱大全"
    };
    QList<RecFormat> fmts {
        RecFormat::XQF, RecFormat::BIN, RecFormat::JSON,
        RecFormat::PGN_ICCS, RecFormat::PGN_ZH, RecFormat::PGN_CC
    };

    //Tools::writeTxtFile("test.txt", "", QIODevice::WriteOnly);
    // 调节三个循环变量的初值、终值，控制转换目录
    for (int dir = fd; dir != td; ++dir)
        for (int fIndex = ff; fIndex != ft; ++fIndex)
            for (int tIndex = tf; tIndex != tt; ++tIndex)
                if (tIndex > 0 && tIndex != fIndex)
                    transDir(dirfroms[dir] + Instance::getExtName(fmts[fIndex]), fmts[fIndex], fmts[tIndex], true);
    //Tools::operateDir(dirfroms[dir] + getExtName(fmts[fIndex]), writeFileNames__, nullptr, true);
}

bool testInstance()
{
    Instance ins("01.xqf");
    ins.write("01.bin");
    QString qstr { ins.toString() }; //ins.toFullString()
    for (auto ct : { SeatManager::ChangeType::EXCHANGE, SeatManager::ChangeType::ROTATE, SeatManager::ChangeType::SYMMETRY }) {
        ins.changeSide(ct);
        qstr += ins.toString();
    }
    Tools::writeTxtFile("test.txt", qstr, QIODevice::WriteOnly);

    /*
    QStringList files { "01.bin", "01.json", "01.pgn_iccs", "01.pgn_zh", "01.pgn_cc" }; //"4.xqf"
    for (int i = 0; i < files.length(); ++i) { //
        Instance ins1(files[i]);
        ins1.write(files[i == files.length() - 1 ? i : i + 1]);
    }
    //*/

    // 参数: 起始目录，终止目录，来源起始格式，来源终止格式，目标起始格式，目标终止格式
    testTransDir__(0, 0, 0, 1, 1, 2);

    //testTransDir__(0, 2, 0, 1, 1, 6);
    //testTransDir__(0, 2, 0, 6, 1, 6);
    //testTransDir__(2, 3, 0, 1, 1, 2);

    return true;
}
