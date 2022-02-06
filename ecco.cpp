#include "ecco.h"
#include "instance.h"
#include "instanceio.h"
#include "move.h"
#include "piece.h"
#include "seat.h"
#include "tools.h"
#include <QDebug>
#include <QFile>
#include <functional>

enum EccoRecordIndex {
    REC_SN,
    REC_NAME,
    REC_NUMS,
    REC_MVSTRS,
    REC_PRE_SN,
    REC_REGSTR
};

enum EccoTableIndex {
    TABLE_ID,
    TABLE_SN,
    TABLE_NAME,
    TABLE_REGSTR
};

#ifdef DEBUG
static QString boutStrsFileName { "output/eccoBoutStrs.txt" };
QString eccoBoutStrs;
QTextStream streamBoutStrs(&eccoBoutStrs);
#endif

constexpr QChar UnorderChar { '|' }, OrderChar { '*' }, SeparateChar { '/' }, ExceptChar { L'除' };

Ecco::Ecco()
{
    QString dbName { "output/data.db" }, libTblName { "ecco" }, manTblName { "manual" };
    dbName_ = dbName;
    libTblName_ = libTblName;
    manTblName_ = manTblName;

    // 如文件不存在，则新建空文件
    if (!QFileInfo::exists(dbName)) {
        QFile file(dbName_);
        file.open(QIODevice::WriteOnly);
        file.close();
    }

    database_ = QSqlDatabase::addDatabase("QSQLITE");
    database_.setDatabaseName(dbName_);
    database_.open();
}

Ecco::~Ecco()
{
    database_.close();
}

bool Ecco::setECCO(PInstance ins)
{
    QStringList eccoRec = getECCO(ins);
    if (eccoRec.isEmpty())
        return false;

    ins->setEcco(eccoRec);
    return true;
}

bool Ecco::setECCO(QList<PInstance> insList)
{
    for (auto& pins : insList)
        if (!setECCO(pins))
            return false;

    return true;
}

void Ecco::initEccoLib()
{
    QString htmlStr;
    /*// 并发下载开局库的网页数据源
    QList<QString> urls;
    for (auto cindex : QString { "abcde" })
        urls.append(QString("https://www.xqbase.com/ecco/ecco_%1.htm").arg(cindex));
    htmlStr = Tools::downHtmlsFromUrlsBlockingReduced(urls);
#ifdef DEBUG
    Tools::writeTxtFile("output/eccoHtml.txt", htmlStr, QIODevice::WriteOnly);
#endif

    // 清洁网页字符串内容
    htmlStr.remove(QRegularExpression("</?(?:div|font|img|strong|center|meta|dl|dt|table|tr|td|em|p|li|dir"
                                      "|html|head|body|title|a|b|tbody|script|br|span)[^>]*>",
        QRegularExpression::UseUnicodePropertiesOption));
    htmlStr.replace(QRegularExpression(R"((\n|\r)\s+)", QRegularExpression::UseUnicodePropertiesOption), " ");
#ifdef DEBUG
    qDebug() << __FUNCTION__ << __LINE__;
    Tools::writeTxtFile("output/eccoCleanHtml.txt", htmlStr, QIODevice::WriteOnly);
#endif
    //*/
    htmlStr = Tools::readTxtFile("output/eccoCleanHtml.txt"); // 读取已下载的文件

    // 提取开局内容至结构化记录
    QMap<QString, QStringList> eccoRecords;
    setEccoRecord_(eccoRecords, htmlStr);
#ifdef DEBUG
    QString recordsStr;
    int recIndex = 0;
    for (auto& record : eccoRecords)
        QTextStream(&recordsStr) << ++recIndex << "." << record.join("\n>> ") << "\n\n";

    // 已与原始网页，核对完全一致
    Tools::writeTxtFile("output/eccoRecords.txt", recordsStr, QIODevice::WriteOnly);
//    qDebug() << __FUNCTION__ << __LINE__;
#endif

    // 设置记录内正则字符串
    setEccoRecordRegstrField_(eccoRecords);

    //  存入数据库文件
    writeEccoLib_(eccoRecords);
}

void Ecco::downAllXqbaseManual()
{
    QList<int> idList;
    int firstId = 401, endId = 12141; // 12141;
    for (int id = firstId; id <= endId; ++id)
        idList.append(id);

    QList<InfoMap> infoMapList = downXqbaseManual_(idList);
    setRowcols_(infoMapList);
    writeManual_(infoMapList, firstId == 1);
}

