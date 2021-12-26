#ifndef ASPECT_H
#define ASPECT_H

#include <QMap>
#include <QTextStream>

enum class Color;

class Instance;

enum Evaluate {
    Count,
    Value
};

class Aspect {
public:
    Aspect(const QString& fen, Color color, int rowcols);

    QString fen;
    Color color;
    int rowcols;

    // Evaluate::Count 历史棋谱中某局面下该着法已发生的次数
    // Evaluate::Value 对应某局面的本着价值权重(通过局面评价函数计算)
    QList<int> evaluate { 1, 0 };
};

class Aspects {

public:
    /*
    Aspects() = default;
    ~Aspects() = default;*/

    void append(Instance& instance);

    QMap<int, QList<int>> getAspectRowCols(const QString& fen, Color color) const;
    Aspect getAspect(const QString& fen, Color color, int rowcols) const;

    void read(const QString& fileName);
    void write(const QString& fileName) const;

    QString toString() const;

private:
    QString getKey_(const QString& fen, Color color) const;
    QPair<QString, Color> getFenColor_(const QString& key) const;

    void append_(const Aspect& aspect);

    void read_(QTextStream& stream);
    void write_(QTextStream& stream) const;

    QMap<QString, QMap<int, QList<int>>> aspectMap_ {};
    static const QString FILETAG_;
};

#endif // ASPECT_H
