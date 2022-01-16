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

constexpr QChar Forward { '+' }, Unforward { '-' };
constexpr QChar UnorderChar { '|' }, OrderChar { '*' }, ExceptChar { L'除' };

Ecco::Ecco()
{
    QString dbName { "output/data.db" }, tblName { "ecco" };
    dbName_ = dbName;
    libTblName_ = tblName;
}

bool Ecco::setECCO(PInstance ins)
{
    int count = regList_.count();
    if (count == 0)
        setRegExpList_();

    for (int i = 0; i < count; ++i) {
        QString rowcols = ins->getRowCols();
        if (regList_[i].match(rowcols).hasMatch()) {
            QSqlRecord record = table_.record(i);
            ins->setEcco(record.value(TABLE_SN).toString(), record.value(TABLE_NAME).toString());
            return true;
        }
    }

    return false;
}

bool Ecco::setECCO(QList<PInstance> insList)
{
    for (auto& pins : insList)
        setECCO(pins);

    return true;
}

void Ecco::initEccoLib()
{
    QString htmlStr;
    /*// 并发下载开局库的网页数据源
    QList<QString> urls;
    for (auto cindex : QString { "abcde" })
        urls.append(QString("https://www.xqbase.com/ecco/ecco_%1.htm").arg(cindex));
    htmlStr = Tools::downHtmlsFromUrls(urls);
#ifdef DEBUG
    qDebug() << __FUNCTION__ << __LINE__;
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
    qDebug() << __FUNCTION__ << __LINE__;
#endif

    // 设置记录内正则字符串
    setEccoRecordRegstrField_(eccoRecords);

    //  存入数据库文件
    restoreEccoRecord_(eccoRecords);
}

void Ecco::downXqbaseManual()
{
    QList<QString> htmlStrs;
    //*// 并发下载象棋百科棋谱库的网页数据源
    QList<QString> urls;
    int firstId = 100, endId = 110; // MAX:12141
    QString urlTemp { "https://www.xqbase.com/xqbase/?gameid=" };
    for (int id = firstId; id <= endId; ++id)
        urls.append(urlTemp + QString::number(id));

    htmlStrs = Tools::downHtmlsFromUrlsBlocking(urls);
#ifdef DEBUG
    Tools::writeTxtFile("output/manualHtml.txt", htmlStrs.join("\n\n\n"), QIODevice::WriteOnly);
#endif

    // 清洁网页字符串内容, 解析字符串，存入数据库
    QList<QPair<int, QString>> infoIndex_regStrs = {
        { SOURCE, R"(https%3A%2F%2Fwww.xqbase.com%2Fxqbase%2F%3Fgameid%3D(\d+?)&)" },
        { TITLE, R"(<title>(.*?)</title>)" },
        { EVENT, R"(>([^>]+赛[^>]*?)<)" },
        { BLACK, R"(>黑方 ([^<]*?)<)" },
        { RED, R"(>红方 ([^<]*?)<)" },
        { DATE, R"(>(\d+年\d+月(?:\d+日)?)(?: ([^<]*?))?<)" }, // SITE
        { ECCOSN, R"(>([A-E]\d{2})\. ([^<]*?)<)" }, // ECCONAME
        { MOVESTR, R"(<pre>\s*?(1\.[\s\S]*?)\((.+?)\)</pre\>)" } // RESULT
    };

    QString manualStr {};
    QTextStream stream(&manualStr);
    for (auto& htmlStr : htmlStrs) {
        InfoMap info { InstanceIO::getInitInfoMap() };
        for (auto& infoIndex_regStr : infoIndex_regStrs) {
            QRegularExpression reg(infoIndex_regStr.second, QRegularExpression::UseUnicodePropertiesOption);
            auto match = reg.match(htmlStr);

            int infoIndex = infoIndex_regStr.first;
            QString infoName { InstanceIO::getInfoName(infoIndex) };
            info[infoName] = match.captured(1);
            if (infoIndex == SOURCE)
                info[infoName].prepend(urlTemp);
            else if (infoIndex == MOVESTR)
                info[infoName].remove("\r\n");

            int infoIndex2 { (infoIndex == DATE ? SITE
                                                : (infoIndex == ECCOSN           ? ECCONAME
                                                        : (infoIndex == MOVESTR) ? RESULT
                                                                                 : -1)) };
            if (infoIndex2 != -1)
                info[InstanceIO::getInfoName(infoIndex2)] = match.captured(2);
        }

        for (auto& name : info.keys())
            stream << name << ':' << info[name] << '\n';
        stream << '\n';
    }
#ifdef DEBUG
    qDebug() << __FUNCTION__ << __LINE__;
    Tools::writeTxtFile("output/manualCleanHtml.txt", manualStr, QIODevice::WriteOnly);
#endif

    //*/
    //    htmlStr = Tools::readTxtFile("output/manualCleanHtml.txt"); // 读取已下载的文件
}

