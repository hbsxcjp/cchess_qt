#ifndef MANUAL_H
#define MANUAL_H
// 中国象棋棋谱类型 by-cjp

#define DEBUG

#include <QTextStream>

class Piece;
enum class PieceColor;
enum class PieceKind;

class Seat;
using Coord = QPair<int, int>;
using CoordPair = QPair<Coord, Coord>;
using SeatPair = QPair<Seat*, Seat*>;

enum class SeatSide;
enum class ChangeType;

class Board;
class Move;
using InfoMap = QMap<QString, QString>;
enum class InfoIndex;
enum class StoreType;

class ManualMove;
class ManualMoveAppendIterator;

class Aspect;
using PAspect = Aspect*;

enum class ManualStatus {
    LAYOUT,
    PLAY,
    MOVEDEMO,
};

class Manual {
public:
    Manual();
    Manual(const QString& fileName);
    Manual(const InfoMap& infoMap);
    ~Manual();

    void reset();
    bool read(const QString& fileName);
    bool read(const InfoMap& infoMap);
    bool write(const QString& fileName);

    QList<Piece*> getAllPiece() const;
    QList<Seat*> getLiveSeats() const;

    // 初始化开局库专用
    Move* append_zhStr(const QString& zhStr);
    ManualMove* manualMove() const { return manualMove_; }
    ManualMoveAppendIterator appendIter();

    bool changeLayout(ChangeType ct);

    InfoMap& getInfoMap() { return info_; }
    const InfoMap& getInfoMap() const { return info_; }

    QString getInfoValue(InfoIndex nameIndex);
    void setInfoValue(InfoIndex nameIndex, const QString& value);

    ManualStatus status() const { return status_; }
    void setStatus(ManualStatus status) { status_ = status; }

    QString getECCORowcols() const;
    void setEcco(const QStringList& eccoRec);

    QList<Coord> canPut(Piece* piece) const;
    QList<Coord> canMove(const Coord& coord) const;

    // PGN_ZH、PGN_CC格式解析不是严格按深度搜索或广度搜索，因此设置数值不能嵌入每步添加着法，只能最后统一设置
    void setFEN(const QString& fen, PieceColor color);
    void setBoard();

    SeatSide getHomeSide(PieceColor color) const;
    QString getPieceChars() const;

    QString boardString(bool full = false) const;
    QString toMoveString(StoreType storeType) const;
    QString toString(StoreType storeType) const;
    QString toFullString();

    // 返回全部着法的记录指针列表; 记录为自分配内存，调用函数负责释放记录内存
    QList<Aspect> getAspectList();

private:
    Board* board_;
    ManualMove* manualMove_;

    InfoMap info_;
    ManualStatus status_;
};

#endif // MANUAL_H
