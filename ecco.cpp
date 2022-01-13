#include "ecco.h"
#include "instance.h"
#include "move.h"
#include "piece.h"
#include "seat.h"
#include "tools.h"
#include <QDebug>
#include <QFile>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <functional>

enum RecordIndex {
    SN_I,
    NAME_I,
    NUMS_I,
    MVSTRS_I,
    PRE_SN_I,
    REGSTR_I
};

#ifdef DEBUG
static QString boutStrsFileName { "output/eccoBoutStrs.txt" };
QString eccoBoutStrs;
QTextStream streamBoutStrs(&eccoBoutStrs);
#endif

// '+': 前进，不加‘|’   '*': 前进，加‘|’  '-','／': 不前进，加‘|’
constexpr QChar Forward { '+' }, Forward_Or { '*' }, NotForward_Or { '-' };
constexpr QChar NotOrderChar { '|' }, OrderChar { '*' }, ExceptChar { L'除' };

Ecco::Ecco(const QString& libFileName)
{
    QString libStr { Tools::readTxtFile(libFileName) };
    // setEcco_someField__(libStr);
    // setEcco_regstrField__();
}

Ecco::Ecco(const QString& dbName, const QString& tblName)
{
    // setRegList_db__(dbName, tblName);
    dbName + tblName;
}

bool Ecco::setECCO(PInstance ins) const
{
    //    ins->go();
    return true;
}

bool Ecco::setECCO(QList<PInstance> insList) const
{
    for (auto& pins : insList)
        setECCO(pins);

    return true;
}

void InitEcco::initEccoLib()
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
}

void InitEcco::handleInstanceHtmlStr(const QString& downedStr)
{
    qDebug() << "InitEcco::handleInstanceDownStr()";
    if (downedStr.isEmpty())
        return;

    // 解析字符串，存入数据库
    QMap<QString, QString> regStr = {
        { "\\<title\\>(.*)\\</title\\>", "TITLE" },
        { "\\>([^\\>]+赛[^\\>]*)\\<", "EVENT" },
        { "\\>黑方 ([^\\<]*)\\<", "BLACK" },
        { "\\>红方 ([^\\<]*)\\<", "RED" },
        { "\\>(\\d+年\\d+月(?:\\d+日)?)(?: ([^\\<]*))?\\<", "DATE" }, // SITE_INDEX 3
        { "\\>([A-E]\\d{2})\\. ([^\\<]*)\\<", "ECCOSN" }, // ECCONAME_INDEX 16
        { "\\<pre\\>\\s*(1\\.[\\s\\S]*?)\\((.+)\\)\\</pre\\>", "MOVESTR" } // RESULT_INDEX 10
    };
    InfoMap info { Instance::getInitInfoMap() };
    info["SOURCE"] = downedStr.left(downedStr.indexOf(QChar('\n')));
    for (auto& key : regStr.keys()) {
        QRegularExpression reg(key, QRegularExpression::UseUnicodePropertiesOption);
        auto match = reg.match(downedStr);
        if (!match.hasMatch())
            continue;

        QString value { match.captured(1) };
        QString infoKey { regStr[key] };
        if (infoKey == "MOVESTR")
            value.replace("\r\n", " ");
        info[infoKey] = value;
        QString infoKey2 { (infoKey == "DATE" ? "SITE"
                                              : (infoKey == "ECCOSN"           ? "ECCONAME"
                                                      : (infoKey == "MOVESTR") ? "RESULT"
                                                                               : "")) };
        if (!infoKey2.isEmpty()) {
            info[infoKey2] = match.captured(2);
        }
    }

    // Tools::writeTxtFile("test.txt", downedStr, QIODevice::Append);
    QString qstr {};
    QTextStream stream(&qstr);
    for (auto& key : info.keys())
        stream << key << ':' << info[key] << '\n';
    stream << '\n';
    Tools::writeTxtFile("instance.txt", qstr, QIODevice::Append);

    //    emit instanceFinished();
}

