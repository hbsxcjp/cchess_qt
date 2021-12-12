#include "instance.h"
#include "aspect.h"
#include "board.h"
#include "move.h"
#include "piece.h"
#include "seat.h"
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

static const QStringList fileExtNames { "xqf", "bin", "json", "pgn_iccs", "pgn_zh", "pgn_cc" };

static const char FILETAG[] = "learnchess";

Instance::Instance()
    : board_(new Board())
    , rootMove_(new Move())
    , curMove_(rootMove_)
    , info_({ { "TITLE", "" }, { "EVENT", "" }, { "DATE", "" },
          { "SITE", "" }, { "BLACK", "" }, { "RED", "" },
          { "OPENING", "" }, { "WRITER", "" }, { "AUTHOR", "" },
          { "TYPE", "" }, { "RESULT", "" }, { "VERSION", "" },
          { "SOURCE", "" }, { "FEN", Pieces::FENStr }, { "ICCSSTR", "" },
          { "ECCOSN", "" }, { "ECCONAME", "" }, { "MOVESTR", "" } })
{
}

Instance::Instance(const QString& fileName)
    : Instance()
{
    if (fileName.isEmpty())
        return;

    SaveFormat fmt = getSaveFormat(QFileInfo(fileName).suffix());
    switch (fmt) {
    case SaveFormat::NOTFMT:
        break;
    case SaveFormat::XQF:
        readXQF__(fileName);
        break;
    case SaveFormat::BIN:
        readBIN__(fileName);
        break;
    case SaveFormat::JSON:
        readJSON__(fileName);
        break;
    default:
        readPGN__(fileName, fmt);
    }
}

Instance::~Instance()
{
    Move::deleteMove(rootMove_); // 驱动函数
    delete board_;
}

void Instance::write(const QString& fileName) const
{
    SaveFormat fmt = getSaveFormat(QFileInfo(fileName).suffix());
    switch (fmt) {
    case SaveFormat::NOTFMT:
        break;
    case SaveFormat::XQF:
        break;
    case SaveFormat::BIN:
        writeBIN__(fileName);
        break;
    case SaveFormat::JSON:
        writeJSON__(fileName);
        break;
    default:
        writePGN__(fileName, fmt);
    }
}

PMove Instance::appendMove(const MovSeat& movSeat, const QString& remark, bool isOther)
{
    if (isOther)
        curMove_->undo();
    Q_ASSERT(movSeat.first->getPiece());
    Q_ASSERT(board_->isCanMove(movSeat));
    QString zhStr { board_->getZhStr(movSeat) };

#ifdef DEBUG
    //*/ 观察棋盘局面与着法
    Tools::writeTxtFile("test.txt",
        (movSeat.first->toString() + movSeat.second->toString()
            + zhStr + (isOther ? " isOther.\n" : "\n")),
        QIODevice::Append);
    //*/
#endif

    if (isOther)
        curMove_->done();
    curMove_->addMove(movSeat, zhStr, remark, isOther);
    go(isOther);

#ifdef DEBUG
    // 观察棋盘局面与着法
    Tools::writeTxtFile("test.txt",
        (board_->toString() + curMove_->toString() + '\n'),
        QIODevice::Append);
#endif

    return curMove_;
}

PMove Instance::appendMove(int rowcols, const QString& remark, bool isOther)
{
    return appendMove(board_->getMovSeat(rowcols), remark, isOther);
}

PMove Instance::appendMove(QString iccszhStr, SaveFormat fmt, const QString& remark, bool isOther)
{
    if (fmt == SaveFormat::PGN_ICCS)
        return appendMove(board_->getMovSeat(
                              { { Pieces::getRowFromICCSChar(iccszhStr[1]),
                                    Pieces::getColFromICCSChar(iccszhStr[0]) },
                                  { Pieces::getRowFromICCSChar(iccszhStr[3]),
                                      Pieces::getColFromICCSChar(iccszhStr[2]) } }),
            remark, isOther);

    if (isOther)
        curMove_->undo();
    auto movseat = board_->getMovSeat(iccszhStr);
    if (isOther)
        curMove_->done();

    return appendMove(movseat, remark, isOther);
}

