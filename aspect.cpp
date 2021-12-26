#include "aspect.h"
#include "instance.h"
#include <QDataStream>
#include <QFile>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

const QString Aspects::FILETAG_ { "learnchess_aspects" };

Aspect::Aspect(const QString& fen, Color color, int rowcols)
    : fen { fen }
    , color { color }
    , rowcols { rowcols }
{
}

Aspects::Aspects(Instance& instance)
{
    append(instance);
}

Aspects::Aspects(const QString& fileName)
{
    read(fileName);
}

void Aspects::append(Instance& instance)
{
    for (auto& aspect : instance.getAspectList())
        append_(aspect);
}

QMap<int, QList<int>> Aspects::getAspectRowCols(const QString& fen, Color color) const
{
    return aspectMap_.value(getKey_(fen, color));
}

Aspect Aspects::getAspect(const QString& fen, Color color, int rowcols) const
{
    Aspect aspect = Aspect(fen, color, rowcols);
    aspect.evaluate = getAspectRowCols(fen, color)[rowcols];
    return aspect;
}

void Aspects::read(const QString& fileName)
{
    QFile file(fileName);
    if (!file.exists() || !(file.open(QIODevice::ReadOnly))) {
        file.close();
        return;
    }

    QTextStream stream(&file);
    QString fileTag;
    stream >> fileTag;
    if (fileTag == FILETAG_)
        read_(stream);

    file.close();
}

void Aspects::write(const QString& fileName) const
{
    QFile file(fileName);
    if (!(file.open(QIODevice::WriteOnly))) {
        file.close();
        return;
    }

    QTextStream stream(&file);
    stream << FILETAG_ << '\n';
    write_(stream);
    file.close();
}

QString Aspects::toString() const
{
    QString result;
    QTextStream stream(&result);
    write_(stream);

    return result;
}

QString Aspects::getKey_(const QString& fen, Color color) const
{
    return QString("%1_%2").arg(fen).arg(int(color));
}

QPair<QString, Color> Aspects::getFenColor_(const QString& key) const
{
    int index = key.indexOf('_');
    return { key.left(index), Color(key.mid(index + 1, 1).toInt()) };
}

void Aspects::append_(const Aspect& aspect)
{
    QString key = getKey_(aspect.fen, aspect.color);
    int rowcols = aspect.rowcols;
    if (aspectMap_.contains(key)) {
        QMap<int, QList<int>>& rowColsMap = aspectMap_[key];
        if (rowColsMap.contains(rowcols)) {
            rowColsMap[rowcols][Evaluate::Count]++;
        } else
            rowColsMap[rowcols] = aspect.evaluate;
    } else
        aspectMap_[key] = { { rowcols, aspect.evaluate } };
}

void Aspects::read_(QTextStream& stream)
{
    QString aspStr { stream.readAll() };
    QRegularExpression lineReg(R"(([\S]*) \{(.+?)\}\n)"), evalReg(R"((\d{4})\[(.+?)\] )");
    QRegularExpressionMatch lineMatch, evalMatch;
    QRegularExpressionMatchIterator lineMatchIter = lineReg.globalMatch(aspStr);
    while (lineMatchIter.hasNext()) {
        QMap<int, QList<int>> rowcolsMap {};

        lineMatch = lineMatchIter.next();
        QRegularExpressionMatchIterator evalMatchIter = evalReg.globalMatch(lineMatch.captured(2));
        while (evalMatchIter.hasNext()) {
            QList<int> evalList {};

            evalMatch = evalMatchIter.next();
            for (auto& eval : evalMatch.captured(2).split(' '))
                if (!eval.isEmpty())
                    evalList.append(eval.toUInt());

            rowcolsMap[evalMatch.captured(1).toUInt()] = evalList;
        }

        aspectMap_[lineMatch.captured(1)] = rowcolsMap;
    }
}

void Aspects::write_(QTextStream& stream) const
{
    for (auto aspIter = aspectMap_.keyValueBegin(); aspIter != aspectMap_.keyValueEnd(); ++aspIter) {
        auto& rowcolsMap = (*aspIter).second;
        stream << (*aspIter).first << " {"; // fen
        for (auto rowcolsIter = rowcolsMap.keyValueBegin();
             rowcolsIter != rowcolsMap.keyValueEnd(); ++rowcolsIter) {
            auto& evalList = (*rowcolsIter).second;
            stream << qSetFieldWidth(4) << qSetPadChar('0') << (*rowcolsIter).first
                   << qSetFieldWidth(0) << qSetPadChar(' ') << "["; // rowcols
            for (auto eval : evalList)
                stream << eval << ' ';
            stream << "] ";
        }
        stream << "}\n";
    }
}
