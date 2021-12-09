#ifndef INSTANCE_H
#define INSTANCE_H
// 中国象棋棋盘布局类型 by-cjp

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

enum class RecFormat {
    XQF,
    BIN,
    JSON,
    PGN_ICCS,
    PGN_ZH,
    PGN_CC,
    NOTFMT
};

class MoveRec;

class Instance {
public:
    Instance();
    ~Instance();
    Instance(const QString& fileName);
    bool write(const QString& fileName) const;

    // 添加着法，如着法无效则返回空指针
    PMove appendMove_seats(const MovSeat& movseat, const QString& remark, bool isOther);
    PMove appendMove_iccszh(QString iccszhStr, RecFormat fmt, const QString& remark, bool isOther);
    PMove appendMove_zh_tolerateError(QString zhStr, bool isOther);

    bool go(); // 前进
    bool goOther(); // 前进变着
    int goEnd(); // 前进至底
    int goTo(PMove move); // 前进至指定move

    bool back(); // 回退本着，或变着
    bool backNext(); // 本着非变着，则回退一着
    bool backOther(); // 回退变着
    int backToPre(); // 回退至前着，如果当前为变着，则回退至首变着再回退
    int backStart(); // 回退至首着
    int backTo(PMove move); // 后退至指定move
    int goInc(int inc); // 前进或后退数步，返回实际着数

    void changeLayout(ChangeType ct);

    int getMovCount() const { return movCount_; }
    int getRemCount() const { return remCount_; }
    int getRemLenMax() const { return remLenMax_; }
    int getMaxRow() const { return maxRow_; }
    int getMaxCol() const { return maxCol_; }
    const QString& remark() const;

    static const QString getExtName(const RecFormat fmt);
    static RecFormat getRecFormat(const QString& ext_);
    static InfoMap getInitInfoMap();
    static void writeInfoMap(QTextStream& stream, const InfoMap& info);

    // 返回全部着法的记录指针列表; 记录为自分配内存，调用函数负责释放记录内存
    QList<MoveRec> getMoveReces();

    const QString toString();
    const QString toFullString();

private:
    bool read__(const QString& fileName);

    bool readXQF__(const QString& fileName);
    bool readBIN__(const QString& fileName);
    bool writeBIN__(const QString& fileName) const;
    bool readJSON__(const QString& fileName);
    bool writeJSON__(const QString& fileName) const;
    bool readPGN__(const QString& fileName, RecFormat fmt);
    bool writePGN__(const QString& fileName, RecFormat fmt) const;

    void readInfo_PGN__(QTextStream& stream);
    void writeInfo_PGN__(QTextStream& stream) const;
    void readMove_PGN_ICCSZH__(QTextStream& stream, RecFormat fmt);
    void writeMove_PGN_ICCSZH__(QTextStream& stream, RecFormat fmt) const;
    QString remarkNo__(int nextNo, int colNo) const;
    void readMove_PGN_CC__(QTextStream& stream);
    void writeMove_PGN_CC__(QTextStream& stream) const;

    // PGN_ZH、PGN_CC格式解析不是严格按深度搜索或广度搜索，因此设置数值不能嵌入每步添加着法，只能最后统一设置
    void setMoveNums__();

    void setFEN__(const QString& fen, Color color);
    const QString fen__() const;
    const QString moveInfo__() const;

    PBoard board_;
    PMove rootMove_, curMove_;
    InfoMap info_;
    int movCount_ { 0 }, remCount_ { 0 }, remLenMax_ { 0 }, maxRow_ { 0 }, maxCol_ { 0 };
};

struct Record {
    Record() = default;
    Record(int weight, bool killing, bool willKill, bool isCatch, bool isFailed);

    int count; // 历史棋谱中某局面下该着法已发生的次数
    int weight; // 对应某局面的本着价值权重(通过局面评价函数计算)

    bool killing; // 将
    bool willKill; // 杀
    bool isCatch; // 捉
    bool isFailed; // 失败
};

class MoveRec {
public:
    MoveRec(const QString& fen, Color color, int rowcols, const Record& record);

    QString fen;
    Color color;
    int rowcols;

    Record record;
};

QDataStream& operator<<(QDataStream& out, const Record& record);
QDataStream& operator>>(QDataStream& in, Record& record);

void transDir(const QString& dirName, RecFormat fromfmt, RecFormat tofmt, bool isPrint);
bool testInstance();

#endif // INSTANCE_H
