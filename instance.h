#ifndef INSTANCE_H
#define INSTANCE_H
// 中国象棋棋盘布局类型 by-cjp

//#define DEBUG

#include <QMap>
#include <QTextStream>

class Piece;
using PPiece = Piece*;
class Pieces;
enum class Color;

class Seat;
class Seats;
using PSeat = Seat*;
using SeatCoord = QPair<int, int>;
using MovSeat = QPair<PSeat, PSeat>;

enum class Side;
enum class ChangeType;

class Board;
using PBoard = Board*;

class Move;
using PMove = Move*;
using InfoMap = QMap<QString, QString>;

enum class SaveFormat {
    XQF,
    BIN,
    JSON,
    PGN_ICCS,
    PGN_ZH,
    PGN_CC,
    NOTFMT
};

class Aspect;
using PAspect = Aspect*;

class Instance {
public:
    Instance();
    Instance(const QString& fileName);
    ~Instance();

    void write(const QString& fileName) const;

    // 添加着法，如着法无效则返回空指针
    PMove appendMove(const MovSeat& movseat, const QString& remark, bool isOther);
    PMove appendMove(int rowcols, const QString& remark, bool isOther);
    PMove appendMove(QString iccszhStr, SaveFormat fmt, const QString& remark, bool isOther);

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

    int getMovCount() const { return movCount_; }
    int getRemCount() const { return remCount_; }
    int getRemLenMax() const { return remLenMax_; }
    int getMaxRow() const { return maxRow_; }
    int getMaxCol() const { return maxCol_; }
    const QString& remark() const;

    static const QString getExtName(const SaveFormat fmt);
    static SaveFormat getSaveFormat(const QString& ext_);

    const QString toString();
    const QString toFullString();

    // 返回全部着法的记录指针列表; 记录为自分配内存，调用函数负责释放记录内存
    QList<PAspect> getAspectList();

private:
    void readXQF__(const QString& fileName);
    void readBIN__(const QString& fileName);
    void writeBIN__(const QString& fileName) const;
    void readJSON__(const QString& fileName);
    void writeJSON__(const QString& fileName) const;
    void readPGN__(const QString& fileName, SaveFormat fmt);
    void writePGN__(const QString& fileName, SaveFormat fmt) const;

    void readInfo_PGN__(QTextStream& stream);
    void writeInfo_PGN__(QTextStream& stream) const;
    void readMove_PGN_ICCSZH__(QTextStream& stream, SaveFormat fmt);
    void writeMove_PGN_ICCSZH__(QTextStream& stream, SaveFormat fmt) const;
    QString remarkNo__(int nextNo, int colNo) const;
    void readMove_PGN_CC__(QTextStream& stream);
    void writeMove_PGN_CC__(QTextStream& stream) const;

    // PGN_ZH、PGN_CC格式解析不是严格按深度搜索或广度搜索，因此设置数值不能嵌入每步添加着法，只能最后统一设置
    void setMoveNums__();

    void setFEN__(const QString& fen, Color color);
    const QString fen__() const;
    void setBoard__();
    const QString moveInfo__() const;

    PBoard board_;
    PMove rootMove_, curMove_;
    InfoMap info_;
    int movCount_ { 0 }, remCount_ { 0 }, remLenMax_ { 0 }, maxRow_ { 0 }, maxCol_ { 0 };
};

#endif // INSTANCE_H
