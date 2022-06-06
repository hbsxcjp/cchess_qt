#include "manualIO.h"
#include "boardseats.h"
#include "manual.h"
#include "manualmove.h"
#include "manualmoveiterator.h"
#include "move.h"
#include "piece.h"
#include "piecebase.h"
#include "seat.h"
#include "seatbase.h"
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

const QString FILETAG_{"learnchess_instace\n"};

const QStringList INFONAME_{"TITLE",   "EVENT",   "DATE",          "SITE",
                            "BLACK",   "RED",     "OPENING",       "WRITER",
                            "AUTHOR",  "TYPE",    "RESULT",        "VERSION",
                            "SOURCE",  "FEN",     "ECCOSN",        "ECCONAME",
                            "MOVESTR", "ROWCOLS", "CALUATE_ECCOSN"};

const QStringList SUFFIXNAME_{"xqf",      "bin",    "json",
                              "pgn_iccs", "pgn_zh", "pgn_cc"};

QString ManualIO::getInfoName(InfoIndex nameIndex) {
  return INFONAME_.at(int(nameIndex));
}

const QStringList &ManualIO::getAllInfoName() { return INFONAME_; }

// InfoMap ChessManualIO::getInitInfoMap()
//{
//     return { { getInfoName(InfoIndex::FEN), PieceBase::FENSTR } };
// }

const QStringList &ManualIO::getSuffixNames() { return SUFFIXNAME_; }

QString ManualIO::getSuffixName(StoreType stroreType) {
  return SUFFIXNAME_.at(int(stroreType));
}

bool ManualIO::read(Manual *manual, const QString &fileName) {
  if (fileName.isEmpty())
    return false;

  ManualIO *manualIO = getManualIO_(fileName);
  if (!manualIO)
    return false;

  QFile file(fileName);
  if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
    file.close();
    return false;
  }

  bool succeeded = manualIO->read_(manual, file);
  file.close();
  delete manualIO;

  return succeeded;
}

bool ManualIO::read(Manual *manual, const InfoMap &infoMap,
                    StoreType storeType) {
  if (storeType != StoreType::PGN_ICCS && storeType != StoreType::PGN_ZH &&
      storeType != StoreType::PGN_CC)
    return false;

  // 构造pgn字符串
  QString pgnString;
  QTextStream stream(&pgnString);
  writeInfoToStream_(infoMap, stream);
  stream << infoMap.value(getInfoName(InfoIndex::MOVESTR)) << '\n';

  ManualIO *manualIO = getManualIO_(storeType);
  if (!manualIO)
    return false;

  bool succeeded =
      static_cast<ManualIO_pgn *>(manualIO)->readString(manual, pgnString);
  delete manualIO;

  return succeeded;
}

bool ManualIO::write(const Manual *manual, const QString &fileName) {
  if (fileName.isEmpty())
    return false;

  ManualIO *manualIO = getManualIO_(fileName);
  if (!manualIO)
    return false;

  QFile file(fileName);
  if (!(file.open(QIODevice::WriteOnly))) {
    file.close();
    return false;
  }

  bool succeeded = manualIO->write_(manual, file);
  file.close();
  delete manualIO;

  return succeeded;
}

QString ManualIO::getInfoString(const Manual *manual) {
  QString string;
  QTextStream stream(&string);
  writeInfo_(manual, stream);

  return string;
}

QString ManualIO::getMoveString(const Manual *manual, StoreType storeType) {
  QString string;
  QTextStream stream(&string);
  ManualIO *manualIO = getManualIO_(storeType);
  if (manualIO) {
    manualIO->writeMove_(manual, stream);
    delete manualIO;
  }

  return string;
}

QString ManualIO::getString(const Manual *manual, StoreType storeType) {
  return getInfoString(manual) + getMoveString(manual, storeType);
}

ManualIO *ManualIO::getManualIO_(StoreType storeType) {
  switch (storeType) {
  case StoreType::XQF:
    return new ManualIO_xqf;
  case StoreType::BIN:
    return new ManualIO_bin;
  case StoreType::JSON:
    return new ManualIO_json;
  case StoreType::PGN_ICCS:
    return new ManualIO_pgn_iccs;
  case StoreType::PGN_ZH:
    return new ManualIO_pgn_zh;
  case StoreType::PGN_CC:
    return new ManualIO_pgn_cc;
  default:
    break;
  }

  return Q_NULLPTR;
}