void Ecco::checkXqbaseManual()
{
#ifdef DEBUG
    QString result;
    QTextStream stream(&result);
    QSqlQuery query;
    int startId = 1, endId = 400;
    query.exec(QString("SELECT * FROM %1 WHERE id>=%2 AND id<=%3;")
                   .arg(manTblName_)
                   .arg(startId)
                   .arg(endId));
    //    query.exec(QString("SELECT * FROM %1 WHERE %2 IS NULL;")
    //                   .arg(manTblName_)
    //                   .arg(InstanceIO::getInfoName(InfoIndex::CALUATE_ECCOSN)));
    QList<QPair<int, QString>> id_eccosnList;
    while (query.next()) {
        QString eccoRowcols = query.value(InstanceIO::getInfoName(InfoIndex::ROWCOLS)).toString(),
                eccoSN = query.value(InstanceIO::getInfoName(InfoIndex::ECCOSN)).toString();
        QStringList eccoRec = getECCO(eccoRowcols);
        if (eccoRec.at(0) == eccoSN)
            id_eccosnList.append({ query.value("id").toInt(), eccoSN });
        else
            stream << QString("ECCOSN:%1\nECCONAME:%2\nCALUATE_ECCOSN:\n%3\n\n")
                          .arg(query.value(InstanceIO::getInfoName(InfoIndex::ECCOSN)).toString())
                          .arg(query.value(InstanceIO::getInfoName(InfoIndex::ECCONAME)).toString())
                          .arg(query.value(InstanceIO::getInfoName(InfoIndex::CALUATE_ECCOSN)).toString());
    }

    for (auto& id_eccosn : id_eccosnList) {
        query.exec(QString("UPDATE %1 SET %2 = '%3' WHERE id = %4;")
                       .arg(manTblName_)
                       .arg(InstanceIO::getInfoName(InfoIndex::CALUATE_ECCOSN))
                       .arg(id_eccosn.second)
                       .arg(id_eccosn.first));
    }

    Tools::writeTxtFile(QString("output/%1.txt").arg(__FUNCTION__), result, QIODevice::WriteOnly);
#endif
}

QString Ecco::getRowcols_(const QString& zhStr, Instance& ins, bool isGo)
{
    static const QMap<QString, QString> mv_premv {
        { "车一平二", "马二进三" },
        { "车二进六", "车一平二" },
        { "车八进四", "车九平八" },
        { "车九平八", "马八进七" },
        { "马八进七", "马七退八" },
        { "马七进六", "兵七进一" },
        { "马三进四", "兵三进一" },
        { "马二进三", "马三退二" },
        { "炮八平七", "卒３进１" },
        { "车８进５", "炮８平９" },
        { "车９平８", "马８进７" },
        { "炮８进２", "车二退六" },
        { "炮８平９", "车９平８" }
    };

    PMove move = ins.appendMove_ecco(zhStr);
    if (move) {
        if (!isGo)
            ins.backNext();
    } else {
        PMove premove = move;
        QStringList premvs { zhStr };
        while (!premove && mv_premv.contains(premvs.first())) {
            const QString& premv = mv_premv[premvs.first()];
#ifdef DEBUG
            streamBoutStrs << QString("\t\tpremv:%1\n").arg(premv);
#endif
            premove = ins.appendMove_ecco(premv);
            premvs.prepend(premv);
        }
        if (premove) {
            for (int i = 1; i < premvs.count(); ++i) {
                move = ins.appendMove_ecco(premvs.at(i));
            }
            for (int i = 0; i < premvs.count(); ++i)
                ins.backNext();
        }
    }

    QString rowcols = move ? move->rowcols() : QString();
#ifdef DEBUG
    //    if (!move)
    streamBoutStrs << QString("\t\t%1:%2 %3: %4\n")
                          .arg(rowcols.isEmpty() ? "失败" : "OK  ")
                          .arg(zhStr)
                          .arg(isGo)
                          .arg(rowcols);
#endif

    return rowcols;
}