bool Instance::go(bool isOther)
{
    return isOther ? goOther() : goNext();
}

bool Instance::goNext()
{
    if (!curMove_->nextMove())
        return false;

    curMove_ = curMove_->nextMove();
    curMove_->done();
    return true;
}

bool Instance::goOther()
{
    if (!curMove_->otherMove())
        return false;

    curMove_->undo();
    curMove_ = curMove_->otherMove();
    curMove_->done();
    return true;
}

void Instance::goEnd()
{
    while (goNext())
        ;
}

void Instance::goTo(PMove move)
{
    Q_ASSERT(move);
    backStart();
    for (auto& move : move->getPrevMoveList())
        move->done();

    curMove_ = move;
}

bool Instance::back(bool isOther)
{
    return isOther ? backOther() : backNext();
}

// bool Instance::back()
//{
//     return curMove_->isOther() ? backOther() : backNext();
// }

bool Instance::backNext()
{
    if (!curMove_->preMove())
        return false;

    curMove_->undo();
    curMove_ = curMove_->preMove();
    return true;
}

bool Instance::backOther()
{
    if (!curMove_->isOther())
        return false;

    curMove_->undo(); // 变着回退
    curMove_ = curMove_->preMove();
    curMove_->done(); // 前变执行
    return true;
}

bool Instance::backToPre()
{
    while (backOther())
        ;

    return backNext();
}

void Instance::backStart()
{
    while (backToPre())
        ;
}

void Instance::backTo(PMove move)
{
    while (curMove_->preMove() && curMove_ != move)
        backToPre();
}

// void Instance::goInc(int inc)
//{
//     int incCount { abs(inc) };
//     // std::function<void(Instance*)> fbward = inc > 0 ? &Instance::go : &Instance::back;
//     auto fbward = std::mem_fn(inc > 0 ? &Instance::goNext : &Instance::back);
//     while (incCount-- && fbward(this))
//         ;
// }

void Instance::changeLayout(ChangeType ct)
{
    std::function<void(bool)>
        changeMove__ = [&](bool isOther) {
            PMove move;
            if (isOther) {
                curMove_->undo();
                move = curMove_->otherMove();
                move->changeLayout(board_, ct);
                curMove_->done();
            } else {
                move = curMove_->nextMove();
                move->changeLayout(board_, ct);
            }

            go(isOther);
            if (move->nextMove())
                changeMove__(false);

            if (move->otherMove())
                changeMove__(true);
            back(isOther);
        };

    PMove curMove { curMove_ };
    backStart();
    board_->changeLayout(ct);
    if (rootMove_->nextMove())
        changeMove__(false);

    backStart();
    PMove firstMove { rootMove_->nextMove() ? rootMove_->nextMove() : nullptr };
    setFEN__(board_->getFEN(), firstMove ? firstMove->color() : Color::RED);

    goTo(curMove);
}

const QString& Instance::remark() const { return rootMove_->remark_; }

const QString Instance::getExtName(const SaveFormat fmt)
{
    return fmt != SaveFormat::NOTFMT ? fileExtNames.at(int(fmt)) : "";
}

SaveFormat Instance::getSaveFormat(const QString& ext_)
{
    int index = fileExtNames.indexOf(ext_.toLower());
    return index >= 0 ? SaveFormat(index) : SaveFormat::NOTFMT;
}

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

            isOther ? goOther() : goNext();
            if (move->nextMove())
                __printMoveBoard(move->nextMove(), false);

            if (move->otherMove())
                __printMoveBoard(move->otherMove(), true);

            isOther ? backOther() : backNext();
        };

    if (rootMove_->nextMove())
        __printMoveBoard(rootMove_->nextMove(), false);

    return qstr;
}