ManualIO *ManualIO::getManualIO_(const QString &fileName) {
  int index = SUFFIXNAME_.indexOf(QFileInfo(fileName).suffix().toLower());
  return getManualIO_(index < 0 ? StoreType::NOTSTORETYPE : StoreType(index));
}

void ManualIO::readInfo_(Manual *manual, QTextStream &stream) {
  QString qstr{}, line{};
  while (!(line = stream.readLine()).isEmpty()) // 以空行为终止特征
    qstr.append(line);

  InfoMap &infoMap = manual->getInfoMap();
  QRegularExpression infoReg(R"(\[(\w+)\s+\"([\s\S]*?)\"\])",
                             QRegularExpression::UseUnicodePropertiesOption);
  auto matchIter = infoReg.globalMatch(qstr);
  while (matchIter.hasNext()) {
    QRegularExpressionMatch match = matchIter.next();
    infoMap[match.captured(1)] = match.captured(2);
  }

  manual->setBoard();
}

void ManualIO::writeInfo_(const Manual *manual, QTextStream &stream) {
  // 去掉不需要显示的内容
  InfoMap infoMap{manual->getInfoMap()};
  for (InfoIndex infoIndex :
       {InfoIndex::MOVESTR, InfoIndex::ROWCOLS, InfoIndex::CALUATE_ECCOSN})
    infoMap.remove(getInfoName(infoIndex));
  infoMap.remove("id"); // 数据库存储时自动添加字段

  writeInfoToStream_(infoMap, stream);
}

void ManualIO::writeInfoToStream_(const InfoMap &infoMap, QTextStream &stream) {
  for (auto &key : infoMap.keys())
    stream << '[' << key << " \"" << infoMap[key] << "\"]\n";
  stream << '\n';
}

