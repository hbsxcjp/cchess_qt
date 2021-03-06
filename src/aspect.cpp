#include "aspect.h"
#include "manual.h"
#include <QDataStream>
#include <QFile>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

const QString Aspects::FILETAG_ { "learnchess_aspects" };

Aspect::Aspect(const QString& fen, PieceColor color, const QString& rowcols)
    : fen { fen }
    , color { color }
    , rowcols { rowcols }
{
}

Aspects::Aspects(Manual& manual)
{
    append(manual);
}

Aspects::Aspects(const QString& fileName)
{
    read(fileName);
}

void Aspects::append(Manual& manual)
{
    for (auto& aspect : manual.getAspectList())
        append_(aspect);
}

QMap<QString, QList<int>>
Aspects::getAspectRowCols(const QString& fen, PieceColor color) const
{
    return aspectMap_.value(getKey_(fen, color));
}

Aspect
Aspects::getAspect(const QString& fen,
    PieceColor color,
    const QString& rowcols) const
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

QString
Aspects::toString() const
{
    QString result;
    QTextStream stream(&result);
    write_(stream);

    return result;
}

QString
Aspects::getKey_(const QString& fen, PieceColor color) const
{
    return QString("%1_%2").arg(fen).arg(int(color));
}

QPair<QString, PieceColor>
Aspects::getFenColor_(const QString& key) const
{
    int index = key.indexOf('_');
    return { key.left(index), PieceColor(key.mid(index + 1, 1).toInt()) };
}

void Aspects::append_(const Aspect& aspect)
{
    QString key = getKey_(aspect.fen, aspect.color);
    QString rowcols = aspect.rowcols;
    if (aspectMap_.contains(key)) {
        QMap<QString, QList<int>>& rowColsMap = aspectMap_[key];
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
    static QRegularExpression lineReg(R"(([\S]*) \{(.+?)\}\n)"),
        evalReg(R"((\d{4})\[(.+?)\] )");
    QRegularExpressionMatch lineMatch, evalMatch;
    QRegularExpressionMatchIterator lineMatchIter = lineReg.globalMatch(aspStr);
    while (lineMatchIter.hasNext()) {
        QMap<QString, QList<int>> rowcolsMap {};

        lineMatch = lineMatchIter.next();
        QRegularExpressionMatchIterator evalMatchIter = evalReg.globalMatch(lineMatch.captured(2));
        while (evalMatchIter.hasNext()) {
            QList<int> evalList {};

            evalMatch = evalMatchIter.next();
            for (auto& eval : evalMatch.captured(2).split(' ', Qt::SkipEmptyParts))
                evalList.append(eval.toUInt());

            rowcolsMap[evalMatch.captured(1)] = evalList;
        }

        aspectMap_[lineMatch.captured(1)] = rowcolsMap;
    }
}

void Aspects::write_(QTextStream& stream) const
{
    for (auto aspIter = aspectMap_.keyValueBegin();
         aspIter != aspectMap_.keyValueEnd();
         ++aspIter) {
        auto& rowcolsMap = (*aspIter).second;
        stream << (*aspIter).first << " {"; // fen
        for (auto rowcolsIter = rowcolsMap.keyValueBegin();
             rowcolsIter != rowcolsMap.keyValueEnd();
             ++rowcolsIter) {
            auto& evalList = (*rowcolsIter).second;
            stream << (*rowcolsIter).first << "["; // rowcols
            for (auto eval : evalList)
                stream << eval << ' ';
            stream << "] ";
        }
        stream << "}\n";
    }
}