QStringList Ecco::getRowcolsList_(const QString& mvstr, bool isOrder, Instance& ins)
{
    static const QRegularExpression
        reg_m { QRegularExpression(QString(R"(([%1]{4}))").arg(Pieces::getZhChars()),
            QRegularExpression::UseUnicodePropertiesOption) },
        reg_notOrder { QRegularExpression(
            "马二进一/车九平八"
            "|马二进三/马八进七"
            "|马八进七／马八进九"
            "|炮八平六/马八进九"
            "|兵三进一.兵七进一"
            "|相三进五／相七进五"
            "|仕四进五／仕六进五"
            "|马８进７/车９进１"
            "|马８进７/马２进３"
            "|炮８进４/炮２进４"
            "|炮８平５／炮２平５"
            "|炮８平６／炮２平４／炮８平４／炮２平６"
            "|卒３进１/马２进３"
            "|象３进５／象７进５"
            "|象７进５／象３进５"
            "|士６进５／士４进５",
            QRegularExpression::UseUnicodePropertiesOption) };

    QStringList rowcolsList;
    bool isFirst = true, isUnorderMvstr = mvstr.contains(reg_notOrder);
    QRegularExpressionMatchIterator matchIter = reg_m.globalMatch(mvstr);
    while (matchIter.hasNext()) {
        // 着法非强制按顺序，且非前进着法，且为首着，则为变着
        bool isGo = isOrder || (!isUnorderMvstr && isFirst);
        QString zhStr = matchIter.next().captured(1);
        QString rowcols = getRowcols_(zhStr, ins, isGo);
        if (!rowcols.isEmpty())
            rowcolsList.append(rowcols);

        isFirst = false;
    }

    return rowcolsList;
}

// 处理有顺序的的连续着法标记
QString Ecco::getColorRowcols_(const QString& mvstrs, const QString& moveRegStr, Instance& ins)
{
    QString colorRowcols;
    QStringList mvstrList = mvstrs.split(OrderChar);
    int count = mvstrList.count();
    for (int i = 0; i < count; ++i) {
        const QString& mvstr { mvstrList.at(i) };
        // 包含"红/黑方："的前置着法，强制按顺序走棋
        bool isOrder = mvstr.contains("方：");
        QStringList rowcolsList = getRowcolsList_(mvstr, isOrder, ins);
        int num { rowcolsList.length() };
        QString rowcols { rowcolsList.join(UnorderChar) },
            // 是否本回合的最后顺序着法
            indexStr { isOrder ? "" : (i == count - 1 ? "1," : "0,") };
        // 着法数量大于1，或属于多顺序着法的非最后着法
        if (num > 1 || (count > 1 && i != count - 1))
            rowcols = (mvstr.contains(ExceptChar)
                    // B30 "走过除...以外的着法" // =>(?!%1) 否定顺序环视 见《精通正则表达式》P66.
                    ? QString("%1+?").arg(moveRegStr)
                    : QString("(?:%1){%2%3}").arg(rowcols).arg(indexStr).arg(num));

        // 按顺序添加着法组
        colorRowcols.append(rowcols);
    }

    return colorRowcols;
}

QString Ecco::getRegStr_(const BoutStrs& boutStrs, Instance& ins)
{
    // 处理不分先后标记（最后回合，或本回合已无标记时调用）
    std::function<void(QString&, int&)>
        handleUnorderChar_ = [](QString& groupRowcols, int& boutNotOrderCount) {
            // regStr的最后字符有标记，则去掉最后一个标记
            if (!groupRowcols.isEmpty() && groupRowcols.back() == UnorderChar)
                groupRowcols.remove(groupRowcols.length() - 1, 1);
            // 如果存在有一个以上标记，则包裹处理
            if (boutNotOrderCount > 1)
                groupRowcols = QString("(?:%1){%2}").arg(groupRowcols).arg(boutNotOrderCount);

            boutNotOrderCount = 0;
        };

    ins.reset();
    QChar preBoutNo { '1' };
    QString moveRegStr { R"((?:\d{4}))" };
    int boutNotOrderCount[] = { 0, 0 };
    QString regStr[] = { {}, {} };
    QStringList boutGroupRowcols = { {}, {} };
    for (QChar boutNo : boutStrs.keys()) {
        for (int color = 0; color < COLORNUM; ++color) {
            const QString& mvstrs = boutStrs[boutNo][color];
            if (!mvstrs.isEmpty()) {
                bool endIsUnorderChar = mvstrs.back() == UnorderChar;
                // 处理不分先后标记（本回合已无标记）
                if (!endIsUnorderChar)
                    handleUnorderChar_(boutGroupRowcols[color], boutNotOrderCount[color]);

                // 取得某种颜色的着法结果
                QString colorRowcols { getColorRowcols_(mvstrs, moveRegStr, ins) };
                // 处理不分先后标记（本回合有标记）
                if (endIsUnorderChar) {
                    colorRowcols.append(UnorderChar);
                    ++boutNotOrderCount[color];
                }

                boutGroupRowcols[color].append(colorRowcols);
            }

            // 处理不分先后标记（最后回合）
            if (boutNo == boutStrs.lastKey())
                handleUnorderChar_(boutGroupRowcols[color], boutNotOrderCount[color]);

            // 存入结果字符串
            if (boutNotOrderCount[color] == 0) {
                regStr[color].append(boutGroupRowcols[color]);
                boutGroupRowcols[color] = "";
            }
        }

        preBoutNo = boutNo;
    }

    return QString("~%1%2*?-%3").arg(regStr[0]).arg(moveRegStr).arg(regStr[1]);
}