bool ManualIO_xqf::read_(Manual *manual, QFile &file) {
  QDataStream stream(&file);
  // stream.setByteOrder(QDataStream::LittleEndian);

  //文件标记'XQ'=$5158/版本/加密掩码/ProductId[4], 产品(厂商的产品号)
#define PIECENUM 32
  char Signature[3]{}, Version{}, headKeyMask{}, ProductId[4]{}, headKeyOrA{},
      headKeyOrB{}, headKeyOrC{}, headKeyOrD{},
      // 加密的钥匙和/棋子布局位置钥匙/棋谱起点钥匙/棋谱终点钥匙
      headKeysSum{}, headKeyXY{}, headKeyXYf{}, headKeyXYt{},
      headQiziXY[PIECENUM]{}, // 32个棋子的原始位置
      // 用单字节坐标表示, 将字节变为十进制, 十位数为X(0-8)个位数为Y(0-9),
      // 棋盘的左下角为原点(0, 0). 32个棋子的位置从1到32依次为:
      // 红: 车马相士帅士相马车炮炮兵兵兵兵兵 (位置从右到左, 从下到上)
      // 黑: 车马象士将士象马车炮炮卒卒卒卒卒 (位置从右到左,
      // 从下到上)PlayStepNo[2],
      PlayStepNo[2]{}, headWhoPlay{}, headPlayResult{}, PlayNodes[4]{},
      PTreePos[4]{}, Reserved1[4]{},
      // 该谁下 0-红先, 1-黑先/最终结果 0-未知, 1-红胜 2-黑胜, 3-和棋
      headCodeA_H[16]{}, TitleA[65]{}, TitleB[65]{}, //对局类型(开,中,残等)
      Event[65]{}, Date[17]{}, Site[17]{}, Red[17]{}, Black[17]{},
      Opening[65]{}, Redtime[17]{}, Blktime[17]{}, Reservedh[33]{},
      RMKWriter[17]{}, Author[17]{}; //, Other[528]{}; // 棋谱评论员/文件的作者

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
  stream.readRawData(&headKeyXYt, 1);       // = 16 bytes
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
  stream.readRawData(Author, 16);    // = 496 bytes

  assert(Signature[0] == 0x58 || Signature[1] == 0x51);
  // L" 检查密码校验和不对，不等于0。\n";
  assert((headKeysSum + headKeyXY + headKeyXYf + headKeyXYt) % 256 == 0);
  // L" 这是一个高版本的XQF文件，您需要更高版本的XQStudio来读取这个文件。\n";
  assert(Version <= 18);

  unsigned char KeyXY{}, KeyXYf{}, KeyXYt{}, F32Keys[PIECENUM],
      *head_QiziXY{(unsigned char *)headQiziXY};
  int KeyRMKSize{};
  if (Version <= 10) { // version <= 10 兼容1.0以前的版本
    KeyRMKSize = KeyXYf = KeyXYt = 0;
  } else {
    std::function<unsigned char(unsigned char, unsigned char)> __calkey =
        [](unsigned char bKey, unsigned char cKey) {
          // % 256; // 保持为<256
          return (((((bKey * bKey) * 3 + 9) * 3 + 8) * 2 + 1) * 3 + 8) * cKey;
        };
    KeyXY = __calkey(headKeyXY, headKeyXY);
    KeyXYf = __calkey(headKeyXYf, KeyXY);
    KeyXYt = __calkey(headKeyXYt, KeyXYf);
    KeyRMKSize = (((static_cast<unsigned char>(headKeysSum) * 256 +
                    static_cast<unsigned char>(headKeyXY)) %
                   32000) +
                  767);  // % 65536
    if (Version >= 12) { // 棋子位置循环移动
      QList<unsigned char> Qixy(std::begin(headQiziXY),
                                std::end(headQiziXY)); // 数组不能拷贝
      for (int i = 0; i != PIECENUM; ++i)
        head_QiziXY[(i + KeyXY + 1) % PIECENUM] = Qixy[i];
    }
    for (int i = 0; i != PIECENUM; ++i)
      head_QiziXY[i] -= KeyXY; // 保持为8位无符号整数，<256
  }
  int KeyBytes[4]{(headKeysSum & headKeyMask) | headKeyOrA,
                  (headKeyXY & headKeyMask) | headKeyOrB,
                  (headKeyXYf & headKeyMask) | headKeyOrC,
                  (headKeyXYt & headKeyMask) | headKeyOrD};
  const char copyright[]{"[(C) Copyright Mr. Dong Shiwei.]"};
  for (int i = 0; i != PIECENUM; ++i)
    F32Keys[i] = copyright[i] & KeyBytes[i % 4]; // ord(c)

  // 取得棋子字符串
  QString pieceChars(90, PieceBase::NULLCHAR);
  char pieChars[]{"RNBAKABNRCCPPPPPrnbakabnrccppppp"}; // QiziXY设定的棋子顺序
  for (int i = 0; i != PIECENUM; ++i) {
    int xy = head_QiziXY[i];
    if (xy <= 89)
      // 用单字节坐标表示, 将字节变为十进制,
      // 十位数为X(0-8),个位数为Y(0-9),棋盘的左下角为原点(0, 0)
      pieceChars[xy % 10 * 9 + xy / 10] = pieChars[i];
  }

  QTextCodec *codec = QTextCodec::codecForName("gbk");
  auto &infoMap = manual->getInfoMap();
  infoMap["VERSION"] = QString::number(Version).simplified();
  infoMap["RESULT"] = (QMap<unsigned char, QString>{
      {0, "未知"}, {1, "红胜"}, {2, "黑胜"}, {3, "和棋"}})[headPlayResult];
  infoMap["TYPE"] = (QMap<unsigned char, QString>{
      {0, "全局"}, {1, "开局"}, {2, "中局"}, {3, "残局"}})[headCodeA_H[0]];
  infoMap["TITLE"] = codec->toUnicode(TitleA).simplified();
  infoMap["EVENT"] = codec->toUnicode(Event).simplified();
  infoMap["DATE"] = codec->toUnicode(Date).simplified();
  infoMap["SITE"] = codec->toUnicode(Site).simplified();
  infoMap["RED"] = codec->toUnicode(Red).simplified();
  infoMap["BLACK"] = codec->toUnicode(Black).simplified();
  infoMap["OPENING"] = codec->toUnicode(Opening).simplified();
  infoMap["WRITER"] = codec->toUnicode(RMKWriter).simplified();
  infoMap["AUTHOR"] = codec->toUnicode(Author).simplified();
  manual->setFEN(SeatBase::pieCharsToFEN(pieceChars),
                 PieceColor::RED); // 可能存在不是红棋先走的情况？

  std::function<unsigned char(unsigned char, unsigned char)> __sub =
      [](unsigned char a, unsigned char b) { return a - b; }; // 保持为<256

  auto __readBytes = [&](char *bytes, int size) {
    int pos = file.pos();
    stream.readRawData(bytes, size);
    if (Version > 10) // '字节解密'
      for (int i = 0; i != size; ++i)
        bytes[i] = __sub(bytes[i], F32Keys[(pos + i) % 32]);
  };

  auto __getRemarksize = [&]() {
    char clen[4]{};
    __readBytes(clen, 4);
    return *(int *)clen - KeyRMKSize;
  };

  char data[4]{}, &frc{data[0]}, &trc{data[1]}, &tag{data[2]};
  std::function<QString()> __readDataAndGetRemark = [&]() {
    __readBytes(data, 4);
    int RemarkSize{};
    if (Version <= 10) {
      tag = ((tag & 0xF0) ? 0x80 : 0) | ((tag & 0x0F) ? 0x40 : 0);
      RemarkSize = __getRemarksize();
    } else {
      tag &= 0xE0;
      if (tag & 0x20)
        RemarkSize = __getRemarksize();
    }
    if (RemarkSize > 0) { // # 如果有注解
      char rem[2048]{};
      __readBytes(rem, RemarkSize);
      return codec->toUnicode(rem).simplified();
    } else
      return QString{};
  };

  file.seek(1024);
  manual->manualMove()->setCurRemark(__readDataAndGetRemark());
  manual->setBoard();
  ManualMoveAppendIterator appendIter{manual->appendIter()};
  if (tag & 0x80) //# 有左子树
    while (stream.status() == QDataStream::Status::Ok && !appendIter.isEnd()) {
      auto remark = __readDataAndGetRemark();
      //# 一步棋的起点和终点有简单的加密计算，读入时需要还原
      int fcolrow = __sub(frc, 0X18 + KeyXYf),
          tcolrow = __sub(trc, 0X20 + KeyXYt);
      assert(fcolrow <= 89 && tcolrow <= 89);

      int frow = fcolrow % 10, fcol = fcolrow / 10, trow = tcolrow % 10,
          tcol = tcolrow / 10;

      CoordPair coordPair{{frow, fcol}, {trow, tcol}};
      bool hasNext = tag & 0x80, hasOther = tag & 0x40;
      if (coordPair == manual->manualMove()->curCoordPair()) {
        qDebug() << file << coordPair << remark;
        if (!remark.isEmpty())
          manual->manualMove()->setCurRemark(remark);
      } else {
        appendIter.append_coordPair(coordPair, remark, hasNext, hasOther);
      }
    }

  return true;
}

