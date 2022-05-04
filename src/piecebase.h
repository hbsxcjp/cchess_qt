#ifndef PIECEBASE_H
#define PIECEBASE_H

#include <QList>

enum class PieceColor;
enum class PieceKind;

// 棋子基础类
class PieceBase {
public:
    static QString getZhChars();
    static QString getIccsChars();

    static bool isLinePiece(QChar name);
    static bool isPawnPiece(QChar name);
    static bool isStrongePiece(QChar name);
    static bool isAdvisorBishopPiece(QChar name);
    static bool isPiece(QChar name);

    static int getKindNum(PieceKind kind);
    static int getRowFrom(QChar ch);
    static int getColFrom(QChar ch);

    static QChar getCh(PieceColor color, PieceKind kind);
    static QChar getName(PieceColor color, PieceKind kind);
    static QChar getPrintName(PieceColor color, PieceKind kind);
    static QChar getColorName(PieceColor color);

    static PieceKind getKind(QChar ch);
    static PieceKind getKind(PieceColor color, QChar name);

    static PieceColor getColor(QChar ch);
    static PieceColor getOtherColor(PieceColor color);
    static PieceColor getColorFromZh(QChar numZh);

    static int getIndex(int seatsLen, bool isBottom, QChar preChar);
    static QChar getIndexChar(int seatsLen, bool isBottom, int index);

    static int getMovNum(bool isBottom, QChar movChar);
    static QChar getMovChar(bool isSameRow, bool isBottom, bool isLowToUp);

    static int getNum(PieceColor color, QChar numChar);
    static QChar getNumChar(PieceColor color, int num);

    static QChar getIccsChar(int col);
    static int getCol(bool isBottom, int num);
    static QChar getColChar(PieceColor color, bool isBottom, int col);

    static const QList<PieceColor> ALLCOLORS;
    static const QList<PieceKind> ALLKINDS;

    static const QString FENSTR;
    static const QChar NULLCHAR;
};

#endif // PIECEBASE_H