void Ecco::setEccoRecordRegstrField_(QMap<QString, QStringList>& eccoRecords)
{
#ifdef DEBUG
    int recIndex {};
#endif

    QRegularExpression reg_pre { QRegularExpression("(红方：.+)\\s+(黑方：.+)",
        QRegularExpression::UseUnicodePropertiesOption) };

    Instance ins;
    for (auto& record : eccoRecords) {
        QString sn { record[REC_SN] }, mvstrs { record[REC_MVSTRS] };
        if (sn.length() != 3 || mvstrs.isEmpty())
            continue;

#ifdef DEBUG
        streamBoutStrs << ++recIndex << "." << record.join("\n>> ") << "\n";
#endif
        BoutStrs boutStrs {};
        // 处理前置着法字符串
        if (!record[REC_PRE_SN].isEmpty()) {
            QString pre_mvstrs = eccoRecords[record[REC_PRE_SN]][REC_MVSTRS];
            QRegularExpressionMatch match = reg_pre.match(pre_mvstrs);
            if (match.hasMatch()) {
#ifdef DEBUG
                streamBoutStrs << "\tRedBlack@:" + match.captured(1) + match.captured(2) + '\n'; // RedBlack@:40个
#endif
                // 处理前置着法描述字符串————"红方：黑方："
                boutStrs.insert('1', { match.captured(1), match.captured(2) });
            } else {
#ifdef DEBUG
                streamBoutStrs << "\tOterPre@:" + pre_mvstrs + '\n'; // OterPre@:75个
#endif
                // 处理其他的前置着法描述字符串
                setBoutStrs_(boutStrs, sn, pre_mvstrs, true);
            }
        }

        // 处理着法字符串
        setBoutStrs_(boutStrs, sn, mvstrs, false);
#ifdef DEBUG
        for (auto boutNo : boutStrs.keys())
            streamBoutStrs << '\t' << boutNo << ". " << boutStrs[boutNo].join("\t\t") << '\n';
#endif

        // 设置着法正则描述字符串
        record[REC_REGSTR] = getRegStr_(boutStrs, ins);

#ifdef DEBUG
        streamBoutStrs << "\t\tregstr: " + record[REC_REGSTR] + "\n\n";
        Tools::writeTxtFile(boutStrsFileName, eccoBoutStrs, QIODevice::WriteOnly);
#endif
    }
}

