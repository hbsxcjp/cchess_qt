#include "tools.h"
#include <QApplication>
#include <QEventLoop>
#include <QTextCodec>
#include <QTextStream>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QtDebug>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

QString Tools::strippedName(const QString& fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

QStringList Tools::readStringList(QSettings& settings, const QString& stringListKey, const QString& stringKey)
{
    QStringList result;
    const int count = settings.beginReadArray(stringListKey);
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        result.append(settings.value(stringKey).toString());
    }
    settings.endArray();
    return result;
}

void Tools::writeStringList(const QStringList& stringList, QSettings& settings,
    const QString& stringListKey, const QString& stringKey)
{
    const int count = stringList.size();
    settings.beginWriteArray(stringListKey);
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        settings.setValue(stringKey, stringList.at(i));
    }
    settings.endArray();
}

QString Tools::readTxtFile(const QString& fileName)
{
    QString qstr {};
    QFile file(fileName);
    if (!file.exists() || !(file.open(QIODevice::ReadOnly | QIODevice::Text))) {
        file.close();
        return qstr;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    //    stream.setCodec(QTextCodec::codecForLocale());
    //    stream.setAutoDetectUnicode(true);
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
    stream.setCodec("UTF-8");
    //    stream.setCodec(QTextCodec::codecForLocale());
    //    stream.setAutoDetectUnicode(true);
    stream << qstr;
    file.close();
    return true;
}

void Tools::operateDir(const QString& dirName, std::function<void(const QString&, void*)> operateFile,
    void* arg, bool recursive)
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

QString Tools::downHtml(const QString& url, const char* codeName)
{
    //    qDebug() << "downHtml()" << url << QThread::currentThread();
    QString result;
    QUrl qurl(url);
    if (!qurl.isValid())
        return result;

    QTextCodec* codec = QTextCodec::codecForName(codeName);
    QNetworkAccessManager* manager = new QNetworkAccessManager;
    QNetworkReply* reply = manager->get(QNetworkRequest(qurl));

    // 同步请求
    QEventLoop loop;
    QTimer timer;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(120 * 1000); // 设置下载延时时间
    loop.exec();

    result = codec->toUnicode(reply->readAll());
    reply->deleteLater();
    manager->deleteLater();

    //    qDebug() << "downHtml() end." << result;
    return result;
}

static QString downHtml_GB2312(const QString& url)
{
    return Tools::downHtml(url, "GB2312"); // GB18030
}

static void appendString(QString& result, const QString& str)
{
    result.append(str);
}

QString Tools::downHtmlsFromUrlsBlockingReduced(QList<QString> urls, QtConcurrent::ReduceOption reducOption)
{
    return QtConcurrent::blockingMappedReduced(urls, downHtml_GB2312, appendString, reducOption);
}

QList<QString> Tools::downHtmlsFromUrlsBlocking(QList<QString> urls)
{
    return QtConcurrent::blockingMapped(urls, downHtml_GB2312);
}