bool ManualIO_xqf::write_(const Manual *manual, QFile &file) {
  Q_UNUSED(manual)
  Q_UNUSED(file)

  return false;
}

bool ManualIO_bin::read_(Manual *manual, QFile &file) {
  QDataStream stream(&file);
  QString fileTag;
  stream >> fileTag;
  if (fileTag != FILETAG_) // 文件标志不对
    return false;

  bool hasInfo;
  stream >> hasInfo;
  if (hasInfo) {
    int infoNum;
    stream >> infoNum;
    QString key;
    InfoMap &infoMap = manual->getInfoMap();
    for (int i = 0; i < infoNum; ++i) {
      stream >> key >> infoMap[key];
    }
  }
  manual->setBoard();

  QString remark;
  stream >> remark;
  manual->manualMove()->setCurRemark(remark);
  ManualMoveAppendIterator appendIter{manual->appendIter()};
  while (stream.status() == QDataStream::Status::Ok && !appendIter.isEnd()) {
    QString rowcols;
    QString remark;
    bool hasNext, hasOther;
    stream >> rowcols >> remark >> hasNext >> hasOther;

    appendIter.append_rowcols(rowcols, remark, hasNext, hasOther);
  }

  return true;
}

bool ManualIO_bin::write_(const Manual *manual, QFile &file) {
  QDataStream stream(&file);
  stream << FILETAG_;

  const InfoMap &infoMap = manual->getInfoMap();
  bool hasInfo{!infoMap.isEmpty()};
  stream << hasInfo;
  if (hasInfo) {
    int infoNum = infoMap.size();
    stream << infoNum;
    for (auto &key : infoMap.keys())
      stream << key << infoMap[key];
  }
  stream << manual->manualMove()->getCurRemark();

  ManualMoveFirstNextIterator firstNextIter(manual->manualMove());
  while (firstNextIter.hasNext()) {
    Move *move = firstNextIter.next();
    stream << move->rowcols() << move->remark() << move->hasNext()
           << move->hasOther();
  }

  return true;
}