// 获取回合着法字符串
void Ecco::setBoutStrs_(BoutStrs& boutStrs, const QString& sn, const QString& mvstrs, bool isPreMvStrs)
{
    // 捕捉一步着法: 1.着法，2.可能的“不分先后”标识，3.可能的“此前...”着法
    QString rich_mvStr { QString(R"(((?:[%1]{4}(?:／[%1]{4})*)|……)(\s?\(不.先后[)，])?(?:[^\da-z]*?此前.*?走(.+?)\))?)")
                             .arg(Pieces::getZhChars()) };
    // 捕捉一个回合着法：1.序号，2-4.首着、“不分先后”、“此前...”，5-7.着法、“不分先后”、“此前...”
    QRegularExpression reg_bm = QRegularExpression(QString(R"(([\da-z]).\s?%1(?:，%1)?)").arg(rich_mvStr),
        QRegularExpression::UseUnicodePropertiesOption);
    QRegularExpressionMatchIterator matchIter = reg_bm.globalMatch(mvstrs);
    while (matchIter.hasNext()) {
        QRegularExpressionMatch match = matchIter.next();
        QChar boutNo = match.captured(1).at(0);
        for (int i = 2; i <= match.lastCapturedIndex(); ++i) {
            QString qstr { match.captured(i) };
            if (qstr.isEmpty())
                continue;

            int color = i / 5;
            if (i == 2 || i == 5) { // 回合着法
                if (qstr == "……")
                    continue;

                // B32 黑棋第一、二着的顺序有误
                if (sn == "B32" && color == 1 && boutNo < '3')
                    qstr = (boutNo == '1') ? "炮８平６" : "马８进７";
                // B21 棋子文字错误
                else if (sn == "B21") {
                    if (qstr.contains("象七进九"))
                        qstr.replace(L'象', L'相');
                    else if (qstr.contains("车８进５"))
                        boutNo = 'r';
                }

                // 已存在序号为boutNo的着法字符串
                if (boutStrs.find(boutNo) != boutStrs.end()) {
                    QString& mvstr = boutStrs[boutNo][color];
                    // 已存在着法字符串尾部不匹配查找到的字符串
                    if (!mvstr.endsWith(qstr)) {
                        if (!mvstr.isEmpty()) {
                            int orderIndex = mvstr.contains(OrderChar) ? mvstr.lastIndexOf(OrderChar) + 1 : 0;
                            mvstr.append(mvstr.length() - orderIndex == qstr.length() ? '/' : OrderChar);
                        }
                        mvstr.append(qstr);
                    }
                } else {
                    QStringList mvstrs { {}, {} };
                    mvstrs[color] = qstr;
                    boutStrs.insert(boutNo, mvstrs);
                }
#ifdef DEBUG
                streamBoutStrs << "\tBoutMoveStr:" + qstr + '\n'; // BoutMoveStr:1687? 1520?
#endif
            } else if (i == 3 || i == 6) { // "不分先后" i=3, 6
                for (QChar bNo : boutStrs.keys()) {
                    if (bNo > boutNo)
                        break;

                    for (QString& mvstrs : boutStrs[bNo]) {
                        if (!mvstrs.isEmpty())
                            mvstrs.append(UnorderChar);
                        if (bNo == boutNo && i == 3)
                            break;
                    }
                }
#ifdef DEBUG
                streamBoutStrs << "\tNotOrder:" + qstr + '\n'; // NotOrder:26个
#endif
            } else if (i == 4 || i == 7) { // "此前..."
                if (sn == "D41" && !isPreMvStrs)
                    // D41 前置字符串的“此前”部分，转移至前面颜色的字段
                    color = 0;
                else if (sn == "D29") {
                    // D29 先处理红棋着法 (此前红可走马八进七，黑可走马２进３、车９平４)
                    boutStrs[boutNo][0].prepend(qstr.left(4));
                    qstr.remove(0, 8);
                }
#ifdef DEBUG
                // PreInsertBout:53个     \tInsertBout:103个
                streamBoutStrs << (isPreMvStrs ? "\tPreInsertBout:" : "\tInsertBout:") + qstr + '\n';
#endif

                // 处理"此前..."着法描述字符串 去尾括号后比较，避免重复着法
                if (!boutStrs[boutNo][color].contains(qstr))
                    boutStrs[boutNo][color].prepend(qstr.append(OrderChar));
            }
        }
    }
}

