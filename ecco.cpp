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
    PRE_MVSTRS_I,
    REGSTR_I
};

static QString boutStrsFileName { "output/boutStrs.txt" };

// '-','／': 不前进，加‘|’   '*': 前进，加‘|’    '+': 前进，不加‘|’
constexpr QChar Forward { '+' }, ForwardOr { '*' },
    NotForwardOr { '-' }, NotForwardOr_alias { L'／' }, ExceptNotForwardOr { L'除' }, BlankChar { ' ' };

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
    qDebug() << __FUNCTION__ << __LINE__;
    // 并发下载开局库的网页数据源
    QList<QString> urls;
    for (auto cindex : QString { "abcde" })
        urls.append(QString("https://www.xqbase.com/ecco/ecco_%1.htm").arg(cindex));
    QString htmlStr = Tools::downHtmlsFromUrls(urls);
#ifdef DEBUG
    Tools::writeTxtFile("output/eccoHtml.txt", htmlStr, QIODevice::WriteOnly);
#endif

    qDebug() << __FUNCTION__ << __LINE__;
    // 清洁网页字符串内容
    htmlStr.remove(QRegularExpression("</?(?:div|font|img|strong|center|meta|dl|dt|table|tr|td|em|p|li|dir"
                                      "|html|head|body|title|a|b|tbody|script|br|span)[^>]*>",
        QRegularExpression::UseUnicodePropertiesOption));
    htmlStr.replace(QRegularExpression(R"((\n|\r)\s+)", QRegularExpression::UseUnicodePropertiesOption), " ");
#ifdef DEBUG
    Tools::writeTxtFile("output/eccoCleanHtml.txt", htmlStr, QIODevice::WriteOnly);
#endif

    // 提取开局内容至结构化记录
    QMap<QString, QStringList> eccoRecords;
    setEccoRecord_(eccoRecords, htmlStr);
#ifdef DEBUG
    QString qstr {};
    QTextStream stream(&qstr);
    int no2 = 0;
    for (auto& record : eccoRecords) {
        QString sn { record[SN_I] }, mvstrs { record[MVSTRS_I] };
        if (sn.length() != 3 || mvstrs.isEmpty())
            continue;

        stream << no2++ << '.';
        for (auto& str : record)
            if (!str.isEmpty())
                stream << str << '\n';
        stream << "\n";
    }
    // 已与原始网页，核对完全一致！
    Tools::writeTxtFile("output/records.txt", qstr, QIODevice::WriteOnly);
    qDebug() << "setEccoLibField: " << eccoRecords.size();
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
void InitEcco::insertBoutStr_(BoutStrs& boutStrs, QChar boutNo, int color, QString mvstrs,
    QRegularExpression reg_m, QRegularExpression reg_bp)
{
    if (boutStrs.find(boutNo) == boutStrs.end())
        boutStrs.insert(boutNo, QList<QString> { {}, {} });

    QString mvstr_result {};
    // 处理一串着法字符串内存在先后顺序的着法
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
        mvstr_result.append((mvstr == "马八进七" ? ForwardOr : Forward) + mvstr);
        mvstrs.replace(match.capturedStart(i), mvstr.length(), QString(mvstr.length(), BlankChar));
    }

    // 处理连续描述的着法
    QRegularExpressionMatchIterator matchIter = reg_m.globalMatch(mvstrs);
    while (matchIter.hasNext())
        mvstr_result.append(NotForwardOr + matchIter.next().captured(1));

    // B30 "除...以外的着法"
    if (mvstrs.contains(ExceptNotForwardOr))
        mvstr_result.append(ExceptNotForwardOr);

    boutStrs[boutNo][color] = mvstr_result;
}

