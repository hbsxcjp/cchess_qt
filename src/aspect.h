#ifndef ASPECT_H
#define ASPECT_H

#include <QMap>
#include <QTextStream>

enum class PieceColor;

class Manual;

enum Evaluate {
    Count,
    Value
};

// 一种基本情形. 某局面配对开局正则表达式时，通过交换颜色或旋转转换成红底、
// 左右对称转换两种情形进行匹配（匹配结果增加是否交换或是否旋转、是否左右对称交换两个信息）
class Aspect {
public:
    Aspect(const QString& fen, PieceColor color, const QString& rowcols);

    QString fen;
    PieceColor color;
    QString rowcols;

    // Evaluate::Count 历史棋谱中某局面下该着法已发生的次数
    // Evaluate::Value 对应某局面的本着价值权重(通过局面评价函数计算)
    QList<int> evaluate { 1, 0 };
};

class Aspects {

public:
    Aspects() = default;
    Aspects(Manual& manual);
    Aspects(const QString& fileName);

    void append(Manual& manual);

    QMap<QString, QList<int>> getAspectRowCols(const QString& fen, PieceColor color) const;
    Aspect getAspect(const QString& fen, PieceColor color, const QString& rowcols) const;

    void read(const QString& fileName);
    void write(const QString& fileName) const;

    QString toString() const;

private:
    QString getKey_(const QString& fen, PieceColor color) const;
    QPair<QString, PieceColor> getFenColor_(const QString& key) const;

    void append_(const Aspect& aspect);

    void read_(QTextStream& stream);
    void write_(QTextStream& stream) const;

    QMap<QString, QMap<QString, QList<int>>> aspectMap_ {};
    static const QString FILETAG_;
};

#endif // ASPECT_H