// 将着法描述组合成着法字符串列表（“此前...”，“...不分先后”）
void InitEcco::insertBoutStr_(BoutStrs& boutStrs, QChar boutNo, int color, QString mvstrs)
{
    // 没有此回合序号的着法字符串，则添加
    if (boutStrs.find(boutNo) == boutStrs.end())
        boutStrs.insert(boutNo, QStringList { {}, {} });

    QString mvstr_result {};
    // 处理一串着法字符串内存在先后顺序的着法
    QRegularExpression reg_bp = QRegularExpression("(炮二平五)?.(马二进三).*(车一平二).(车二进六)?"
                                                   "|(马８进７).+(车９平８)"
                                                   "|此前可走(马二进三)、(马八进七)、(兵三进一)和(兵七进一)",
        QRegularExpression::UseUnicodePropertiesOption);
    QRegularExpressionMatch match = reg_bp.match(mvstrs);
    for (int i = 1; i <= match.lastCapturedIndex(); ++i) {
        QString mvstr { match.captured(i) };
        if (mvstr.isEmpty())
            continue;

        // C83 可选着法"车二进六"不能加入顺序着法，因为需要回退，以便解决与后续“炮８进２”的冲突
        if (mvstr == "车二进六"
            && mvstrs.contains("红此前可走马二进三、马八进七、车一平二、车二进六、兵三进一和兵七进一"))
            continue;

        // B22~B24, D42,D52~D55 "马八进七"需要前进，又需要加"|"
        mvstr_result.append(mvstr.prepend(mvstr == "马八进七" ? Forward_Or : Forward));
        mvstrs.replace(match.capturedStart(i), mvstr.length(), QString(mvstr.length(), ' '));
    }

    // 处理连续描述的着法
    QRegularExpression reg_m = QRegularExpression(QString(R"(([%1]{4}))").arg(Pieces::getZhChars()),
        QRegularExpression::UseUnicodePropertiesOption);
    QRegularExpressionMatchIterator matchIter = reg_m.globalMatch(mvstrs);
    while (matchIter.hasNext())
        mvstr_result.append(NotForward_Or + matchIter.next().captured(1));

    // B30 "除...以外的着法"
    if (mvstrs.contains(ExceptChar))
        mvstr_result.append(ExceptChar);

    if (!boutStrs[boutNo][color].contains(mvstr_result))
        // 添加插入着法的括号标记
        boutStrs[boutNo][color].prepend(QString("{%1}").arg(mvstr_result));
}

// 获取回合着法字符串
void InitEcco::setBoutStrs_(BoutStrs& boutStrs, const QString& sn, const QString& mvstrs, bool isPreMvStrs)
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
                else if (sn == "B21" && qstr.contains("象七进九"))
                    qstr.replace(L'象', L'相');

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
                            mvstrs.append(NotOrderChar);
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

QString InitEcco::getIccses_(bool isSkip, const QString& mvstrs, Instance& ins)
{

    // C11~C14=>"车一平四／车一平六" D40~D43=>"车８进５", 做备份以便后续单独处理
    //, mvstrs_bak[] = { "-车一平四-车一平六", "-车８进５" };
    //        bool handleMvstrs_bak = false;

    // 处理“此前...”着法(每次单独处理) C11~C14=>"车一平四" D40~D43=>"车８进５"
    //            if (handleMvstrs_bak) {
    //                regStr[color].append(getIccses_(isSkip, mvstrs_bak[color], ins));
    //                if (boutNo < 'n') // C11 bout==62时不回退
    //                    ins.backToPre(); // 回退至前着，避免重复走最后一着
    //            } else if (mvstrs.contains(mvstrs_bak[color]))
    //                handleMvstrs_bak = true;

    //            regStr[color].append(getIccses_(isSkip, mvstrs, ins));

    QString iccses {};
    int count { 0 }, groupCount { 1 }; // 分组数
    for (int i = 0; i < mvstrs.length() - 1; i += MOVESTR_LEN + 1) {
        QString mvstr { mvstrs.mid(i, MOVESTR_LEN + 1) };
        // '-','／': 不前进，加‘|’   '*': 前进，加‘|’    '+': 前进，不加‘|’
        bool isOther = mvstr[0] == NotForward_Or,
             // 是否要分组
            splitGroup = (isOther || mvstr[0] == Forward_Or) && i > 0;

        // 添加着法，并前进至此着法
        if (ins.appendMove_ecco(mvstr.mid(1), isOther)) {
            ++count;
            if (splitGroup) {
                if (!iccses.isEmpty())
                    iccses.append('|');
                ++groupCount;
            }

            //            iccses.append(ins.getCurMove()->iccs());
            iccses.append(ins.getCurMove()->rowcols());
            if (isOther)
                ins.backOther();
        } else {
            // 添加着法不成功
            if (splitGroup)
                --groupCount;
#ifdef DEBUG
            streamBoutStrs << "\t\t\t失败: " + mvstr + '\n';
#endif
        }
    }

    if (mvstrs.back() == ExceptChar)
        iccses = QString("(?:(?!%1).*)").arg(iccses); // 否定顺序环视 见《精通正则表达式》P66.
    else {
        if (count > 1)
            iccses = QString("(?:%1)").arg(iccses);

        if (groupCount > 1)
            iccses.append(QString("{%1%2}").arg(mvstrs[0] == NotForward_Or ? "1," : "").arg(groupCount));
    }
    // 添加任意着法
    if (isSkip)
        //        iccses.prepend(R"((?:\w\d\w\d)*)");
        iccses.prepend(R"((?:\d{4})*)");

#ifdef DEBUG
    streamBoutStrs << QString("\t\tmvstrs: %1 %2 {%3}\n").arg(mvstrs).arg(iccses).arg(groupCount);
#endif
    return iccses;
}

