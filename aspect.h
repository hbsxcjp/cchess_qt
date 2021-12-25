#ifndef ASPECT_H
#define ASPECT_H

#include <QMap>
#include <QTextStream>

enum class Color;

class Instance;

class AspectStatus {
public:
    AspectStatus() = default;
    ~AspectStatus() = default;

    AspectStatus(int weight0, bool isKilling0, bool willKill0, bool isCatch0, bool isFailed0);

    int count; // 历史棋谱中某局面下该着法已发生的次数
    int weight; // 对应某局面的本着价值权重(通过局面评价函数计算)

    bool isKilling; // 将
    bool willKill; // 杀
    bool isCatch; // 捉
    bool isFailed; // 失败
};
using PAspectStatus = AspectStatus*;

class Aspect {
public:
    Aspect(const QString& fen, Color color, int rowcols, AspectStatus status);

    QString fen;
    Color color;
    int rowcols;

    AspectStatus status;
};
using PAspect = Aspect*;

class Aspects {
public:
    ~Aspects();

    void appendAspectList(Instance& instance);

    QMap<int, AspectStatus> getRowColsMap(const QString& fen, Color color);

private:
    QString getKey_(const QString& fen, Color color);
    QPair<QString, Color> getFENColor_(const QString& key);

    QMap<QString, QMap<int, AspectStatus>> aspectMap_ {};
};

QTextStream& operator<<(QTextStream& out, const Aspect& aspect);
QTextStream& operator>>(QTextStream& in, Aspect& aspect);

#endif // ASPECT_H