// 获取回合着法字符串数组
void InitEcco::setBoutStrs_(BoutStrs& boutStrs, const QString& sn, const QString& mvstrs, bool isBefore,
    QRegularExpression reg_m, QRegularExpression reg_bm, QRegularExpression reg_bp)
{
    QRegularExpressionMatchIterator matchIter = reg_bm.globalMatch(mvstrs);
    while (matchIter.hasNext()) {
        QRegularExpressionMatch match = matchIter.next();
        QChar boutNo = mvstrs[match.capturedStart(1)];
        if (isBefore && boutNo.isLower())
            boutNo = boutNo.toUpper();

        for (int i = 2; i <= match.lastCapturedIndex(); ++i) {
            QString qstr { match.captured(i) };
            if (qstr.isEmpty())
                continue;

            int color = i / 5;
            qstr.replace(NotForwardOr_alias, NotForwardOr);
            if ((i == 2 || i == 5) && qstr != "……") { // 回合的着法
                // D41 该部分字段的着法与前置部分重复
                if (color == 1 && !isBefore && sn == "D41")
                    continue;

                // B32 第一、二着黑棋的顺序有误
                if (color == 1 && boutNo < '3' && sn == "B32")
                    qstr = (boutNo == '1') ? "炮８平６" : "马８进７";
                // B21 棋子文字错误
                else if (qstr == "象七进九")
                    qstr[0] = L'相';

                if (boutStrs.find(boutNo) != boutStrs.end()) {
                    QString& mvstr = boutStrs[boutNo][color];
                    if (!mvstr.contains(qstr)) {
                        if (!mvstr.isEmpty())
                            mvstr[0] = NotForwardOr;
                        mvstr.append((mvstr.isEmpty() ? Forward : NotForwardOr) + qstr);
                    }
                } else {
                    QList<QString> mvstrs { {}, {} };
                    mvstrs[color] = Forward + qstr;
                    boutStrs.insert(boutNo, mvstrs);
                }
            } else if (i == 3 || i == 6) { // "不分先后" i=3, 6
                for (QChar bNo : boutStrs.keys()) {
                    if (bNo > boutNo)
                        break;

                    for (QString& mvstrs : boutStrs[bNo])
                        if (mvstrs.length() == MOVESTR_LEN + 1)
                            mvstrs[0] = ForwardOr;
                }
            } else if (i == 4 || i == 7) { // "此前..."
                QChar insertBoutNo = boutNo.unicode() - 'n' + 'g'; // 往前7个字符
                if (sn == "D41" && !isBefore)
                    color = 0; // D41 转移至前面颜色的字段
                if (sn == "D29") {
                    // D29 先处理红棋着法 (此前红可走马八进七，黑可走马２进３、车９平４)
                    insertBoutStr_(boutStrs, insertBoutNo, 0, "此前红可走马八进七", reg_m, reg_bp);
                    insertBoutStr_(boutStrs, insertBoutNo, 1, "黑可走马２进３、车９平４", reg_m, reg_bp);
                }

                // 处理"此前..."着法描述字符串
                insertBoutStr_(boutStrs, insertBoutNo, color, qstr, reg_m, reg_bp);
            }
        }
    }
}

QString InitEcco::getIccses_(const QString& mvstrs, Instance& ins)
{
    QString iccses {};
    int count { 0 }, groupCount { 1 }; // 分组数
    for (int i = 0; i < mvstrs.length() - 1; i += MOVESTR_LEN + 1) {
        QString mvstr { mvstrs.mid(i, MOVESTR_LEN + 1) };
        // '-','／': 不前进，加‘|’   '*': 前进，加‘|’    '+': 前进，不加‘|’
        bool isOther = mvstr[0] == NotForwardOr,
             split = isOther || mvstr[0] == ForwardOr,
             // 添加着法，并前进至此着法
            succes = ins.appendMove_ecco(mvstr.mid(1), isOther);
        if (succes) {
            ++count;
            if (split && i > 0) {
                if (groupCount > 0)
                    iccses.append('|');
                ++groupCount;
            }

            iccses.append(ins.getCurMove()->iccs());
            if (isOther)
                ins.backOther();
        } else {
            --groupCount;

#ifdef DEBUG
            Tools::writeTxtFile(boutStrsFileName, "\t\t失败:" + mvstr + '\n', QIODevice::Append);
#endif
        }
    }
    if (mvstrs.back() == ExceptNotForwardOr)
        iccses = "(?:(?!" + iccses + ").*)"; // 否定顺序环视 见《精通正则表达式》P66.
    else {
        if (count > 1)
            iccses = "(?:" + iccses + ")";
        if (groupCount > 1)
            iccses.append((mvstrs[0] == NotForwardOr ? "{1," : "{") + QString::number(groupCount) + "}");
    }

#ifdef DEBUG
    Tools::writeTxtFile(boutStrsFileName,
        QString("\tmvstrs:%1 %2 {%3}\n").arg(mvstrs).arg(iccses).arg(groupCount), QIODevice::Append);
#endif
    return iccses;
}

// 只考虑一种基本情形. 某局面配对开局正则表达式时，转换成红底且左右对称两种情形进行匹配
QString InitEcco::getRegStr_(const BoutStrs& boutStrs, Instance& ins)
{
    ins.reset();
    // C11~C14=>"车一平四／车一平六" D40~D43=>"车８进５", 做备份以便后续单独处理
    QList<QString> regStr { {}, {} }, mvstrs_bak { "-车一平四-车一平六", "-车８进５" };
    for (int color = 0; color < COLORNUM; ++color) {
        bool handleMvstrs_bak = false;
        for (QChar boutNo : boutStrs.keys()) {
            const QString& mvstrs = boutStrs[boutNo][color];
            if (mvstrs.isEmpty())
                continue;

            // 处理“此前...”着法(每次单独处理) C11~C14=>"车一平四" D40~D43=>"车８进５"
            if (handleMvstrs_bak) {
                regStr[color].append(getIccses_(mvstrs_bak[color], ins));
                if (boutNo < 'n') // C11 bout==62时不回退
                    ins.backToPre(); // 回退至前着，避免重复走最后一着
            } else if (mvstrs.contains(mvstrs_bak[color]))
                handleMvstrs_bak = true;

            regStr[color].append(getIccses_(mvstrs, ins));
        }
    }

    QString blackRegStr;
    if (!regStr[1].isEmpty())
        blackRegStr = "&" + regStr[1];

    return QString("(?:^%1.*?%2)").arg(regStr[0]).arg(blackRegStr);
}

