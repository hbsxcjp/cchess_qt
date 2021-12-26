#ifndef INSTANCE_H
#define INSTANCE_H
// 中国象棋棋谱类型 by-cjp

#define DEBUG

#include <QTextStream>

class Piece;
using PPiece = Piece*;
class Pieces;
enum class Color;

class Seat;
class Seats;
using PSeat = Seat*;
using SeatCoord = QPair<int, int>;
using SeatCoordPair = QPair<SeatCoord, SeatCoord>;
using MovSeat = QPair<PSeat, PSeat>;

enum class Side;
enum class ChangeType;

class Board;
using PBoard = Board*;

class Move;
using PMove = Move*;
using InfoMap = QMap<QString, QString>;

class Aspect;
using PAspect = Aspect*;

enum class PGN {
    ICCS,
    ZH,
    CC
};

class Instance {
    friend class InstanceIO;

public:
    ~Instance();

    // 添加着法，如着法无效则返回空指针
    PMove appendMove(const MovSeat& movseat, const QString& remark, bool isOther);
    PMove appendMove(int rowcols, const QString& remark, bool isOther);
    PMove appendMove(SeatCoordPair seatCoordlPair, const QString& remark, bool isOther);
    PMove appendMove(QList<QChar> iccs, const QString& remark, bool isOther);
    PMove appendMove(QString zhStr, const QString& remark, bool isOther);

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
    void goInc(int inc); // 前进或后退数步，返回实际着数

    void changeLayout(ChangeType ct);

    InfoMap& getInfoMap() { return info_; }
    const InfoMap& getInfoMap_const() const { return info_; }

    int getMovCount() const { return movCount_; }
    int getRemCount() const { return remCount_; }
    int getRemLenMax() const { return remLenMax_; }
    int getMaxRow() const { return maxRow_; }
    int getMaxCol() const { return maxCol_; }

    QString getZhChars() const;

    PMove getRootMove() const { return rootMove_; }
    PMove getCurMove() const { return curMove_; }
    SeatCoordPair getCurSeatCoordPair() const;

    // PGN_ZH、PGN_CC格式解析不是严格按深度搜索或广度搜索，因此设置数值不能嵌入每步添加着法，只能最后统一设置
    void setMoveNums();

    void setFEN(const QString& fen, Color color);
    void setBoard();

    const QString moveInfo() const;

    const QString toString();
    const QString toFullString();

    // 返回全部着法的记录指针列表; 记录为自分配内存，调用函数负责释放记录内存
    QList<Aspect> getAspectList();

private:
    Instance();

    const QString fen__() const;

    PBoard board_;
    PMove rootMove_, curMove_;
    InfoMap info_;
    int movCount_ { 0 }, remCount_ { 0 }, remLenMax_ { 0 }, maxRow_ { 0 }, maxCol_ { 0 };
};

#endif // INSTANCE_H
