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

QStringList Tools::readStringList(QSettings& settings, const QString& listKey, const QString& key)
{
    QStringList result;
    const int count = settings.beginReadArray(listKey);
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        result.append(settings.value(key).toString());
    }
    settings.endArray();

    return result;
}

void Tools::writeStringList(const QStringList& stringList, QSettings& settings,
    const QString& listKey, const QString& key)
{
    const int count = stringList.size();
    settings.beginWriteArray(listKey);
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        settings.setValue(key, stringList.at(i));
    }
    settings.endArray();
}

QString Tools::readTxtFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.exists() || !(file.open(QIODevice::ReadOnly | QIODevice::Text))) {
        file.close();
        return QString();
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    //    stream.setCodec(QTextCodec::codecForLocale());
    //    stream.setAutoDetectUnicode(true);
    QString result = stream.readAll();
    file.close();

    return result;
}

bool Tools::writeTxtFile(const QString& fileName, const QString& string, QIODevice::OpenMode flags)
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
    stream << string;
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

static QString downHtmlFromUrl(const QString& url, const char* codeName)
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

static QString downHtmlFromUrl_GB2312(const QString& url)
{
    return downHtmlFromUrl(url, "GB2312"); // GB18030
}

static void appendString(QString& result, const QString& string)
{
    result.append(string);
}

QString Tools::downHtmlsFromUrlsBlockingReduced(QList<QString> urls, QtConcurrent::ReduceOption reducOption)
{
    return QtConcurrent::blockingMappedReduced(urls, downHtmlFromUrl_GB2312, appendString, reducOption);
}

QList<QString> Tools::downHtmlsFromUrlsBlocking(QList<QString> urls)
{
    return QtConcurrent::blockingMapped(urls, downHtmlFromUrl_GB2312);
}