bool ManualIO_json::read_(Manual *manual, QFile &file) {
  QByteArray byteArray = file.readAll();
  QJsonDocument document = QJsonDocument::fromJson(byteArray);
  if (document.isNull())
    return false;

  QJsonObject jsonRoot{document.object()},
      jsonInfo = jsonRoot.find("info")->toObject(),
      moveObj = jsonRoot.find("rootMove")->toObject();
  QString rootRemark = jsonRoot.find("remark")->toString();
  InfoMap &infoMap = manual->getInfoMap();
  for (auto iter = jsonInfo.constBegin(); iter != jsonInfo.constEnd(); ++iter)
    infoMap[iter.key()] = iter.value().toString();
  manual->setBoard();

  //    std::function<void(bool, QJsonObject)>
  //        __readMove = [&](bool isOther, QJsonObject item) {
  //            QJsonValue mitem { item.value("m") };
  //            Move* move {};
  //            if (!mitem.isUndefined()) {
  //                QString mvstr { mitem.toString() };
  //                int pos { mvstr.indexOf(' ') };
  //                QString rowcols { mvstr.left(pos) }, remark { mvstr.mid(pos
  //                + 1) };

  //                move = manual->goAppendMove(rowcols, remark, isOther);
  //            }

  //            QJsonValue nitem { item.value("n") }, oitem { item.value("o") };
  //            if (!nitem.isUndefined())
  //                __readMove(false, nitem.toObject());

  //            if (!oitem.isUndefined())
  //                __readMove(true, oitem.toObject());

  //            if (move)
  //                manual->manualMove()->backIs(isOther);
  //        };

  manual->manualMove()->setCurRemark(rootRemark);
  //    __readMove(false, jsonRootMove);
  //    manual->manualMove()->setMoveNums();
  QJsonObject otherMoveObj;
  QStack<QJsonObject> preOtherMoveObj;
  preOtherMoveObj.push(moveObj);
  ManualMoveAppendIterator appendIter{manual->appendIter()};
  while (!appendIter.isEnd()) {
    QString moveStr{moveObj.value("m").toString()};
    int pos{moveStr.indexOf(' ')};
    QString rowcols{moveStr.left(pos)}, remark{moveStr.mid(pos + 1)};

    QJsonValue nextValue{moveObj.value("n")}, otherValue{moveObj.value("o")};
    bool hasNext{!nextValue.isUndefined()}, hasOther{!otherValue.isUndefined()};
    appendIter.append_rowcols(rowcols, remark, hasNext, hasOther);

    if (hasOther) {
      otherMoveObj = otherValue.toObject();
      if (hasNext)
        preOtherMoveObj.push(otherMoveObj);
    }
    if (hasNext)
      moveObj = nextValue.toObject();
    else
      moveObj = hasOther ? otherMoveObj : preOtherMoveObj.pop();
  }

  return true;
}

