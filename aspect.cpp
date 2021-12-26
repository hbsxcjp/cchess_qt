#include "aspect.h"
#include "instance.h"
#include <QDataStream>
#include <QFile>

const QString Aspects::FILETAG_ { "learnchess_aspects\n" };

Aspect::Aspect(const QString& fen, Color color, int rowcols)
    : fen { fen }
    , color { color }
    , rowcols { rowcols }
{
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
    stream << FILETAG_;
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
    QString fen_color;
    QChar token;
    int rowcolsCount = 0, rowcols = 0, evalCount = 0, eval = 0;
    while (!stream.atEnd()) {
        stream >> fen_color >> rowcolsCount >> token;

        QMap<int, QList<int>> rowcolsMap {};
        for (int i = 0; i < rowcolsCount; ++i) {
            stream >> rowcols >> evalCount >> token;

            QList<int> evalList {};
            for (int j = 0; j < evalCount; ++j) {
                stream >> eval;
                evalList.append(eval);
            }
            rowcolsMap[rowcols] = evalList;

            stream >> token;
        }

        aspectMap_[fen_color] = rowcolsMap;
        stream >> token;
    }
}

void Aspects::write_(QTextStream& stream) const
{
    for (auto aspIter = aspectMap_.keyValueBegin(); aspIter != aspectMap_.keyValueEnd(); ++aspIter) {
        auto& rowcolsMap = (*aspIter).second;
        stream << (*aspIter).first << ' ' << rowcolsMap.count() << " {"; // fen
        for (auto rowcolsIter = rowcolsMap.keyValueBegin();
             rowcolsIter != rowcolsMap.keyValueEnd(); ++rowcolsIter) {
            auto& evalList = (*rowcolsIter).second;
            stream << qSetFieldWidth(4) << qSetPadChar('0') << (*rowcolsIter).first
                   << qSetPadChar(' ') << qSetFieldWidth(0)
                   << ' ' << evalList.count() << " ["; // rowcols
            for (auto eval : evalList)
                stream << eval << ' ';
            stream << "]";
        }
        stream << "}\n";
    }
}
