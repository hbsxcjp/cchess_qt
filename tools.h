#ifndef TOOLS_H
#define TOOLS_H

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextCodec>
#include <QTextStream>
#include <QtDebug>

namespace Tools {

QString readTxtFile(const QString& fileName);

bool writeTxtFile(const QString& fileName, const QString& qstr, QIODevice::OpenMode flags);

// 针对目录调用操作函数(第一个参数是带有文件名的绝对文件路径)，可设置是否递归调用
void operateDir(const QString& dirName, void operateFile(const QString&, void*), void* arg, bool recursive);

};

#endif // TOOLS_H