// 获取回合着法字符串
void Ecco::setBoutStrs_(BoutStrs& boutStrs, const QString& sn, const QString& mvstrs, bool isPreMvStrs)
{
    // 处理回合着法
    std::function<void(BoutStrs&, QChar, int, const QString&)>
        handleMvstr_ = [](BoutStrs& boutStrs, QChar boutNo, int color, const QString& qstr) {
            // 已存在序号为boutNo的着法字符串
            if (boutStrs.find(boutNo) != boutStrs.end()) {
                QString& mvstr = boutStrs[boutNo][color];
                // 已存在着法字符串尾部不匹配待处理字符串
                if (!mvstr.endsWith(qstr)) {
                    if (!mvstr.isEmpty()) {
                        // 最后顺序着法字符串的长度等于待处理字符串长度，则添加无顺序字符标记；否则，添加有顺序标记
                        bool sameLength = mvstr.length() - (mvstr.lastIndexOf(OrderChar) + 1) == qstr.length();
                        mvstr.append(sameLength ? SeparateChar : OrderChar);
                    }
                    mvstr.append(qstr);
                }
            } else {
                QStringList mvstrs { {}, {} };
                mvstrs[color] = qstr;
                boutStrs.insert(boutNo, mvstrs);
            }
        };

    // 处理不分先后标记
    std::function<void(BoutStrs&, QChar, int)>
        handleUnorder_ = [](BoutStrs& boutStrs, QChar boutNo, int color) {
            bool isStart = true;
            while (boutStrs.contains(boutNo)) {
                for (QString& mvstrs : boutStrs[boutNo]) {
                    if (!mvstrs.isEmpty())
                        mvstrs.append(UnorderChar);
                    if (isStart && color == 0)
                        break;
                }

                boutNo = QChar(boutNo.unicode() - 1);
                isStart = false;
            }
        };

    // 处理"此前..."着法描述字符串
    std::function<void(BoutStrs&, QChar, int, const QString&)>
        insertMvstr_ = [](BoutStrs& boutStrs, QChar boutNo, int color, const QString& qstr) {
            while (boutStrs.contains(boutNo)) {
                QString& mvstr = boutStrs[boutNo][color];
                if (!mvstr.isEmpty() && !mvstr.contains(qstr))
                    mvstr.prepend(qstr + OrderChar);

                boutNo = QChar(boutNo.unicode() - 1);
            }
        };

    // 捕捉一步着法: 1.着法，2.可能的“不分先后”标识，3.可能的“此前...”着法
    QString mv { QString(R"([%1]{4})").arg(Pieces::getZhChars()) },
        moveStr { QString(R"((?:%1(?:／%1)*))").arg(mv) },
        premStr { QString(R"((?:%1(?:[、／和，黑可走]{1,4}%1)*))").arg(mv) },
        rich_mvStr { QString(R"((%1|……)(\s?\(不.先后[)，])?(?:[^\da-z]*?此前.*?走((?:过除)?%2).*?\))?)")
                         .arg(moveStr)
                         .arg(premStr) };
    // 捕捉一个回合着法：1.序号，2-4.首着、“不分先后”、“此前...”，5-7.着法、“不分先后”、“此前...”
    QRegularExpression reg_bm = QRegularExpression(QString(R"(([\da-z]).\s?%1(?:，%1)?)").arg(rich_mvStr),
        QRegularExpression::UseUnicodePropertiesOption);
    QRegularExpressionMatchIterator matchIter = reg_bm.globalMatch(mvstrs);
    while (matchIter.hasNext()) {
        QRegularExpressionMatch match = matchIter.next();
        QChar boutNo = match.captured(1).at(0);
        if (isPreMvStrs && sn.startsWith("C9"))
            boutNo = boutNo.toUpper();
        for (int i = 2; i <= match.lastCapturedIndex(); ++i) {
            QString qstr { match.captured(i) };
            if (qstr.isEmpty())
                continue;

            int color = i / 5;
            switch ((i - 2) % 3) {
            case 0: // i == 2 || i == 5 // 处理回合着法
                if (qstr == "……")
                    continue;

                // B32 黑棋第一、二着的顺序有误
                if (sn == "B32" && color == 1 && boutNo < '3')
                    qstr = (boutNo == '1') ? "炮８平６" : "马８进７";
                // B21 棋子文字错误
                else if (sn == "B21" && !isPreMvStrs) {
                    if (qstr.contains("象七进九"))
                        qstr.replace(L'象', L'相');
                    else if (qstr.contains("车８进５"))
                        boutNo = 'r';
                }
                // D34 序号有误
                else if (sn == "D34" && !isPreMvStrs)
                    boutNo = '5';
                // C46 优化黑色，根据实例微调红色
                else if (boutNo == '9' && sn == "C46") {
                    QString insertMv { color == 1 ? "炮９平７" : "车九平八" };
                    qstr.prepend(insertMv + OrderChar);
                    if (color == 1)
                        boutStrs['8'][color] = "";
                }

                handleMvstr_(boutStrs, boutNo, color, qstr);
#ifdef DEBUG
                streamBoutStrs << "\tBoutMoveStr:" + qstr + '\n'; // BoutMoveStr:1520
#endif
                break;
            case 1: // i == 3 || i == 6 // "不分先后"
                handleUnorder_(boutStrs, boutNo, color);
#ifdef DEBUG
                streamBoutStrs << "\tNotOrder:" + qstr + '\n'; // NotOrder:26个
#endif
                break;
            case 2: // i == 4 || i == 7 // "此前..."
                if (color == 0) {
                    if (sn == "B40")
                        qstr.append("车一平二");
                    else if (sn == "D43")
                        qstr.append("马二进三");
                } else {
                    if (!isPreMvStrs && sn == "D41")
                        color = 0;
                    // D29 先处理红棋着法 (此前红可走马八进七，黑可走马２进３、车９平４)
                    else if (sn == "D29") {
                        insertMvstr_(boutStrs, boutNo, 0, qstr.left(4));
                        qstr.remove(0, 8);
                    }
                }

                insertMvstr_(boutStrs, boutNo, color, qstr);
#ifdef DEBUG
                // PreInsertBout:53个     \tInsertBout:103个
                streamBoutStrs << QString("\t%1InsertBout:%2\n").arg(isPreMvStrs ? "Pre" : "").arg(qstr);
#endif
            }
        }
    }
}

