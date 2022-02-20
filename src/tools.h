#ifndef TOOLS_H
#define TOOLS_H

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QtConcurrent>

namespace Tools {

QString strippedName(const QString& fullFileName);

QString readTxtFile(const QString& fileName);

bool writeTxtFile(const QString& fileName, const QString& qstr, QIODevice::OpenMode flags);

// 针对目录调用操作函数(第一个参数是带有文件名的绝对文件路径)，可设置是否递归调用
void operateDir(const QString& dirName, std::function<void(const QString&, void*)> operateFile,
    void* arg, bool recursive = false);

QString downHtml(const QString& url, const char* codeName);

QString downHtmlsFromUrlsBlockingReduced(QList<QString> urls,
    QtConcurrent::ReduceOption reducOption = QtConcurrent::ReduceOption::OrderedReduce);

QList<QString> downHtmlsFromUrlsBlocking(QList<QString> urls);
};

#endif // TOOLS_H
