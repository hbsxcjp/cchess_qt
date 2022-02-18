#ifndef ECCO_H
#define ECCO_H

#include <QList>
#include <QMap>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QSqlRecord>
#include <QString>
#include <QtConcurrent>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlTableModel>

class Instance;
using PInstance = Instance*;

#define MOVESTR_LEN 4

using BoutStrs = QMap<QChar, QStringList>;

using InfoMap = QMap<QString, QString>;

class Ecco : public QObject {

public:
    Ecco();
    ~Ecco();

    // 设置棋谱对象的开局名称
    bool setECCO(PInstance ins);
    bool setECCO(QList<PInstance> insList);

    void initEccoLib();
    void downAllXqbaseManual();
    void downSomeXqbaseManual();
    void setRowcolsXqbaseManual(bool setNull);
    void checkEccosnXqbaseManual(bool checkDiff);

private:
    // 初始化开局库的辅助函数
    static QString getRowcols_(const QString& zhStr, Instance& ins, bool isGo);
    static QStringList getRowcolsList_(const QString& mvstr, bool order, Instance& ins);
    static QString getColorRowcols_(const QString& mvstrs, const QString& anyMoveRegStr, Instance& ins);
    static QString getRegStr_(const BoutStrs& boutStrs, const QString& sn, Instance& ins);
    static void setEccoRecordRegstrField_(QMap<QString, QStringList>& eccoRecords);
    static void setBoutStrs_(BoutStrs& boutStrs, const QString& sn, const QString& mvstrs, bool isPreMvStrs);
    static void setEccoRecord_(QMap<QString, QStringList>& eccoRecords, const QString& cleanHtmlStr);

    void writeEccoLib_(QMap<QString, QStringList>& eccoRecords);

    QList<InfoMap> downXqbaseManual_(const QList<int>& idList);
    void setRowcols_(QList<InfoMap>& infoMapList);
    static QString getFieldNames_(const QStringList& names, const QString& suffix);
    void insertManual_(QList<InfoMap>& infoMapList, bool initTable);
    void updateManual_(QList<InfoMap>& infoMapList);

    // 原始和旋转，再加两次左右对称形成四种着法情形进行匹配
    QStringList getECCO(const QString& eccoRowcols);
    QStringList getECCO(PInstance ins);

    // 获取棋谱对象链表
    static QList<PInstance> getInsList_dir__(const QString& dirName);
    static QList<PInstance> getInsList_webfile__(const QString& insFileName);
    static QList<PInstance> getInsList_db__(const QString& dbName, const QString& man_tblName);

    // 存储对象的info数据至数据库（返回对象个数）
    static int storeToDB__(QList<PInstance> insList, const QString& dbName, const QString& tblName);

    QString dbName_, libTblName_, manTblName_;
    QSqlDatabase database_;
};

#endif // ECCO_H