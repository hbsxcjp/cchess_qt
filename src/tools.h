#ifndef TOOLS_H
#define TOOLS_H

#include <QtConcurrent>

namespace Tools {

QStringList readStringList(QSettings& settings, const QString& listKey, const QString& key);

void writeStringList(const QStringList& stringList, QSettings& settings,
    const QString& listKey, const QString& key);

QString readTxtFile(const QString& fileName);

bool writeTxtFile(const QString& fileName, const QString& string, QIODevice::OpenMode flags);

// 针对目录调用操作函数(第一个参数是带有文件名的绝对文件路径)，可设置是否递归调用
void operateDir(const QString& dirName, std::function<void(const QString&, void*)> operateFile,
    void* arg, bool recursive = false);

QString downHtmlsFromUrlsBlockingReduced(QList<QString> urls,
    QtConcurrent::ReduceOption reducOption = QtConcurrent::ReduceOption::OrderedReduce);

QList<QString> downHtmlsFromUrlsBlocking(QList<QString> urls);

int messageBox(const QString& title, const QString& text,
    const QString& name0, const QString& name1 = QString(), const QString& name2 = QString());

qreal getReviseScale();

};

#endif // TOOLS_H