QString Ecco::getRowcols_(const QString& mv, Instance& ins, bool isOther)
{
    PMove curMove = ins.appendMove_ecco(mv, isOther);
    if (curMove)
        return curMove->rowcols();

    return QString {};
}

QString Ecco::getRegStr_(const BoutStrs& boutStrs, Instance& ins, QMap<QString, QString>& mv_premv,
    QRegularExpression& reg_m, QRegularExpression& reg_notOrder, QRegularExpression& reg_order)
{
    ins.reset();
    QChar preBoutNo { '1' };
    QString anyMoveRegStr { R"((?:\d{4})*?)" };
    int boutNotOrderCount[] = { 0, 0 }, preMaxMoveNum[] = { 0, 0 };
    QString regStr[] = { {}, {} }, operateColorMoves[] = { {}, {} }, showColorMoveStr[] = { {}, {} };

    // 处理不分先后标记（最后回合，或本回合已无标记时调用）
    std::function<void(QString[COLORNUM], int[COLORNUM], int)>
        operateNotOrderChar_ = [](QString regStr[COLORNUM], int boutNotOrderCount[COLORNUM], int color) {
            // regStr的最后字符有标记，则去掉最后一个标记
            if (!regStr[color].isEmpty() && regStr[color].back() == UnorderChar)
                regStr[color].remove(regStr[color].length() - 1, 1);
            // 如果存在有一个以上标记，则包裹处理
            if (boutNotOrderCount[color] > 1)
                regStr[color] = QString("(?:%1){%2}").arg(regStr[color]).arg(boutNotOrderCount[color]);

            boutNotOrderCount[color] = 0;
        };

    for (QChar boutNo : boutStrs.keys()) {
        for (int color = 0; color < COLORNUM; ++color) {
            const QString& mvstrs = boutStrs[boutNo][color];
            // 处理不分先后标记（本回合已无标记）
            if ((boutNo == boutStrs.lastKey() && mvstrs.isEmpty())
                || (!mvstrs.isEmpty() && mvstrs.back() != UnorderChar))
                operateNotOrderChar_(regStr, boutNotOrderCount, color);
            if (mvstrs.isEmpty())
                continue;

            // 处理有顺序的的连续着法标记
            QString operateRowcols, showMoveStr;
            int maxMoveNum = 0;
            for (auto& mvstr : mvstrs.split(OrderChar)) {
                QRegularExpressionMatch matchNotOrder = reg_notOrder.match(mvstr),
                                        matchOrder = reg_order.match(mvstr);
                bool notFirstLast = mvstr.contains("不分先后");
                // 前进着数（默认一着），如匹配非顺序正则式则无前进着，如匹配“不分先后”则全部是前进着
                int index = 0, goIndex = matchNotOrder.hasMatch() ? -1 : (notFirstLast ? 1000 : 0);

                // 处理无顺序的连续着法
                QStringList rowcolsList;
                QRegularExpressionMatchIterator matchIter = reg_m.globalMatch(mvstr);
                while (matchIter.hasNext()) {
                    QString mv = matchIter.next().captured(1);
                    // 是否前进着法
                    bool isOrder = matchOrder.captured(1).contains(mv) && mv != "马八进七";
                    // 着法序号大于前进着法序号，且非前进着法，则为变着 (一般情况下，首着isOther=false)
                    bool isOther = index > goIndex && !isOrder;
                    QString rowcols = getRowcols_(mv, ins, isOther);
                    // 某着失败后先走前着
                    if (rowcols.isEmpty() && mv_premv.contains(mv)) {
                        ins.appendMove_ecco(mv_premv[mv], false);
                        rowcols = getRowcols_(mv, ins, false);
                        ins.backNext();
                        ins.backNext();
                    }
                    if (!rowcols.isEmpty()) {
                        rowcolsList.append(rowcols);
                        operateColorMoves[color].append(mv).append(isOther ? Unforward : Forward);
                        showMoveStr.append(mv).append(UnorderChar);
                        if (isOther)
                            ins.backOther();
                        ++index;
                    }
#ifdef DEBUG
                    else
                        streamBoutStrs << QString("\t\t\t失败: %1 %2. isOther:%3\n").arg(mv).arg(boutNo).arg(isOther);
#endif
                }

                QString groupRowcols = rowcolsList.join(UnorderChar);
                if (rowcolsList.length() > 1) {
                    // B30 "除...以外的着法" =>(?!%1) 否定顺序环视 见《精通正则表达式》P66.
                    groupRowcols = (mvstr.contains(ExceptChar)
                            ? QString("(?:(?!%1)%2)").arg(groupRowcols).arg(anyMoveRegStr)
                            : QString("(?:%1){%2%3}").arg(groupRowcols).arg(notFirstLast ? "" : "0,").arg(rowcolsList.length()));
                }

                // 按顺序添加着法组
                operateRowcols.append(groupRowcols);
                if (!showMoveStr.isEmpty())
                    showMoveStr.back() = L'　';
                maxMoveNum = qMax(maxMoveNum, rowcolsList.length());
            }

            // 根据回合序号差距添加任意着法
            if (boutNo.unicode() - preBoutNo.unicode() - preMaxMoveNum[color] > 0) {
#ifdef DEBUG
                streamBoutStrs << QString("\t\tAnyMove：%1-%2-%3\n")
                                      .arg(boutNo.unicode())
                                      .arg(preBoutNo.unicode())
                                      .arg(preMaxMoveNum[color]);
#endif
                operateRowcols.prepend(anyMoveRegStr);
                showMoveStr.prepend("……　");
            }
            preMaxMoveNum[color] = maxMoveNum;

            // 处理不分先后标记
            if (mvstrs.back() == UnorderChar) {
                operateRowcols.append(UnorderChar);
                ++boutNotOrderCount[color];
            }

            // 存入结果字符串
            regStr[color].append(operateRowcols);
            showColorMoveStr[color].append(showMoveStr);

            // 处理不分先后标记（最后回合且有标记）
            if (boutNo == boutStrs.lastKey())
                operateNotOrderChar_(regStr, boutNotOrderCount, color);
        }

        preBoutNo = boutNo;
    }

#ifdef DEBUG
    streamBoutStrs << QString("\t\t红：%1\n\t\t黑：%2\n").arg(operateColorMoves[0]).arg(operateColorMoves[1]);
    streamBoutStrs << QString("\t\t红：%1\n\t\t黑：%2\n").arg(showColorMoveStr[0]).arg(showColorMoveStr[1]);
#endif

    return QString("^%1%2-%3").arg(regStr[0]).arg(anyMoveRegStr).arg(regStr[1]);
}

