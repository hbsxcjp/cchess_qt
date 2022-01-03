#ifndef ECCO_H
#define ECCO_H

#include <QList>
#include <QMap>
#include <QRegularExpression>
#include <QString>

class Instance;
using PInstance = Instance*;

#define MOVESTR_LEN 4

using BoutStrs = QMap<QChar, QList<QString>>;

class Ecco {
public:
    Ecco(const QString& libFileName);
    Ecco(const QString& dbName, const QString& tblName);

    // 设置棋谱对象的开局名称
    bool setECCO(PInstance ins) const;
    bool setECCO(QList<PInstance> insList) const;

private:
    QList<QRegularExpression> regList_ {};
};

class InitEcco : public QObject {
    //    Q_OBJECT;

public:
    void initEccoLib();
    void handleInstanceHtmlStr(const QString& htmlStr);

private:
    // 初始化开局库的辅助函数
    void insertBoutStr_(BoutStrs& boutStrs, QChar boutNo, int color, QString mvstrs,
        QRegularExpression reg_m, QRegularExpression reg_bp);
    void setBoutStrs_(BoutStrs& boutStrs, const QString& sn, const QString& mvstrs, bool isBefore,
        QRegularExpression reg_m, QRegularExpression reg_bm, QRegularExpression reg_bp);
    QString getIccses_(const QString& mvstrs, Instance& ins);
    QString getRegStr_(const BoutStrs& boutStrs, Instance& ins);
    void setEccoRecordRegstrField_(QMap<QString, QStringList>& records);
    void setEccoRecord_(QMap<QString, QStringList>& eccoRecords, const QString& cleanHtmlStr);

    // 获取棋谱对象链表
    static QList<PInstance> getInsList_dir__(const QString& dirName);
    static QList<PInstance> getInsList_webfile__(const QString& insFileName);
    static QList<PInstance> getInsList_db__(const QString& dbName, const QString& man_tblName);

    // 存储对象的info数据至数据库（返回对象个数）
    static int storeToDB__(QList<PInstance> insList, const QString& dbName, const QString& tblName);

    QString dbName_ {}, tblName_ {};
};

#endif // ECCO_H