void Ecco::setEccoRecord_(QMap<QString, QStringList>& eccoRecords, const QString& cleanHtmlStr)
{
    // 使用正则表达式分解字符串，获取字段内容
    QList<QRegularExpression> regs = {
        // field: sn name nums
        QRegularExpression(R"(([A-E])．(\S+?)\((共[\s\S]+?局)\))",
            QRegularExpression::UseUnicodePropertiesOption),
        // field: sn name nums mvstr B2. C0. D1. D2. D3. D4. \\s不包含"　"(全角空格)
        QRegularExpression(R"(([A-E]\d)(?:．|\. )(?:空|([\S^\r]+?)\((共[\s\S]+?局)\)\s+([\s\S]*?))(?=[\s　]*[A-E]\d0．))",
            QRegularExpression::UseUnicodePropertiesOption),
        // field: sn name mvstr nums
        QRegularExpression(R"(([A-E]\d{2})．(\S+)[\s　]+(?:(?![A-E]\d|上一)([\s\S]*?)[\s　]*(无|共[\s\S]+?局)[\s\S]*?(?=上|[A-E]\d{0,2}．))?)",
            QRegularExpression::UseUnicodePropertiesOption),
        // field: sn mvstr C20 C30 C61 C72局面字符串
        QRegularExpression(R"(([A-E]\d)\d局面 =([\s\S]*?)(?=[\s　]*[A-E]\d{2}．))",
            QRegularExpression::UseUnicodePropertiesOption)
    };

    for (int gIndex = 0; gIndex < regs.length(); ++gIndex) {
        QRegularExpressionMatchIterator matchIter = regs[gIndex].globalMatch(cleanHtmlStr);
        while (matchIter.hasNext()) {
            QStringList capTexts = matchIter.next().capturedTexts();
            if (gIndex == 3)
                // C20 C30 C61 C72局面字符串, 供设置前置着法字符串
                eccoRecords[capTexts[1]][REC_MVSTRS] = capTexts[2];
            else {
                // field: SN_I, NAME_I, NUMS_I, MVSTRS_I, PRE_MVSTRS_I, REGSTR_I
                QStringList record { "", "", "", "", "", "" };
                for (int fIndex = REC_SN; fIndex < capTexts.length() - 1; ++fIndex) {
                    int index = fIndex;
                    if (gIndex == 2 && fIndex > 1)
                        index = (fIndex == 3 ? REC_NUMS : REC_MVSTRS);
                    record[index] = capTexts[fIndex + 1];
                }

                eccoRecords[capTexts[1]] = record;
            }
        }
    }

    // 设置pre_mvstrs字段, 前置省略内容 有40+75=115项
#ifdef DEBUG
    int no = 0;
    QString qstr;
    QTextStream stream(&qstr);
#endif
    for (auto& record : eccoRecords) {
        QString sn { record[REC_SN] }, mvstrs { record[REC_MVSTRS] };
#ifdef DEBUG
        if (sn.length() <= 2 && !mvstrs.isEmpty())
            stream << no++ << ". " << sn << " " << mvstrs << '\n';
#endif
        if (sn.length() != 3 || mvstrs.length() < 3)
            continue;

        QString sn_pre {};
        if (mvstrs[0] == L'从') {
            // 三级局面的 C2 C3_C4 C61 C72局面 有40项
            sn_pre = mvstrs.mid(1, 2);
        } else if (mvstrs[0] != L'1') {
            // 前置省略的着法 有75项
            sn_pre = sn.left(2); //  截断为两个字符长度
            if (sn_pre[0] == L'C')
                sn_pre[1] = L'0'; // C0/C1/C5/C6/C7/C8/C9 => C0
            else if (sn_pre == "D5")
                sn_pre = "D51"; // D5 => D51
        } else
            continue;

        record[REC_PRE_SN] = sn_pre;
#ifdef DEBUG
        stream << no++ << ". " << sn_pre << '@' << sn << " " << mvstrs << '\n';
#endif
    }
#ifdef DEBUG
    Tools::writeTxtFile("output/eccoPreMvstrs.txt", qstr, QIODevice::WriteOnly);
#endif
}

