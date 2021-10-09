#include "tools.h"
#include <functional>

QString Tools::readTxtFile(const QString& fileName)
{
    QString qstr {};
    QFile file(fileName);
    if (!file.exists() || !(file.open(QIODevice::ReadOnly | QIODevice::Text))) {
        file.close();
        return qstr;
    }

    QTextStream stream(&file);
    stream.setAutoDetectUnicode(true);
    qstr = stream.readAll();
    file.close();
    return qstr;
}

bool Tools::writeTxtFile(const QString& fileName, const QString& qstr, QIODevice::OpenMode flags)
{
    QFile file(fileName);
    if (!file.open(flags | QIODevice::Text)) {
        file.close();
        return false;
    }

    QTextStream stream(&file);
    stream.setAutoDetectUnicode(true);
    stream << qstr;
    file.close();
    return true;
}

void Tools::operateDir(const QString& dirName, void operateFile(const QString&, void*), void* arg, bool recursive)
{
    QDir dir(dirName);
    QStringList fileList = dir.entryList(QDir::Files);
    for (auto& fileName : fileList)
        operateFile(dir.absoluteFilePath(fileName), arg);

    if (!recursive) // 非递归
        return;

    QStringList dirList = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (auto& name : dirList)
        operateDir(dir.absoluteFilePath(name), operateFile, arg, recursive);
}