bool ManualIO_json::write_(const Manual *manual, QFile &file) {
  QJsonObject jsonRoot, jsonInfo;
  const InfoMap &infoMap = manual->getInfoMap();
  for (auto &key : infoMap.keys())
    jsonInfo.insert(key, infoMap[key]);
  jsonRoot.insert("info", jsonInfo);

  //    std::function<QJsonObject(Move*)>
  //        __getJsonMove = [&](Move* move) {
  //            QJsonObject item {};
  //            if (!move->isRoot()) {
  //                QString mvstr;

  //                QTextStream(&mvstr) << move->rowcols() << ' ' <<
  //                move->remark(); item.insert("m", mvstr);
  //            }

  //            if (move->hasNext())
  //                item.insert("n", __getJsonMove(move->nextMove()));

  //            if (move->hasOther())
  //                item.insert("o", __getJsonMove(move->otherMove()));

  //            return item;
  //        };

  jsonRoot.insert("remark", manual->manualMove()->getCurRemark());
  //    jsonRoot.insert("rootMove",
  //    __getJsonMove(manual->manualMove()->rootMove()));
  QStack<Move *> moves;
  ManualMoveFirstNextIterator firstNextIter(manual->manualMove());
  while (firstNextIter.hasNext())
    moves.push(firstNextIter.next());

  QJsonObject behindJsonMove;
  QStack<QJsonObject> preOtherJsonMoves;
  while (!moves.isEmpty()) {
    Move *move = moves.pop();

    QJsonObject JsonMove(
        {{"m", QString("%1 %2").arg(move->rowcols()).arg(move->remark())}});
    if (move->hasNext())
      JsonMove.insert("n", behindJsonMove);
    if (move->hasOther())
      JsonMove.insert("o", preOtherJsonMoves.pop());

    if (move->isOther())
      preOtherJsonMoves.push(JsonMove);

    behindJsonMove = JsonMove;
  }
  jsonRoot.insert("rootMove", behindJsonMove);

  QJsonDocument document;
  document.setObject(jsonRoot);
  file.write(document.toJson());

  return true;
}

bool ManualIO_pgn::readString(Manual *manual, QString &pgnString) {
  QTextStream stream(&pgnString);
  return read_(manual, stream);
}

bool ManualIO_pgn::read_(Manual *manual, QFile &file) {
  QTextStream stream(&file);
  return read_(manual, stream);
}

bool ManualIO_pgn::write_(const Manual *manual, QFile &file) {
  QTextStream stream(&file);
  return write_(manual, stream);
}

void ManualIO_pgn::readMove_pgn_iccszh_(Manual *manual, QTextStream &stream,
                                        bool isPGN_ZH) {
  QString moveStr{stream.readAll()};
  //    QString otherBeginStr { R"((\()?)" }; // 1:( 变着起始标志
  QString boutStr{R"((\d+\.)?[\s.]*\b)"}; // 1: 回合着法起始标志
  QString ICCSZhStr{QString(R"(([%1]{4})\b)")
                        .arg(isPGN_ZH
                                 ? PieceBase::getZhChars()
                                 : PieceBase::getIccsChars())}; // 2: 回合着法
  QString remarkStr{R"((?:\s*\{([\s\S]*?)\})?)"};               // 3: 注解
  QString otherEndStr{R"(\s*(\)+)?)"}; // 4:) 变着结束标志，可能存在多个右括号
  QString otherBeginStr{R"((\((?=\d+\.))?)"}; // 5:( 变着起始标志

  //    QString movePat { otherBeginStr + boutStr + ICCSZhStr + remarkStr +
  //    otherEndStr };
  QString movePat{boutStr + ICCSZhStr + remarkStr + otherEndStr +
                  otherBeginStr};
  QRegularExpression moveReg(movePat,
                             QRegularExpression::UseUnicodePropertiesOption),
      remReg(remarkStr + R"(\s*1\.)",
             QRegularExpression::UseUnicodePropertiesOption);

  int index = 0;
  QRegularExpressionMatch match = remReg.match(moveStr);
  if (match.hasMatch()) {
    manual->manualMove()->setCurRemark(match.captured(1));
    index = match.capturedEnd();
  }

  //    QList<Move*> preOtherMoves {};
  ManualMoveAppendIterator appendIter{manual->appendIter()};
  auto matchIter = moveReg.globalMatch(moveStr, index);
  while (matchIter.hasNext()) {
    match = matchIter.next();
    //        bool isOther { !match.captured(1).isEmpty() };
    //        if (isOther)
    //            preOtherMoves.append(manual->manualMove()->move());

    //        QString iccsOrZhStr { match.captured(3) }, remark {
    //        match.captured(4) }; manual->goAppendMove(iccsOrZhStr, remark,
    //        isPGN_ZH, isOther);

    //        int num = match.captured(5).length();
    //        while (num-- && !preOtherMoves.isEmpty()) {
    //            auto otherMove = preOtherMoves.takeLast();
    //            manual->manualMove()->goTo(otherMove);
    //        }

    QString iccsOrZhStr{match.captured(2)}, remark{match.captured(3)};
    int endBranchNum{match.captured(4).length()};
    bool hasOther{!match.captured(5).isEmpty()};
    appendIter.append_iccsZhStr(iccsOrZhStr, remark, isPGN_ZH, endBranchNum,
                                hasOther);
  }

  //    manual->manualMove()->backStart();
}