void InitEcco::setEccoRecordRegstrField_(QMap<QString, QStringList>& records)
{
    QString ZhWChars { Pieces::getZhChars() },
        // 着法中文字符组
        mvstr { "([" + ZhWChars + "]{4})" },
        // 着法，含多选的复式着法
        mvStrs { "(?:[" + ZhWChars + "]{4}(?:／[" + ZhWChars + "]{4})*)" },
        // 捕捉一步着法: 1.着法，2.可能的“此前...”着法，3. 可能的“／\\n...$”着法
        rich_mvStr { "(" + mvStrs + "|……)(?:[，、；\\s　和\\(\\)以／]|$)(\\(?不.先后[\\)，])?([^"
            + ZhWChars + "]*?此前[^\\)]+?\\))?" },
        // 捕捉一个回合着法：1.序号，2.一步着法的首着，3-5.着法或“此前”，4-6.着法或“此前”或“／\\n...$”
        bout_rich_mvStr { "([\\da-z]).[ \\n\\r\\s]?" + rich_mvStr + "(?:" + rich_mvStr + ")?" };

    QRegularExpression regs[] = {
        QRegularExpression(mvstr, QRegularExpression::UseUnicodePropertiesOption),
        QRegularExpression(bout_rich_mvStr, QRegularExpression::UseUnicodePropertiesOption),
        QRegularExpression("红方：(.+)\\s+黑方：(.+)", QRegularExpression::UseUnicodePropertiesOption),
        QRegularExpression("(炮二平五)?.(马二进三).*(车一平二).(车二进六)?"
                           "|(马８进７).+(车９平８)"
                           "|此前可走(马二进三)、(马八进七)、(兵三进一)和(兵七进一)",
            QRegularExpression::UseUnicodePropertiesOption)
    };

#ifdef DEBUG
    Tools::writeTxtFile(boutStrsFileName, "", QIODevice::WriteOnly);
    int no {};
#endif
    Instance ins;
    for (auto& record : records) {
        QString sn { record[SN_I] }, mvstrs { record[MVSTRS_I] };
        if (sn.length() != 3 || mvstrs.isEmpty())
            continue;

        BoutStrs boutStrs {};
        // 处理前置着法字符串
        QString pre_mvstrs { record[PRE_MVSTRS_I] };
        if (!pre_mvstrs.isEmpty()) {
            QRegularExpressionMatch match = regs[2].match(pre_mvstrs);
            if (match.hasMatch()) {
                // 处理前置着法描述字符串————"局面开始：红方：黑方："
                insertBoutStr_(boutStrs, '1', 0, match.captured(1), regs[0], regs[3]);
                insertBoutStr_(boutStrs, '1', 1, match.captured(2), regs[0], regs[3]);
            } else
                // 处理其他的前置着法描述字符串
                setBoutStrs_(boutStrs, sn, pre_mvstrs, true, regs[0], regs[1], regs[3]);
        }

        // 处理着法字符串
        setBoutStrs_(boutStrs, sn, mvstrs, false, regs[0], regs[1], regs[3]);

#ifdef DEBUG
        QString qstr {};
        QTextStream stream(&qstr);
        stream << "No." << no++ << "\n";
        for (auto& str : record)
            stream << str << ' ';
        stream << "\n";
        for (auto boutNo : boutStrs.keys()) {
            stream << boutNo << '.';
            for (auto& mvstrs : boutStrs[boutNo]) {
                stream << mvstrs << ' ';
            }
            stream << '\n';
        }
        Tools::writeTxtFile(boutStrsFileName, qstr, QIODevice::Append);
#endif

        // 设置着法正则描述字符串
        record[REGSTR_I] = getRegStr_(boutStrs, ins);

#ifdef DEBUG
        Tools::writeTxtFile(boutStrsFileName, "\tregstr:" + record[REGSTR_I] + "\n\n", QIODevice::Append);
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
    // int no = 1;
    for (auto& record : eccoRecords) {
        QString sn { record[SN_I] }, mvstrs { record[MVSTRS_I] };
        if (sn.length() != 3 || mvstrs.length() < 3)
            continue;

        QString sn_pre {};
        // 三级局面的 C2 C3_C4 C61 C72局面 有40项
        if (mvstrs[0] == L'从') {
            sn_pre = mvstrs.mid(1, 2);
            // 前置省略的着法 有75项
        } else if (mvstrs[0] != L'1') {
            //  截断为两个字符长度
            sn_pre = sn.left(2);
            if (sn_pre[0] == L'C')
                sn_pre[1] = L'0'; // C0/C1/C5/C6/C7/C8/C9 => C0
            else if (sn_pre == "D5")
                sn_pre = "D51"; // D5 => D51
        } else
            continue;

        // 利用QMap的字符串定位查找功能
        record[PRE_MVSTRS_I] = eccoRecords[sn_pre][MVSTRS_I];
        // qDebug() << no++ << sn << record[PRE_MVSTRS_I] << record[MVSTRS_I];
    }
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
