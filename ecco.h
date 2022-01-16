#ifndef ECCO_H
#define ECCO_H

#include <QList>
#include <QMap>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QSqlRecord>
#include <QString>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlTableModel>

class Instance;
using PInstance = Instance*;

#define MOVESTR_LEN 4

using BoutStrs = QMap<QChar, QStringList>;

// 一种基本情形. 某局面配对开局正则表达式时，通过交换颜色或旋转转换成红底、
// 左右对称转换两种情形进行匹配（匹配结果增加是否交换或是否旋转、是否左右对称交换两个信息）
class Ecco : public QObject {

public:
    Ecco();
    ~Ecco();

    // 设置棋谱对象的开局名称
    bool setECCO(PInstance ins);
    bool setECCO(QList<PInstance> insList);

    void initEccoLib();
    void downXqbaseManual();

private:
    // 初始化开局库的辅助函数
    static void setBoutStrs_(BoutStrs& boutStrs, const QString& sn, const QString& mvstrs, bool isPreMvStrs);
    static QString getRowcols_(const QString& mv, Instance& ins, bool isOther);
    static QString getRegStr_(const BoutStrs& boutStrs, Instance& ins, QMap<QString, QString>& mv_premv,
        QRegularExpression& reg_m, QRegularExpression& reg_notOrder, QRegularExpression& reg_order);
    static void setEccoRecordRegstrField_(QMap<QString, QStringList>& eccoRecords);
    static void setEccoRecord_(QMap<QString, QStringList>& eccoRecords, const QString& cleanHtmlStr);
    void restoreEccoRecord_(QMap<QString, QStringList>& eccoRecords);

    // 提取开局库内容建立正则对象
    void activeEccoQuery_();
    QPair<QString, QString> getECCO(PInstance ins);

    // 获取棋谱对象链表
    static QList<PInstance> getInsList_dir__(const QString& dirName);
    static QList<PInstance> getInsList_webfile__(const QString& insFileName);
    static QList<PInstance> getInsList_db__(const QString& dbName, const QString& man_tblName);

    // 存储对象的info数据至数据库（返回对象个数）
    static int storeToDB__(QList<PInstance> insList, const QString& dbName, const QString& tblName);

    QString dbName_, libTblName_;
    QSqlDatabase database_;
    QSqlQuery eccoQuery_;
    QList<QRegularExpression> regList_;
};

#endif // ECCO_H
