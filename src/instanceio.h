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

enum class InfoIndex {
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
    MOVESTR,
    ROWCOLS,
    CALUATE_ECCOSN,
    NOTINFOINDEX
};

enum class StoreType {
    XQF,
    BIN,
    JSON,
    PGN_ICCS,
    PGN_ZH,
    PGN_CC,
    NOTSTORETYPE
};

class InstanceIO {
public:
    static QString getInfoName(InfoIndex nameIndex);
    static const QStringList& getAllInfoName();
    static InfoMap getInitInfoMap();

    static const QStringList& getSuffixNames();
    static QString getSuffixName(StoreType suffixIndex);
    static StoreType getSuffixIndex(const QString& fileName);

    static bool read(Instance* ins, const QString& fileName);
    static bool write(const Instance* ins, const QString& fileName);

    static bool parsePGN_String(Instance* ins, QString& pgnString, PGN pgn = PGN::ZH);
    static bool constructPGN_String(const Instance* ins, QString& pgnString, PGN pgn = PGN::ZH);
    static bool constructPGN_String(const InfoMap& infoMap, QString& pgnString); // 网页下载棋谱

protected:
    InstanceIO() = default; // 允许子类创建实例
    static InstanceIO* getInstanceIO_(const QString& fileName);

    virtual bool read_(Instance* ins, QFile& file) = 0;
    virtual bool write_(const Instance* ins, QFile& file) = 0;

    static const QString FILETAG_;
    static const QStringList INFONAME_;
    static const QStringList SUFFIXNAME_;
};

class InstanceIO_xqf : public InstanceIO {

protected:
    using InstanceIO::InstanceIO;

    virtual bool read_(Instance* ins, QFile& file);
    virtual bool write_(const Instance* ins, QFile& file);
};

class InstanceIO_bin : public InstanceIO {

protected:
    using InstanceIO::InstanceIO;

    virtual bool read_(Instance* ins, QFile& file);
    virtual bool write_(const Instance* ins, QFile& file);
};

class InstanceIO_json : public InstanceIO {

protected:
    using InstanceIO::InstanceIO;

    virtual bool read_(Instance* ins, QFile& file);
    virtual bool write_(const Instance* ins, QFile& file);
};

class InstanceIO_pgn : public InstanceIO {

public:
    virtual bool parse(Instance* ins, QString& pgnString);
    virtual bool string(const Instance* ins, QString& pgnString);

    void writeInfo(const InfoMap& infoMap, QTextStream& stream) const;

protected:
    using InstanceIO::InstanceIO;

    virtual bool read_(Instance* ins, QFile& file);
    virtual bool write_(const Instance* ins, QFile& file);

    void readInfo_(Instance* ins, QTextStream& stream);
    void writeInfo_(const Instance* ins, QTextStream& stream) const;

    void readMove_pgn_iccszh_(Instance* ins, QTextStream& stream, bool isPGN_ZH);
    bool writeMove_pgn_iccszh_(const Instance* ins, QTextStream& stream, bool isPGN_ZH) const;

    virtual void readMove_(Instance* ins, QTextStream& stream) = 0;
    virtual bool writeMove_(const Instance* ins, QTextStream& stream) const = 0;

private:
    virtual bool input_(Instance* ins, QTextStream& stream);
    virtual bool output_(const Instance* ins, QTextStream& stream);
};

class InstanceIO_pgn_iccs : public InstanceIO_pgn {

protected:
    using InstanceIO_pgn::InstanceIO_pgn;

    virtual void readMove_(Instance* ins, QTextStream& stream);
    virtual bool writeMove_(const Instance* ins, QTextStream& stream) const;
};

class InstanceIO_pgn_zh : public InstanceIO_pgn {

protected:
    using InstanceIO_pgn::InstanceIO_pgn;

    virtual void readMove_(Instance* ins, QTextStream& stream);
    virtual bool writeMove_(const Instance* ins, QTextStream& stream) const;
};

class InstanceIO_pgn_cc : public InstanceIO_pgn {

protected:
    using InstanceIO_pgn::InstanceIO_pgn;

    virtual void readMove_(Instance* ins, QTextStream& stream);
    virtual bool writeMove_(const Instance* ins, QTextStream& stream) const;
};

#endif // INSTANCEIO_H