void Ecco::writeEccoLib_(QMap<QString, QStringList>& eccoRecords)
{
    QSqlQuery query;
    QSqlDatabase::database().transaction(); // 启动事务
    query.exec(QString("DROP TABLE %1;")
                   .arg(libTblName_));
    query.exec(QString("CREATE TABLE %1 ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                       "sort_id INTEGER,"
                       "sn TEXT NOT NULL,"
                       "name TEXT NOT NULL,"
                       "regstr TEXT"
                       ");")
                   .arg(libTblName_));

    for (auto& record : eccoRecords)
        if (record[REC_NAME].length() > 1)
            query.exec(QString("INSERT INTO %1 (sn,name,regstr)"
                               "VALUES('%2','%3','%4');")
                           .arg(libTblName_)
                           .arg(record[REC_SN])
                           .arg(record[REC_NAME])
                           .arg(record[REC_REGSTR]));

    // C99 (不包括C20~C49)，因此将C99的sort_id排在C20与C19之间
    query.exec(QString("UPDATE %1 SET sort_id = id;")
                   .arg(libTblName_));
    query.exec(QString("UPDATE %1 SET sort_id = 128 WHERE sn = 'C99';")
                   .arg(libTblName_));

    QSqlDatabase::database().commit(); // 提交事务
}

QList<InfoMap> Ecco::downXqbaseManual_(const QList<int>& idList)
{
    QList<QPair<InfoIndex, QRegularExpression>> infoIndex_regs = {
        //        { InfoIndex::SOURCE,
        //            QRegularExpression(R"(https%3A%2F%2Fwww.xqbase.com%2Fxqbase%2F%3Fgameid%3D(\d+?)&)",
        //                QRegularExpression::UseUnicodePropertiesOption) },
        { InfoIndex::TITLE,
            QRegularExpression(R"(<title>(.*?)</title>)",
                QRegularExpression::UseUnicodePropertiesOption) },
        { InfoIndex::EVENT,
            QRegularExpression(R"(>([^>]+赛[^>]*?)<)",
                QRegularExpression::UseUnicodePropertiesOption) },
        { InfoIndex::BLACK,
            QRegularExpression(R"(>黑方 ([^<]*?)<)",
                QRegularExpression::UseUnicodePropertiesOption) },
        { InfoIndex::RED,
            QRegularExpression(R"(>红方 ([^<]*?)<)",
                QRegularExpression::UseUnicodePropertiesOption) },
        { InfoIndex::DATE,
            QRegularExpression(R"(>(\d+年\d+月(?:\d+日)?)(?: ([^<]*?))?<)",
                QRegularExpression::UseUnicodePropertiesOption) }, // SITE
        { InfoIndex::ECCOSN,
            QRegularExpression(R"(>([A-E]\d{2})\. ([^<]*?)<)",
                QRegularExpression::UseUnicodePropertiesOption) }, // ECCONAME
        { InfoIndex::MOVESTR,
            QRegularExpression(R"(<pre>\s*?(1\.[\s\S]*?)\((.+?)\)</pre\>)",
                QRegularExpression::UseUnicodePropertiesOption) } // RESULT
    };

    QList<InfoMap> infoMapList;
    //*// 并发下载象棋百科棋谱库的网页数据源
    QList<QString> urls;
    for (auto& id : idList)
        urls.append(QString("https://www.xqbase.com/xqbase/?gameid=%1").arg(id));
    QList<QString> htmlStrs = Tools::downHtmlsFromUrlsBlocking(urls);
    for (int index = 0; index < htmlStrs.count(); ++index) {
        InfoMap infoMap { InstanceIO::getInitInfoMap() };
        for (auto& infoIndex_reg : infoIndex_regs) {
            InfoIndex infoIndex = infoIndex_reg.first;
            QString infoName { InstanceIO::getInfoName(infoIndex) };
            auto match = infoIndex_reg.second.match(htmlStrs.at(index));
            infoMap[infoName] = match.captured(1);
            if (infoIndex == InfoIndex::MOVESTR) {
                infoMap[infoName].replace("\r\n", " ");
            }

            InfoIndex infoIndex2 { (infoIndex == InfoIndex::DATE
                    ? InfoIndex::SITE
                    : (infoIndex == InfoIndex::ECCOSN           ? InfoIndex::ECCONAME
                            : (infoIndex == InfoIndex::MOVESTR) ? InfoIndex::RESULT
                                                                : InfoIndex::NOTINFOINDEX)) };
            if (infoIndex2 != InfoIndex::NOTINFOINDEX)
                infoMap[InstanceIO::getInfoName(infoIndex2)] = match.captured(2);
        }
        infoMap[InstanceIO::getInfoName(InfoIndex::SOURCE)] = urls.at(index);

        infoMapList.append(infoMap);
    }

    return infoMapList;
}

