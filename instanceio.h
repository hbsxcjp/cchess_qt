#ifndef INSTANCEIO_H
#define INSTANCEIO_H
// 中国象棋棋谱存储类型 by-cjp

#include <QFile>
#include <QTextStream>

class Instance;
enum class PGN {
    ICCS,
    ZH,
    CC
};

using InfoMap = QMap<QString, QString>;

enum InfoNameIndex {
    TITLE,
    EVENT,
    DATE,
    SITE,
    BLACK,
    RED,
    OPENING,
    WRITER,
    AUTHOR,
    TYPE,
    RESULT,
    VERSION,
    SOURCE,
    FEN,
    ECCOSN,
    ECCONAME,
    MOVESTR
};

enum SuffixIndex {
    XQF,
    BIN,
    JSON,
    PGN_ICCS,
    PGN_ZH,
    PGN_CC
};

class InstanceIO {
public:
    static QString getInfoName(int nameIndex);
    static InfoMap getInitInfoMap();

    static QString getSuffixName(int suffixIndex);
    static int getSuffixIndex(const QString& fileName);

    static Instance* read(const QString& fileName);
    static void write(const Instance* ins, const QString& fileName);

    static Instance* parseString(QString& pgnString, PGN pgn = PGN::ZH);
    static QString pgnString(const Instance* ins, PGN pgn = PGN::ZH);
    static QString pgnString(const InfoMap& infoMap); // 网页下载棋谱

protected:
    InstanceIO() = default; // 允许子类创建实例
    static InstanceIO* getInstanceIO_(const QString& fileName);

    virtual void read_(Instance* ins, QFile& file) = 0;
    virtual void write_(const Instance* ins, QFile& file) = 0;

    static const QString FILETAG_;
    static const QStringList INFONAME_;
    static const QStringList SUFFIXNAME_;
};

class InstanceIO_xqf : public InstanceIO {

protected:
    using InstanceIO::InstanceIO;

    virtual void read_(Instance* ins, QFile& file);
    virtual void write_(const Instance* ins, QFile& file);
};

class InstanceIO_bin : public InstanceIO {

protected:
    using InstanceIO::InstanceIO;

    virtual void read_(Instance* ins, QFile& file);
    virtual void write_(const Instance* ins, QFile& file);
};

class InstanceIO_json : public InstanceIO {

protected:
    using InstanceIO::InstanceIO;

    virtual void read_(Instance* ins, QFile& file);
    virtual void write_(const Instance* ins, QFile& file);
};

class InstanceIO_pgn : public InstanceIO {

public:
    virtual void parse(Instance* ins, QString& pgnString);
    virtual QString string(const Instance* ins);

    void writeInfo(const InfoMap& infoMap, QTextStream& stream) const;

protected:
    using InstanceIO::InstanceIO;

    virtual void read_(Instance* ins, QFile& file);
    virtual void write_(const Instance* ins, QFile& file);

    void readInfo_(Instance* ins, QTextStream& stream);
    void writeInfo_(const Instance* ins, QTextStream& stream) const;

    void readMove_pgn_iccszh_(Instance* ins, QTextStream& stream, bool isPGN_ZH);
    void writeMove_pgn_iccszh_(const Instance* ins, QTextStream& stream, bool isPGN_ZH) const;

    virtual void readMove_(Instance* ins, QTextStream& stream) = 0;
    virtual void writeMove_(const Instance* ins, QTextStream& stream) const = 0;

private:
    virtual void generate_(Instance* ins, QTextStream& stream);
    virtual void output_(const Instance* ins, QTextStream& stream);
};

class InstanceIO_pgn_iccs : public InstanceIO_pgn {

protected:
    using InstanceIO_pgn::InstanceIO_pgn;

    virtual void readMove_(Instance* ins, QTextStream& stream);
    virtual void writeMove_(const Instance* ins, QTextStream& stream) const;
};

class InstanceIO_pgn_zh : public InstanceIO_pgn {

protected:
    using InstanceIO_pgn::InstanceIO_pgn;

    virtual void readMove_(Instance* ins, QTextStream& stream);
    virtual void writeMove_(const Instance* ins, QTextStream& stream) const;
};

class InstanceIO_pgn_cc : public InstanceIO_pgn {

protected:
    using InstanceIO_pgn::InstanceIO_pgn;

    virtual void readMove_(Instance* ins, QTextStream& stream);
    virtual void writeMove_(const Instance* ins, QTextStream& stream) const;
};

#endif // INSTANCEIO_H