QList<PAspect> Instance::getAspectList()
{
    QList<PAspect> aspectList {};
    PMove curMove = curMove_;

    backStart();
    std::function<void(const PMove&)>
        appendAspect__ = [&](const PMove& move) {
            // 待补充棋局状态变量的取值
            Color color = move->color();
            aspectList.append(new Aspect(board_->getFEN(),
                color,
                move->rowcols(),
                board_->getAspectStatus(color)));

            move->done();
            if (move->nextMove())
                appendAspect__(move->nextMove());
            move->undo();

            if (move->otherMove())
                appendAspect__(move->otherMove());
        };

    if (rootMove_->nextMove())
        appendAspect__(rootMove_->nextMove()); // 驱动函数

    goTo(curMove);
    return aspectList;
}

void Instance::readXQF__(const QString& fileName)
{
    QFile file(fileName);
    if (!file.exists() || !(file.open(QIODevice::ReadOnly)))
        return;

    QDataStream stream(&file);
    // stream.setByteOrder(QDataStream::LittleEndian);

    //文件标记'XQ'=$5158/版本/加密掩码/ProductId[4], 产品(厂商的产品号)
    char Signature[3] {}, Version {}, headKeyMask {}, ProductId[4] {},
        headKeyOrA {}, headKeyOrB {}, headKeyOrC {}, headKeyOrD {},
        // 加密的钥匙和/棋子布局位置钥匙/棋谱起点钥匙/棋谱终点钥匙
        headKeysSum {}, headKeyXY {}, headKeyXYf {}, headKeyXYt {},
        headQiziXY[PIECENUM] {}, // 32个棋子的原始位置
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
    stream.readRawData(headQiziXY, PIECENUM); // = 48 bytes
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

    unsigned char KeyXY {}, KeyXYf {}, KeyXYt {}, F32Keys[PIECENUM],
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
        KeyRMKSize = (((static_cast<unsigned char>(headKeysSum) * 256
                           + static_cast<unsigned char>(headKeyXY))
                          % 32000)
            + 767); // % 65536
        if (Version >= 12) { // 棋子位置循环移动
            QList<unsigned char> Qixy(std::begin(headQiziXY), std::end(headQiziXY)); // 数组不能拷贝
            for (int i = 0; i != PIECENUM; ++i)
                head_QiziXY[(i + KeyXY + 1) % PIECENUM] = Qixy[i];
        }
        for (int i = 0; i != PIECENUM; ++i)
            head_QiziXY[i] -= KeyXY; // 保持为8位无符号整数，<256
    }
    int KeyBytes[4] {
        (headKeysSum & headKeyMask) | headKeyOrA,
        (headKeyXY & headKeyMask) | headKeyOrB,
        (headKeyXYf & headKeyMask) | headKeyOrC,
        (headKeyXYt & headKeyMask) | headKeyOrD
    };
    const char copyright[] { "[(C) Copyright Mr. Dong Shiwei.]" };
    for (int i = 0; i != PIECENUM; ++i)
        F32Keys[i] = copyright[i] & KeyBytes[i % 4]; // ord(c)

    // 取得棋子字符串
    QString pieceChars(90, Pieces::nullChar);
    char pieChars[] { "RNBAKABNRCCPPPPPrnbakabnrccppppp" }; // QiziXY设定的棋子顺序
    for (int i = 0; i != PIECENUM; ++i) {
        int xy = head_QiziXY[i];
        if (xy <= 89)
            // 用单字节坐标表示, 将字节变为十进制,  十位数为X(0-8),个位数为Y(0-9),棋盘的左下角为原点(0, 0)
            pieceChars[xy % 10 * 9 + xy / 10] = pieChars[i];
    }

    QTextCodec* codec = QTextCodec::codecForName("gbk");
    info_["VERSION"] = QString::number(Version).simplified();
    info_["RESULT"] = (QMap<unsigned char, QString> { { 0, "未知" }, { 1, "红胜" }, { 2, "黑胜" }, { 3, "和棋" } })[headPlayResult];
    info_["TYPE"] = (QMap<unsigned char, QString> { { 0, "全局" }, { 1, "开局" }, { 2, "中局" }, { 3, "残局" } })[headCodeA_H[0]];
    info_["TITLE"] = codec->toUnicode(TitleA).simplified();
    info_["EVENT"] = codec->toUnicode(Event).simplified();
    info_["DATE"] = codec->toUnicode(Date).simplified();
    info_["SITE"] = codec->toUnicode(Site).simplified();
    info_["RED"] = codec->toUnicode(Red).simplified();
    info_["BLACK"] = codec->toUnicode(Black).simplified();
    info_["OPENING"] = codec->toUnicode(Opening).simplified();
    info_["WRITER"] = codec->toUnicode(RMKWriter).simplified();
    info_["AUTHOR"] = codec->toUnicode(Author).simplified();
    setFEN__(Seats::pieCharsToFEN(pieceChars), Color::RED); // 可能存在不是红棋先走的情况？

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
            return codec->toUnicode(rem).simplified();
        } else
            return QString {};
    };

    std::function<void(bool)> __readMove = [&](bool isOther) {
        auto remark = __readDataAndGetRemark();
        //# 一步棋的起点和终点有简单的加密计算，读入时需要还原
        int fcolrow = __sub(frc, 0X18 + KeyXYf), tcolrow = __sub(trc, 0X20 + KeyXYt);
        assert(fcolrow <= 89 && tcolrow <= 89);

        int frow = fcolrow % 10, fcol = fcolrow / 10, trow = tcolrow % 10, tcol = tcolrow / 10;

        MovSeat movSeat { board_->getMovSeat({ { frow, fcol }, { trow, tcol } }) };
        PMove move {};
        if (curMove_->movSeat() == movSeat) {
            if (!remark.isEmpty())
                curMove_->remark_ = remark;
        } else
            move = appendMove(movSeat, remark, isOther);

        char ntag { tag };
        if (ntag & 0x80) //# 有左子树
            __readMove(false);

        if (ntag & 0x40) // # 有右子树
            __readMove(true);

        if (move)
            back(isOther);
    };

    file.seek(1024);
    rootMove_->remark_ = __readDataAndGetRemark();
    setBoard__();
    if (tag & 0x80) //# 有左子树
        __readMove(false);

    setMoveNums__();
    file.close();
}