void Ecco::setEccoRecordRegstrField_(QMap<QString, QStringList>& eccoRecords)
{
#ifdef DEBUG
    int recIndex {};
#endif

    QRegularExpression reg_pre { QRegularExpression("红方：(.+)\\s+黑方：(.+)",
        QRegularExpression::UseUnicodePropertiesOption) },
        reg_m { QRegularExpression(QString(R"(([%1]{4}))").arg(Pieces::getZhChars()),
            QRegularExpression::UseUnicodePropertiesOption) },
        reg_notOrder { QRegularExpression(
            "马二进一/车九平八"
            "|马二进三/马八进七"
            "|炮八平六/马八进九"
            "|车１平２/炮９平７"
            "|马８进７/车９进１"
            "|马８进７/马２进３"
            "|炮８进４/炮２进４",
            QRegularExpression::UseUnicodePropertiesOption) },
        reg_order { QRegularExpression(
            "(马二进三.+?车一平二(?:.+?车二进六)?"
            "|马８进７.+?车９平８)",
            QRegularExpression::UseUnicodePropertiesOption) };
    QMap<QString, QString> mv_premv {
        { "车一平二", "马二进三" },
        { "车二进六", "车一平二" },
        { "车八进四", "车九平八" },
        { "车九平八", "马八进七" },
        { "马三进四", "兵三进一" },
        { "马七进六", "兵七进一" },
        { "炮八平七", "卒３进１" },
        { "车８进５", "炮８平９" },
        { "车９平８", "马８进７" },
        { "炮８进２", "车二退六" }
    };

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
                // 处理前置着法描述字符串————"红方：黑方："
                boutStrs.insert('1', { match.captured(1), match.captured(2) });
#ifdef DEBUG
                streamBoutStrs << "\tRedBlack@:" + match.captured(1) + match.captured(2) + '\n'; // RedBlack@:40个
#endif
            } else {
                // 处理其他的前置着法描述字符串
                setBoutStrs_(boutStrs, sn, pre_mvstrs, true);
#ifdef DEBUG
                streamBoutStrs << "\tOterPre@:" + pre_mvstrs + '\n'; // OterPre@:75个
#endif
            }
        }

        // 处理着法字符串
        setBoutStrs_(boutStrs, sn, mvstrs, false);
