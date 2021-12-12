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

Aspect::Aspect(const QString& fen, Color color, int rowcols, PAspectStatus status)
    : fen { fen }
    , color { color }
    , rowcols { rowcols }
    , status { status }
{
}

Aspects::~Aspects()
{
    for (auto& rowColList : aspectMap_.values())
        for (auto& status : rowColList.values())
            delete status;
}

void Aspects::appendAspectList(Instance& instance)
{
    for (auto& aspect : instance.getAspectList()) {
        QString key = getKey_(aspect->fen, aspect->color);
        if (aspectMap_.contains(key)) {
            QMap<int, PAspectStatus>& rowColsList = aspectMap_[key];
            int rowcols = aspect->rowcols;
            if (rowColsList.contains(rowcols)) {
                PAspectStatus status = rowColsList[rowcols];
                status->count++;

                delete aspect->status;
            } else
                rowColsList[rowcols] = aspect->status;
        } else
            aspectMap_[key] = { { aspect->rowcols, aspect->status } };

        delete aspect; // 析构时不释放status指针所指内存，而由Aspects去释放
    }
}

QMap<int, PAspectStatus> Aspects::getRowColsList(const QString& fen, Color color)
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

QDataStream& operator<<(QDataStream& out, const Aspect& aspect)
{
    out << aspect.fen << aspect.color << aspect.rowcols
        << aspect.status->count << aspect.status->weight
        << aspect.status->isKilling << aspect.status->willKill
        << aspect.status->isCatch << aspect.status->isFailed;

    return out;
}

QDataStream& operator>>(QDataStream& in, Aspect& aspect)
{
    in >> aspect.fen >> aspect.color >> aspect.rowcols
        >> aspect.status->count >> aspect.status->weight
        >> aspect.status->isKilling >> aspect.status->willKill
        >> aspect.status->isCatch >> aspect.status->isFailed;

    return in;
}