void Instance::readBIN__(const QString& fileName)
{
    QFile file(fileName);
    if (!file.exists() || !(file.open(QIODevice::ReadOnly)))
        return;

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    char* fileTag;
    uint len;
    stream.readBytes(fileTag, len);
    if (strcmp(fileTag, FILETAG) != 0) // 文件标志不对
        return;

    std::function<void(bool)> __readMove = [&](bool isOther) {
        int rowcols;
        char tag;
        QString remark {};
        stream >> rowcols;
        stream.readRawData(&tag, 1);
        if (tag & 0x20)
            stream >> remark;

        appendMove(rowcols, remark, isOther);

        if (tag & 0x80)
            __readMove(false);

        if (tag & 0x40)
            __readMove(true);

        back(isOther);
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
    setBoard__();

    if (tag & 0x40)
        stream >> rootMove_->remark_;
    if (tag & 0x20)
        __readMove(false);

    setMoveNums__();
    file.close();
}

void Instance::writeBIN__(const QString& fileName) const
{
    QFile file(fileName);
    if (!(file.open(QIODevice::WriteOnly)))
        return;

    QDataStream stream(&file);
    // stream.setByteOrder(QDataStream::LittleEndian);
    stream.writeBytes(FILETAG, sizeof(FILETAG));

    std::function<void(const PMove&)> __writeMove = [&](const PMove& move) {
        char tag = ((move->nextMove() ? 0x80 : 0x00)
            | (move->otherMove() ? 0x40 : 0x00)
            | (!move->remark_.isEmpty() ? 0x20 : 0x00));
        stream << move->rowcols();
        stream.writeRawData(&tag, 1);
        if (tag & 0x20)
            stream << move->remark_;
        if (tag & 0x80)
            __writeMove(move->nextMove());
        if (tag & 0x40)
            __writeMove(move->otherMove());
    };

    char tag = ((!info_.isEmpty() ? 0x80 : 0x00)
        | (!rootMove_->remark_.isEmpty() ? 0x40 : 0x00)
        | (rootMove_->nextMove() ? 0x20 : 0x00));
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
        __writeMove(rootMove_->nextMove());
    file.close();
}

void Instance::readJSON__(const QString& fileName)
{
    QFile file(fileName);
    if (!file.exists() || !(file.open(QIODevice::ReadOnly)))
        return;

    QByteArray byteArray = file.readAll();
    QJsonDocument document = QJsonDocument::fromJson(byteArray);
    if (document.isNull()) {
        file.close();
        return;
    }

    QJsonObject jsonRoot { document.object() },
        jsonInfo = jsonRoot.find("info")->toObject(),
        jsonRootMove = jsonRoot.find("rootMove")->toObject();
    QJsonValue jsonRemark = jsonRoot.find("remark")->toObject();
    for (auto iter = jsonInfo.constBegin(); iter != jsonInfo.constEnd(); ++iter)
        info_[iter.key()] = iter.value().toString();
    setBoard__();

    std::function<void(bool, QJsonObject)>
        __readMove = [&](bool isOther, QJsonObject item) {
            QJsonValue mitem { item.value("m") };
            PMove move;
            if (!mitem.isUndefined()) {
                QString mvstr { mitem.toString() };
                int pos { mvstr.indexOf(' ') }, rowcols { mvstr.left(pos).toInt() };
                QString remark { mvstr.mid(pos + 1) };

                move = appendMove(rowcols, remark, isOther);
            }

            QJsonValue nitem { item.value("n") }, oitem { item.value("o") };
            if (!nitem.isUndefined())
                __readMove(false, nitem.toObject());

            if (!oitem.isUndefined())
                __readMove(true, oitem.toObject());

            if (move)
                back(isOther);
        };

    rootMove_->remark_ = jsonRemark.toString();
    __readMove(false, jsonRootMove);

    setMoveNums__();
    file.close();
}

void Instance::writeJSON__(const QString& fileName) const
{
    QFile file(fileName);
    if (!(file.open(QIODevice::WriteOnly)))
        return;

    QJsonObject jsonRoot, jsonInfo;
    for (auto& key : info_.keys())
        jsonInfo.insert(key, info_[key]);
    jsonRoot.insert("info", jsonInfo);

    std::function<QJsonObject(const PMove&)>
        __getJsonMove = [&](const PMove& move) {
            QJsonObject item {};
            if (move != rootMove_) {
                QString mvstr;

                QTextStream(&mvstr) << move->rowcols() << ' ' << move->remark_;
                item.insert("m", mvstr);
            }

            if (move->nextMove())
                item.insert("n", __getJsonMove(move->nextMove()));

            if (move->otherMove())
                item.insert("o", __getJsonMove(move->otherMove()));

            return item;
        };

    jsonRoot.insert("remark", rootMove_->remark_);
    jsonRoot.insert("rootMove", __getJsonMove(rootMove_));

    QJsonDocument document;
    document.setObject(jsonRoot);
    file.write(document.toJson());
}

void Instance::readPGN__(const QString& fileName, SaveFormat fmt)
{
    QFile file(fileName);
    if (!file.exists() || !(file.open(QIODevice::ReadOnly)))
        return;

    QTextStream stream(&file);
    readInfo_PGN__(stream);
    if (fmt != SaveFormat::PGN_CC)
        readMove_PGN_ICCSZH__(stream, fmt);
    else
        readMove_PGN_CC__(stream);

    setMoveNums__();
    file.close();
}

void Instance::writePGN__(const QString& fileName, SaveFormat fmt) const
{
    QFile file(fileName);
    if (!(file.open(QIODevice::WriteOnly)))
        return;

    QTextStream stream(&file);
    writeInfo_PGN__(stream);
    if (fmt != SaveFormat::PGN_CC)
        writeMove_PGN_ICCSZH__(stream, fmt);
    else
        writeMove_PGN_CC__(stream);

    file.close();
}

void Instance::readInfo_PGN__(QTextStream& stream)
{
    QString qstr {}, line {};
    while (!(line = stream.readLine()).isEmpty()) // 以空行为终止特征
        qstr.append(line);

    QRegularExpression infoReg(R"(\[(\w+)\s+\"([\s\S]*?)\"\])",
        QRegularExpression::UseUnicodePropertiesOption);
    auto matchIter = infoReg.globalMatch(qstr);
    while (matchIter.hasNext()) {
        QRegularExpressionMatch match = matchIter.next();
        info_[match.captured(1)] = match.captured(2);
    }

    setBoard__();
}

void Instance::writeInfo_PGN__(QTextStream& stream) const
{
    for (auto& key : info_.keys())
        stream << '[' << key << " \"" << info_[key] << "\"]\n";
    stream << '\n';
}

void Instance::readMove_PGN_ICCSZH__(QTextStream& stream, SaveFormat fmt)
{
    QString moveStr { stream.readAll() };
    bool isPGN_ZH { fmt == SaveFormat::PGN_ZH };
    QString otherBeginStr { R"((\()?)" }; // 1:( 变着起始标志
    QString boutStr { R"((\d+\.)?[\s.]*\b)" }; // 2: 回合着法起始标志
    QString ICCSZhStr { R"(([)"
        + (isPGN_ZH ? board_->getZhChars() : Pieces::ICCS_RowChars + Pieces::ICCS_ColChars)
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

        appendMove(match.captured(3), fmt, match.captured(4), isOther);

        int num = match.captured(5).length();
        while (num-- && !preOtherMoves.isEmpty()) {
            auto otherMove = preOtherMoves.takeLast();
            backTo(otherMove);
        }
    }
    backStart();
}

void Instance::writeMove_PGN_ICCSZH__(QTextStream& stream, SaveFormat fmt) const
{
    bool isPGN_ZH { fmt == SaveFormat::PGN_ZH };
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

            if (move->otherMove()) {
                __writeMove(move->otherMove(), true);
                stream << ")";
            }
            if (move->nextMove())
                __writeMove(move->nextMove(), false);
        };

    stream << __getRemarkStr(rootMove_);
    if (rootMove_->nextMove())
        __writeMove(rootMove_->nextMove(), false);
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
        rems[match.captured(1)] = match.captured(2);
    }

    // 读取每行列表
    QList<QStringList> moveLines {};
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
        appendMove(qstr, SaveFormat::PGN_CC, remark, isOther);

        if (zhStr.back() == L'…')
            __readMove(true, row, col + 1);

        if (row < moveLines.size() - 1)
            __readMove(false, row + 1, col);

        back(isOther);
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

            if (move->nextMove()) {
                lineStr[row + 1][firstcol + 2] = L'↓';
                __setMovePGN_CC(move->nextMove());
            }
            if (move->otherMove()) {
                int fcol { firstcol + 4 }, num { move->otherMove()->CC_ColNo_ * 5 - fcol };
                lineStr[row].replace(fcol, num, QString(num, L'…'));
                __setMovePGN_CC(move->otherMove());
            }
        };

    lineStr.front().replace(0, 3, "　开始");
    lineStr[1][2] = L'↓';
    if (rootMove_->nextMove())
        __setMovePGN_CC(rootMove_->nextMove());
    for (auto& line : lineStr)
        stream << line << '\n';

    std::function<void(const PMove&)>
        __setRemarkPGN_CC = [&](const PMove& move) {
            if (!move->remark_.isEmpty())
                stream << remarkNo__(move->nextNo_, move->CC_ColNo_) << ": {"
                       << move->remark_ << "}\n";

            if (move->nextMove())
                __setRemarkPGN_CC(move->nextMove());
            if (move->otherMove())
                __setRemarkPGN_CC(move->otherMove());
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

            move->done();
            if (move->nextMove())
                __setNums(move->nextMove());
            move->undo();

            if (move->otherMove()) {
                ++maxCol_;
                __setNums(move->otherMove());
            }
        };

    movCount_ = remCount_ = remLenMax_ = maxRow_ = maxCol_ = 0;
    if (rootMove_->nextMove())
        __setNums(rootMove_->nextMove()); // 驱动函数
}

