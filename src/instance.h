#ifndef INSTANCE_H
#define INSTANCE_H
// 中国象棋棋谱类型 by-cjp

#define DEBUG

#include <QTextStream>

enum class PieceColor;

class Seat;
class Seats;
using PSeat = Seat*;
using SeatCoord = QPair<int, int>;
using SeatCoordPair = QPair<SeatCoord, SeatCoord>;
using MovSeat = QPair<PSeat, PSeat>;

enum class SeatSide;
enum class ChangeType;

class Board;
using PBoard = Board*;

class Move;
using PMove = Move*;
using InfoMap = QMap<QString, QString>;
enum class InfoIndex;
enum class StoreType;

class Aspect;
using PAspect = Aspect*;

enum class InsStatus {
    LAYOUT,
    PLAY,
    MOVEDEMO,
};

class Instance {
    friend class InstanceIO;

public:
    Instance();
    ~Instance();

    void reset();

    // 添加着法，如着法无效则返回空指针
    PMove appendMove(const MovSeat& movseat, const QString& remark, bool isOther);
    PMove appendMove(SeatCoordPair seatCoordlPair, const QString& remark, bool isOther);
    PMove appendMove(QList<QChar> iccs, const QString& remark, bool isOther);
    PMove appendMove(const QString& zhStr, const QString& remark, bool isOther);
    PMove appendMove_rowcols(const QString& rowcols, const QString& remark, bool isOther);
    // 初始化开局库专用
    PMove appendMove_ecco(const QString& zhStr);

    bool go(bool isOther);
    bool goNext(); // 前进
    bool goOther(); // 前进变着
    void goEnd(); // 前进至底
    void goTo(PMove move); // 前进至指定move

    bool back(bool isOther);
    bool backOne(); // 回退本着，或变着
    bool backNext(); // 本着非变着，则回退一着
    bool backOther(); // 回退变着
    bool backToPre(); // 回退至前着，如果当前为变着，则回退至首变着再回退
    void backStart(); // 回退至首着
    void backTo(PMove move); // 后退至指定move
    void goOrBackInc(int inc); // 前进或后退数步，返回实际着数

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

    PMove getRootMove() const { return rootMove_; }
    PMove getCurMove() const { return curMove_; }
    bool isStartMove() const { return rootMove_ == curMove_; }
    bool isEndMove() const;
    bool hasOtherMove() const;
    bool isOtherMove() const;

    InsStatus status() const { return status_; }
    void setStatus(InsStatus status) { status_ = status; }

    QString getECCORowcols() const;
    void setEcco(const QStringList& eccoRec);

    SeatCoordPair getCurSeatCoordPair() const;
    QList<SeatCoord> canPut(QChar ch) const;
    QList<SeatCoord> canMove(SeatCoord seatCoord) const;

    // PGN_ZH、PGN_CC格式解析不是严格按深度搜索或广度搜索，因此设置数值不能嵌入每步添加着法，只能最后统一设置
    void setMoveNums();

    void setFEN(const QString& fen, PieceColor color);
    void setBoard();

    SeatSide getHomeSide(PieceColor color) const;

    QString getPieceChars() const;
    QString moveInfo() const;

    QString toString(StoreType storeType) const;
    QString toFullString();

    // 返回全部着法的记录指针列表; 记录为自分配内存，调用函数负责释放记录内存
    QList<Aspect> getAspectList();

private:
    PBoard board_;
    PMove rootMove_;
    PMove curMove_;
    InfoMap info_;
    InsStatus status_;

    int movCount_ { 0 };
    int remCount_ { 0 };
    int remLenMax_ { 0 };
    int maxRow_ { 0 };
    int maxCol_ { 0 };
};

#endif // INSTANCE_H
