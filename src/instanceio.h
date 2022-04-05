#ifndef INSTANCEIO_H
#define INSTANCEIO_H
// 中国象棋棋谱存储类型 by-cjp

#include <QFile>
#include <QTextStream>

class Instance;

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

    static bool read(Instance* ins, const QString& fileName);
    static bool read(Instance* ins, const InfoMap& infoMap, StoreType storeType = StoreType::PGN_ZH);
    static bool write(const Instance* ins, const QString& fileName);

    static QString getInfoString(const Instance* ins);
    static QString getMoveString(const Instance* ins, StoreType storeType = StoreType::PGN_ZH);
    static QString getString(const Instance* ins, StoreType storeType = StoreType::PGN_ZH);

protected:
    InstanceIO() = default; // 允许子类创建实例
    static InstanceIO* getInstanceIO_(StoreType storeType);
    static InstanceIO* getInstanceIO_(const QString& fileName);

    static void readInfo_(Instance* ins, QTextStream& stream);
    static void writeInfo_(const Instance* ins, QTextStream& stream);
    static void writeInfoToStream_(const InfoMap& infoMap, QTextStream& stream);

    virtual void readMove_(Instance* /*ins*/, QTextStream& /*stream*/) { }
    virtual bool writeMove_(const Instance* /*ins*/, QTextStream& /*stream*/) const { return false; }

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
    bool readString(Instance* ins, QString& pgnString);

protected:
    using InstanceIO::InstanceIO;

    virtual bool read_(Instance* ins, QFile& file);
    virtual bool write_(const Instance* ins, QFile& file);

    void readMove_pgn_iccszh_(Instance* ins, QTextStream& stream, bool isPGN_ZH);
    bool writeMove_pgn_iccszh_(const Instance* ins, QTextStream& stream, bool isPGN_ZH) const;

private:
    virtual bool read_(Instance* ins, QTextStream& stream);
    virtual bool write_(const Instance* ins, QTextStream& stream);
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