bool ManualIO_pgn::writeMove_pgn_iccszh_(const Manual *manual,
                                         QTextStream &stream,
                                         bool isPGN_ZH) const {
  auto __getRemarkStr = [&](const QString &remark) {
    return (remark.isEmpty()) ? "" : (" \n{" + remark + "}\n ");
  };

  //    std::function<void(Move*, bool)>
  //        __writeMove_ = [&](Move* move, bool isOther) {
  //            QString boutStr { QString::number((move->nextIndex() + 1) / 2) +
  //            ". " }; bool isEven { move->nextIndex() % 2 == 0 }; stream <<
  //            (isOther ? "(" + boutStr + (isEven ? "... " : "")
  //                               : (isEven ? " " : boutStr))
  //                   << (isPGN_ZH ? move->zhStr() : move->iccs()) << ' '
  //                   << __getRemarkStr(move);

  //            if (move->hasOther()) {
  //                __writeMove_(move->otherMove(), true);
  //                stream << ")";
  //            }
  //            if (move->hasNext())
  //                __writeMove_(move->nextMove(), false);
  //        };

  //    stream << __getRemarkStr(manual->manualMove()->rootMove());
  //    if (!manual->manualMove()->isEmpty())
  //        __writeMove_(manual->getRootMove()->nextMove(), false);
  ManualMoveFirstOtherIterator firstOtherIter(manual->manualMove());
  stream << __getRemarkStr(manual->manualMove()->getCurRemark());
  QStack<Move *> preMoves;
  Move *iterPreMove{manual->manualMove()->move()};
  while (firstOtherIter.hasNext()) {
    Move *move = firstOtherIter.next();
    QString boutStr{QString::number((move->nextIndex() + 1) / 2) + ". "};
    bool isEven{move->nextIndex() % 2 == 0}, isOther{move->isOther()};

    if (!isOther && iterPreMove->isLeaf())
      while (!preMoves.isEmpty()) {
        stream << ")";
        if (preMoves.pop() == move->preMove())
          break;
      }
    stream << (isOther ? "(" + boutStr + (isEven ? "... " : "")
                       : (isEven ? " " : boutStr))
           << (isPGN_ZH ? move->zhStr() : move->iccs()) << ' '
           << __getRemarkStr(move->remark());

    if (isOther)
      preMoves.push(move->preMove());

    iterPreMove = move;
  }

  return true;
}

bool ManualIO_pgn::read_(Manual *manual, QTextStream &stream) {
  readInfo_(manual, stream);
  readMove_(manual, stream);

  return true;
}

bool ManualIO_pgn::write_(const Manual *manual, QTextStream &stream) {
  writeInfo_(manual, stream);
  return writeMove_(manual, stream);
}

void ManualIO_pgn_iccs::readMove_(Manual *manual, QTextStream &stream) {
  readMove_pgn_iccszh_(manual, stream, false);
}

bool ManualIO_pgn_iccs::writeMove_(const Manual *manual,
                                   QTextStream &stream) const {
  return writeMove_pgn_iccszh_(manual, stream, false);
}

void ManualIO_pgn_zh::readMove_(Manual *manual, QTextStream &stream) {
  readMove_pgn_iccszh_(manual, stream, true);
}

bool ManualIO_pgn_zh::writeMove_(const Manual *manual,
                                 QTextStream &stream) const {
  return writeMove_pgn_iccszh_(manual, stream, true);
}

static QString remarkNo__(int nextNo, int colNo) {
  return QString("(%1,%2)").arg(nextNo).arg(colNo);
}

