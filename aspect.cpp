#include "aspect.h"
#include "instance.h"
#include <QDataStream>

AspectStatus::AspectStatus(int weight0, bool isKilling0, bool willKill0, bool isCatch0, bool isFailed0)
    : count(1)
    , weight(weight0)
    , isKilling(isKilling0)
    , willKill(willKill0)
    , isCatch(isCatch0)
    , isFailed(isFailed0)
{
}

Aspect::Aspect(const QString& fen, Color color, int rowcols, AspectStatus status)
    : fen { fen }
    , color { color }
    , rowcols { rowcols }
    , status { status }
{
}

void Aspects::appendAspectList(Instance& instance)
{
    for (auto& aspect : instance.getAspectList()) {
        QString key = getKey_(aspect->fen, aspect->color);
        if (aspectMap_.contains(key)) {
            QMap<int, AspectStatus>& rowColsMap = aspectMap_[key];
            int rowcols = aspect->rowcols;
            if (rowColsMap.contains(rowcols)) {
                rowColsMap[rowcols].count++;
            } else
                rowColsMap[rowcols] = aspect->status;
        } else
            aspectMap_[key] = { { aspect->rowcols, aspect->status } };

        delete aspect;
    }
}

QMap<int, AspectStatus> Aspects::getRowColsMap(const QString& fen, Color color)
{
    return aspectMap_.value(getKey_(fen, color));
}

QString Aspects::getKey_(const QString& fen, Color color)
{
    return QString("%1_%2").arg(fen).arg(int(color));
}

QPair<QString, Color> Aspects::getFENColor_(const QString& key)
{
    int index = key.indexOf('_');
    return { key.left(index), Color(key.mid(index + 1, 1).toInt()) };
}

QTextStream& operator<<(QTextStream& out, const Aspect& aspect)
{
    out << aspect.fen << int(aspect.color) << aspect.rowcols
        << aspect.status.count << aspect.status.weight
        << int(aspect.status.isKilling) << int(aspect.status.willKill)
        << int(aspect.status.isCatch) << int(aspect.status.isFailed);

    return out;
}

QTextStream& operator>>(QTextStream& in, Aspect& aspect)
{
    int color = 0, isKilling = 0, willKill = 0, isCatch = 0, isFailed = 0;
    in >> aspect.fen >> color >> aspect.rowcols
        >> aspect.status.count >> aspect.status.weight
        >> isKilling >> willKill
        >> isCatch >> isFailed;
    // aspect.status.

    return in;
}
