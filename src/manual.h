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
    ~Manual();

    void reset();

    QList<Piece*> getAllPiece() const;
    QList<Seat*> getLiveSeats() const;

    // 添加着法，如着法无效则返回空指针
    Move* goAppendMove(const CoordPair& coordlPair, const QString& remark, bool isOther);
    Move* goAppendMove(const QString& iccsOrZhStr, const QString& remark, bool isOther, bool isPGN_ZH);
    Move* goAppendMove(const QString& rowcols, const QString& remark, bool isOther);
    // 初始化开局库专用
    Move* goAppendMove(const QString& zhStr);

    bool changeLayout(ChangeType ct);

    InfoMap& getInfoMap() { return info_; }
    const InfoMap& getInfoMap() const { return info_; }

    QString getInfoValue(InfoIndex nameIndex);
    void setInfoValue(InfoIndex nameIndex, const QString& value);

    int getMovCount() const { return movCount_; }
    int getRemCount() const { return remCount_; }
    int getRemLenMax() const { return remLenMax_; }
    int maxRow() const { return maxRow_; }
    int maxCol() const { return maxCol_; }

    Move* getRootMove() const;
    Move* getCurMove() const;
    bool curMoveIs(Move* move) const;
    const QString& getCurRemark() const;
    void setCurRemark(const QString& remark) const;
    ManualMove*& manualMove() { return manualMove_; }

    ManualStatus status() const { return status_; }
    void setStatus(ManualStatus status) { status_ = status; }

    QString getECCORowcols() const;
    void setEcco(const QStringList& eccoRec);

    SeatPair getCurSeatPair() const;
    CoordPair getCurCoordPair() const;
    QList<Coord> canPut(Piece* piece) const;
    QList<Coord> canMove(const Coord& coord) const;

    // PGN_ZH、PGN_CC格式解析不是严格按深度搜索或广度搜索，因此设置数值不能嵌入每步添加着法，只能最后统一设置
    void setMoveNums();
    void setFEN(const QString& fen, PieceColor color);
    void setBoard();

    SeatSide getHomeSide(PieceColor color) const;
    QString getPieceChars() const;
    QString moveInfo() const;

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

    int movCount_ { 0 };
    int remCount_ { 0 };
    int remLenMax_ { 0 };
    int maxRow_ { 0 };
    int maxCol_ { 0 };
};

#endif // MANUAL_H