QString InitEcco::getRowcols_(const QString& mv, Instance& ins, bool isOther)
{
    QString rowcols;
    PMove curMove = ins.appendMove_ecco(mv, isOther);
    if (curMove)
        rowcols = curMove->rowcols();
#ifdef DEBUG
    else
        streamBoutStrs << QString("\t\t\t失败: %1 %2\n").arg(mv).arg(isOther);
#endif

    return rowcols;
}

QString InitEcco::getRegStr_(const BoutStrs& boutStrs, Instance& ins, QRegularExpression& reg_m)
{
    ins.reset();
    QString regStr[] = { {}, {} }, anyMoveRegStr { R"((?:\d{4})*?)" };
    QChar preBoutNo { '1' };
    int boutNotOrderCount[COLORNUM] = { 0, 0 };
    for (QChar boutNo : boutStrs.keys()) {
        QString colorRowcols[COLORNUM] = { {}, {} };
        int maxMoveNum[COLORNUM] = { 0, 0 };
        for (int color = 0; color < COLORNUM; ++color) {
            const QString& mvstrs = boutStrs[boutNo][color];
            // 处理不分先后标记（本回合已无标记）
            if (mvstrs.isEmpty() || mvstrs.back() != NotOrderChar) {
                if (!regStr[color].isEmpty() && regStr[color].back() == '|')
                    regStr[color].remove(regStr[color].length() - 1, 1);
                if (boutNotOrderCount[color] > 1)
                    regStr[color] = QString("(?:%1){%2}")
                                        .arg(regStr[color])
                                        .arg(boutNotOrderCount[color]);
                boutNotOrderCount[color] = 0;
            }
            if (mvstrs.isEmpty())
                continue;

            // 处理有顺序的的连续着法标记
            for (auto& mvstr : mvstrs.split(OrderChar)) {
                // 处理无顺序的连续着法
                QStringList rowcolsList;
                QRegularExpressionMatchIterator matchIter = reg_m.globalMatch(mvstr);
                while (matchIter.hasNext()) {
                    QString mv = matchIter.next().captured(1);
                    QString rowcols = getRowcols_(mv, ins, true);
                    if (!rowcols.isEmpty())
                        rowcolsList.append(rowcols);
                }
                QString groupRowcols = rowcolsList.join('|');
                if (rowcolsList.length() > 1) {
                    // B30 "除...以外的着法" =>(?!%1) 否定顺序环视 见《精通正则表达式》P66.
                    groupRowcols = (mvstr.contains(ExceptChar)
                            ? QString("(?:(?!%1)%2)").arg(groupRowcols).arg(anyMoveRegStr)
                            : QString("(?:%1){1,%2}").arg(groupRowcols).arg(rowcolsList.length()));
                }

                // 按顺序添加着法组
                colorRowcols[color].append(groupRowcols);
                maxMoveNum[color] = qMax(maxMoveNum[color], rowcolsList.length());
            }

            // 根据回合序号差距添加任意着法
            if (boutNo.unicode() - preBoutNo.unicode() - maxMoveNum[color] > 0)
                colorRowcols[color].prepend(anyMoveRegStr);

            // 处理不分先后标记
            if (mvstrs.back() == NotOrderChar) {
                colorRowcols[color].append('|');
                ++boutNotOrderCount[color];
            }

            // 存入结果字符串
            regStr[color].append(colorRowcols[color]);
        }

        preBoutNo = boutNo;
    }

    return QString("^%1%2-%3").arg(regStr[0]).arg(anyMoveRegStr).arg(regStr[1]);
}

