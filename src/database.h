#ifndef DATABASE_H
#define DATABASE_H

#include <QItemSelectionModel>
#include <QList>
#include <QMap>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QSqlRecord>
#include <QString>
#include <QTableView>
#include <QtConcurrent>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlTableModel>

#define MOVESTR_LEN 4

class Manual;
using BoutStrs = QMap<QChar, QStringList>;
using InfoMap = QMap<QString, QString>;

class DataBase : public QObject {
public:
    DataBase();
    ~DataBase();

    // 设置棋谱对象的开局名称
    bool setECCO(Manual* manual);
    bool setECCO(QList<Manual*> manuals);

    // 初始化布局库
    void initEccoLib();

    // 初始化棋谱库
    void downAllXqbaseManual();
    void downSomeXqbaseManual();
    void setRowcolsXqbaseManual(bool setNull);
    void checkEccosnXqbaseManual(bool checkDiff);

    // 设置表视图和条目选择模型
    void initInsTableModelView(QTableView* tableView, QItemSelectionModel*& insItemSelModel);
    void updateInsTableModel(const QDate& startDate, const QDate& endDate,
        const QString& title, const QString& event, const QString& site,
        const QString& eccoSn, const QString& eccoName, const QString& result,
        const QString& person, int colorIndex);
    // 获取棋谱表模型
    QSqlTableModel* getInstanceTableModel() { return manualTableModel; };

    // 读取或存入棋谱
    void insertInfoMap(const InfoMap& infoMap) const;
    static InfoMap getInfoMap(const QSqlRecord& record);

    QString getTitleName(QItemSelectionModel*& insItemSelModel) const;
    static QString getTitleName(const InfoMap& infoMap);
    InfoMap getInfoMap(const QString& titleName) const;

private:
    // 初始化开局库的辅助函数
    static QString getRowcols_(const QString& zhStr, Manual& manual, bool isGo);
    static QStringList getRowcolsList_(const QString& mvstr, bool order, Manual& manual);
    static QString getColorRowcols_(const QString& mvstrs, const QString& anyMoveRegStr, Manual& manual);
    static QString getRegStr_(const BoutStrs& boutStrs, const QString& sn, Manual& manual);
    static void setEccoRecordRegstrField_(QMap<QString, QStringList>& eccoRecords);
    static void setBoutStrs_(BoutStrs& boutStrs, const QString& sn, const QString& mvstrs, bool isPreMvStrs);
    static void setEccoRecord_(QMap<QString, QStringList>& eccoRecords, const QString& cleanHtmlStr);
    void writeEccoLib_(QMap<QString, QStringList>& eccoRecords);

    // 初始化棋谱库的辅助函数
    QList<InfoMap> downXqbaseManual_(const QList<int>& idList);
    void setRowcols_(QList<InfoMap>& infoMapList);
    static QString getFieldNames_(const QStringList& names, const QString& suffix);
    void insertManual_(QList<InfoMap>& infoMapList, bool initTable);
    void updateManual_(QList<InfoMap>& infoMapList);

    // 原始和旋转，再加两次左右对称形成四种着法情形进行匹配
    QStringList getECCO(const QString& eccoRowcols);
    QStringList getECCO(Manual* manual);

    // 获取棋谱对象链表
    static QList<Manual*> getManuals_dir__(const QString& dirName);
    static QList<Manual*> getManuals_webfile__(const QString& insFileName);
    static QList<Manual*> getManuals_db__(const QString& dbName, const QString& man_tblName);

    // 存储对象的info数据至数据库（返回对象个数）
    static int storeToDB__(QList<Manual*> manuals, const QString& dbName, const QString& tblName);

    QString dbName_;
    QString libTblName_;
    QString manTblName_;
    QSqlDatabase database_;
    QSqlTableModel* manualTableModel;
};

#endif // ECCO_H