void Instance::setFEN__(const QString& fen, Color color)
{
    info_["FEN"] = (fen + " "
        + (color == Color::RED ? "r" : "b") + " - - 0 1");
}

const QString Instance::fen__() const
{
    return info_["FEN"].left(info_["FEN"].indexOf(' '));
}

void Instance::setBoard__()
{
    board_->setFEN(fen__());
}

const QString Instance::moveInfo__() const
{
    return QString::asprintf(
        "【着法深度：%d, 视图宽度：%d, 着法数量：%d, 注解数量：%d, 注解最长：%d】\n",
        maxRow_, maxCol_, movCount_, remCount_, remLenMax_);
}

struct OperateDirData {
    int fcount {}, dcount {}, movCount {}, remCount {}, remLenMax {};
    QString dirName;
    SaveFormat fromfmt, tofmt;
};

static void transFile__(const QString& fileName, void* odata)
{
    if (!QFileInfo::exists(fileName) || Instance::getSaveFormat(QFileInfo(fileName).suffix()) == SaveFormat::NOTFMT)
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

    // Tools::writeTxtFile("test.txt", fileName + '\n' + toFileName + '\n', QIODevice::Append);
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

void transDir(const QString& dirName, SaveFormat fromfmt, SaveFormat tofmt, bool isPrint)
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

    // Tools::writeTxtFile("test.txt", fromDirName + '\n', QIODevice::Append);
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
    QList<SaveFormat> fmts {
        SaveFormat::XQF, SaveFormat::BIN, SaveFormat::JSON,
        SaveFormat::PGN_ICCS, SaveFormat::PGN_ZH, SaveFormat::PGN_CC
    };

    // Tools::writeTxtFile("test.txt", "", QIODevice::WriteOnly);
    //  调节三个循环变量的初值、终值，控制转换目录
    for (int dir = fd; dir != td; ++dir)
        for (int fIndex = ff; fIndex != ft; ++fIndex)
            for (int tIndex = tf; tIndex != tt; ++tIndex)
                if (tIndex > 0 && tIndex != fIndex)
                    transDir(dirfroms[dir] + "." + Instance::getExtName(fmts[fIndex]), fmts[fIndex], fmts[tIndex], true);
    // Tools::operateDir(dirfroms[dir] + getExtName(fmts[fIndex]), writeFileNames__, nullptr, true);
}

bool testInstance()
{
    Instance ins("01.xqf");
    ins.write("01.bin");
    QString qstr { ins.toString() }; // ins.toFullString()
    for (auto ct : { ChangeType::EXCHANGE, ChangeType::ROTATE, ChangeType::SYMMETRY }) {
        ins.changeLayout(ct);
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

    // testTransDir__(0, 2, 0, 1, 1, 6);
    // testTransDir__(0, 2, 0, 6, 1, 6);
    // testTransDir__(2, 3, 0, 1, 1, 2);

    return true;
}