void InitEcco::setEccoRecordRegstrField_(QMap<QString, QStringList>& eccoRecords)
{
#ifdef DEBUG
    int recIndex {};
#endif

    QRegularExpression reg_pre { QRegularExpression("红方：(.+)\\s+黑方：(.+)",
        QRegularExpression::UseUnicodePropertiesOption) },
        reg_m { QRegularExpression(QString(R"(([%1]{4}))").arg(Pieces::getZhChars()),
            QRegularExpression::UseUnicodePropertiesOption) };
    Instance ins;
    for (auto& record : eccoRecords) {
        QString sn { record[SN_I] }, mvstrs { record[MVSTRS_I] };
        if (sn.length() != 3 || mvstrs.isEmpty())
            continue;

#ifdef DEBUG
        streamBoutStrs << ++recIndex << "." << record.join("\n>> ") << "\n";
#endif
        BoutStrs boutStrs {};
        // 处理前置着法字符串
        if (!record[PRE_SN_I].isEmpty()) {
            QString pre_mvstrs = eccoRecords[record[PRE_SN_I]][MVSTRS_I];
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
        record[REGSTR_I] = getRegStr_(boutStrs, ins, reg_m);
#ifdef DEBUG
        streamBoutStrs << "\t\tregstr: " + record[REGSTR_I] + "\n\n";
        Tools::writeTxtFile(boutStrsFileName, eccoBoutStrs, QIODevice::WriteOnly);
#endif
    }
}

void InitEcco::setEccoRecord_(QMap<QString, QStringList>& eccoRecords, const QString& cleanHtmlStr)
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
                eccoRecords[capTexts[1]][MVSTRS_I] = capTexts[2];
            else {
                // field: SN_I, NAME_I, NUMS_I, MVSTRS_I, PRE_MVSTRS_I, REGSTR_I
                QStringList record { "", "", "", "", "", "" };
                for (int fIndex = SN_I; fIndex < capTexts.length() - 1; ++fIndex) {
                    int index = fIndex;
                    if (gIndex == 2 && fIndex > 1)
                        index = (fIndex == 3 ? NUMS_I : MVSTRS_I);
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
        QString sn { record[SN_I] }, mvstrs { record[MVSTRS_I] };
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

        record[PRE_SN_I] = sn_pre;
#ifdef DEBUG
        stream << no++ << ". " << sn_pre << '@' << sn << " " << mvstrs << '\n';
#endif
    }
#ifdef DEBUG
    Tools::writeTxtFile("output/eccoPreMvstrs.txt", qstr, QIODevice::WriteOnly);
#endif
}

QList<PInstance> InitEcco::getInsList_dir__(const QString& dirName)
{
    QList<PInstance> insList;
    dirName.at(0);
    return insList;
}

QList<PInstance> InitEcco::getInsList_webfile__(const QString& insFileName)
{
    QList<PInstance> insList;
    insFileName.at(0);
    return insList;
}

QList<PInstance> InitEcco::getInsList_db__(const QString& dbName, const QString& man_tblName)
{
    QList<PInstance> insList;
    dbName.at(0);
    man_tblName.at(0);
    return insList;
}

int InitEcco::storeToDB__(QList<PInstance> insList, const QString& dbName, const QString& tblName)
{
    insList.end();
    dbName.at(0);
    tblName.at(0);
    return 0;
}