void Ecco::setRowcols_(QList<InfoMap>& infoMapList)
{
    std::function<void(InfoMap&)> setRowcols__ = [](InfoMap& infoMap) {
        QString pgnString = InstanceIO::pgnString(infoMap);
        PInstance ins = InstanceIO::parseString(pgnString);
        infoMap[InstanceIO::getInfoName(InfoIndex::ROWCOLS)] = ins->getECCORowcols();
    };

    QtConcurrent::blockingMap(infoMapList, setRowcols__);
}

QString Ecco::getFieldNames_(const QStringList& names, const QString& suffix)
{
    QString fieldNames;
    for (auto& name : names)
        fieldNames.append(QString("%1 %2,").arg(name).arg(suffix));

    return fieldNames.remove(fieldNames.length() - 1, 1);
}

void Ecco::writeManual_(QList<InfoMap>& infoMapList, bool initTable)
{
    QSqlQuery query;
    QSqlDatabase::database().transaction(); // 启动事务
    query.exec(QString("SELECT * FROM sqlite_master WHERE type = 'table' AND tbl_name = '%1';")
                   .arg(manTblName_));
    bool hasTable = query.next();
    if (hasTable && initTable) {
        query.exec(QString("DROP TABLE %1;")
                       .arg(manTblName_));
        hasTable = false;
    }
    if (!hasTable)
        query.exec(QString("CREATE TABLE %1 ("
                           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                           "%2);")
                       .arg(manTblName_)
                       .arg(getFieldNames_(InstanceIO::getAllInfoName(), "TEXT")));

    for (auto& infoMap : infoMapList) {
        QString names, values;
        for (auto& name : infoMap.keys()) {
            names.append(name).append(',');
            values.append(QString("'%1',").arg(infoMap[name]));
        }
        names.remove(names.length() - 1, 1);
        values.remove(values.length() - 1, 1);
        query.exec(QString("INSERT INTO %1 (%2)"
                           "VALUES(%3);")
                       .arg(manTblName_)
                       .arg(names)
                       .arg(values));
    }

    QSqlDatabase::database().commit(); // 提交事务
}

QStringList Ecco::getECCO(const QString& eccoRowcols)
{
    static QList<QPair<QStringList, QRegularExpression>> eccoLib;
    if (eccoLib.isEmpty()) {
        QSqlQuery query;
        query.exec(QString("SELECT sort_id, sn, name, regstr FROM %1 "
                           "WHERE length(regstr) > 0 "
                           "ORDER BY sort_id DESC;")
                       .arg(libTblName_));
        while (query.next()) {
            eccoLib.append({ { query.value(TABLE_SN).toString(),
                                 query.value(TABLE_NAME).toString() },
                QRegularExpression(query.value(TABLE_REGSTR).toString()) });
        }
    }

    for (auto& eccoRec : eccoLib)
        if (eccoRowcols.contains(eccoRec.second))
            return eccoRec.first;

    return {};
}

QStringList Ecco::getECCO(PInstance ins)
{
    return getECCO(ins->getECCORowcols());
}

QList<PInstance> Ecco::getInsList_dir__(const QString& dirName)
{
    QList<PInstance> insList;
    dirName.at(0);
    return insList;
}

QList<PInstance> Ecco::getInsList_webfile__(const QString& insFileName)
{
    QList<PInstance> insList;
    insFileName.at(0);
    return insList;
}

QList<PInstance> Ecco::getInsList_db__(const QString& dbName, const QString& man_tblName)
{
    QList<PInstance> insList;
    dbName.at(0);
    man_tblName.at(0);
    return insList;
}

int Ecco::storeToDB__(QList<PInstance> insList, const QString& dbName, const QString& tblName)
{
    insList.end();
    dbName.at(0);
    tblName.at(0);
    return 0;
}