static InfoMap getRems(const QString &remStr) {
  InfoMap rems{};
  QRegularExpression remReg(R"(\s*(\(\d+,\d+\)): \{([\s\S]*?)\})",
                            QRegularExpression::UseUnicodePropertiesOption);
  QRegularExpressionMatch match;
  auto matchIter = remReg.globalMatch(remStr);
  while (matchIter.hasNext()) {
    match = matchIter.next();
    rems[match.captured(1)] = match.captured(2);
  }

  return rems;
}

// 读取每行列表
static QList<QStringList> getMoveLines(const QString &moveStr) {
  QList<QStringList> moveLines{};
  for (auto &line : moveStr.split('\n')) {
    if (line.indexOf(QChar(L'↓')) != -1)
      continue;

    // 读取每行的着法列表
    int offs = 0;
    QStringList lineList{};
    while (offs < line.length() - 4) {
      lineList.append(line.mid(offs, 5));
      offs += 5;
    }
    moveLines.append(lineList);
  }

  return moveLines;
}

void ManualIO_pgn_cc::readMove_(Manual *manual, QTextStream &stream) {
  QString move_remStr{stream.readAll()};
  auto pos0 = move_remStr.indexOf("\n(");
  QString moveStr{move_remStr.left(pos0)}, remStr{move_remStr.mid(pos0)};

  InfoMap rems{getRems(remStr)};
  manual->manualMove()->setCurRemark(rems[remarkNo__(0, 0)]);

  QList<QStringList> moveLines{getMoveLines(moveStr)};
  if (moveLines.length() < 2)
    return;

  int row = 1, col = 0, rowNum = moveLines.length(),
      colNum = moveLines.front().length();
  QStack<QPair<int, int>> preRowCols;
  ManualMoveAppendIterator appendIter{manual->appendIter()};
  while (!appendIter.isEnd()) {
    QString fieldStr{moveLines[row][col]};
    QString zhStr = fieldStr.left(4), remark{rems[remarkNo__(row, col)]};
    bool hasNext{row + 1 < rowNum &&
                 moveLines[row + 1][col].front() != QChar(L'　')},
        hasOther{col < colNum && moveLines[row][col].back() == QChar(L'…')};
    appendIter.append_zhStr(zhStr, remark, hasNext, hasOther);

    if (hasNext && hasOther)
      preRowCols.push({row, col});

    if (hasNext)
      row++;
    else {
      if (!hasOther && !preRowCols.isEmpty()) {
        QPair<int, int> rowcol = preRowCols.pop();
        row = rowcol.first;
        col = rowcol.second;
      }
      do {
        ++col;
      } while (col < colNum && moveLines[row][col].front() == L'…');
    }
  }
}

bool ManualIO_pgn_cc::writeMove_(const Manual *manual,
                                 QTextStream &stream) const {
  QString blankStr((manual->manualMove()->maxCol() + 1) * 5, L'　');
  QVector<QString> lineStr((manual->manualMove()->maxRow() + 1) * 2, blankStr);

  ManualMoveFirstNextIterator firstNextIter(manual->manualMove());
  lineStr.front().replace(
      0, 3, QString("　%1").arg(manual->manualMove()->move()->zhStr()));
  lineStr[1][2] = L'↓';
  while (firstNextIter.hasNext()) {
    Move *move = firstNextIter.next();

    int firstcol{move->cc_ColIndex() * 5}, row{move->nextIndex() * 2};
    lineStr[row].replace(firstcol, 4, move->zhStr());

    if (move->hasNext())
      lineStr[row + 1][firstcol + 2] = L'↓';
    if (move->hasOther()) {
      int fcol{firstcol + 4}, num{move->otherMove()->cc_ColIndex() * 5 - fcol};
      lineStr[row].replace(fcol, num, QString(num, L'…'));
    }
  }
  for (auto &line : lineStr)
    stream << line << '\n';

  std::function<void(Move *)> setRemarkPGN_CC_ = [&](Move *move) {
    if (!move->remark().isEmpty())
      stream << remarkNo__(move->nextIndex(), move->cc_ColIndex()) << ": {"
             << move->remark() << "}\n";
  };

  firstNextIter.reset();
  setRemarkPGN_CC_(manual->manualMove()->move());
  while (firstNextIter.hasNext()) {
    Move *move = firstNextIter.next();
    setRemarkPGN_CC_(move);
  }

  return true;
}