#ifdef DEBUG
        for (auto boutNo : boutStrs.keys())
            streamBoutStrs << '\t' << boutNo << ". " << boutStrs[boutNo].join("\t\t") << '\n';
#endif

        // 设置着法正则描述字符串
        record[REC_REGSTR] = getRegStr_(boutStrs, ins, mv_premv, reg_m, reg_notOrder, reg_order);
#ifdef DEBUG
        streamBoutStrs << "\t\tregstr: " + record[REC_REGSTR] + "\n\n";
        Tools::writeTxtFile(boutStrsFileName, eccoBoutStrs, QIODevice::WriteOnly);
#endif
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

void Ecco::restoreEccoRecord_(QMap<QString, QStringList>& eccoRecords)
{
    QFile file(dbName_);
    file.open(QIODevice::WriteOnly);
    file.close();

    QSqlDatabase DB;
    DB = QSqlDatabase::addDatabase("QSQLITE");
    DB.setDatabaseName(dbName_);
    if (!DB.open())
        return;

    QSqlQuery query;
    query.exec(QString("CREATE TABLE %1 ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT,"
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

    //    QSqlDatabase::database().transaction(); // 启动事务
    //    QSqlDatabase::database().commit(); // 提交事务
}

void Ecco::setRegExpList_()
{
    QSqlDatabase DB;
    DB = QSqlDatabase::addDatabase("QSQLITE");
    DB.setDatabaseName(dbName_);
    if (!DB.open())
        return;

    table_.setTable(libTblName_);
    table_.setFilter("length(regstr) > 0");
    table_.setSort(TABLE_SN, Qt::SortOrder::DescendingOrder); // sn列降序
    table_.select();
    for (int i = 0; i < table_.rowCount(); ++i) {
        QSqlRecord record = table_.record(i);
        regList_.append(QRegularExpression(record.value(TABLE_REGSTR).toString()));
    }
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
