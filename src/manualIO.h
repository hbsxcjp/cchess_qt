#ifndef MANUALIO_H
#define MANUALIO_H
// 中国象棋棋谱存储类型 by-cjp

#include <QFile>
#include <QTextStream>

class Manual;

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

class ManualIO {
public:
    static QString getInfoName(InfoIndex nameIndex);
    static const QStringList& getAllInfoName();
    //    static InfoMap getInitInfoMap();

    static const QStringList& getSuffixNames();
    static QString getSuffixName(StoreType suffixIndex);

    static bool read(Manual* manual, const QString& fileName);
    static bool read(Manual* manual, const InfoMap& infoMap, StoreType storeType = StoreType::PGN_ZH);
    static bool write(const Manual* manual, const QString& fileName);

    static QString getInfoString(const Manual* manual);
    static QString getMoveString(const Manual* manual, StoreType storeType = StoreType::PGN_ZH);
    static QString getString(const Manual* manual, StoreType storeType = StoreType::PGN_ZH);

protected:
    ManualIO() = default; // 允许子类创建实例
    virtual ~ManualIO() = default;

    static void readInfo_(Manual* manual, QTextStream& stream);
    static void writeInfo_(const Manual* manual, QTextStream& stream);

    virtual void readMove_(Manual* /*manual*/, QTextStream& /*stream*/) { }
    virtual bool writeMove_(const Manual* /*manual*/, QTextStream& /*stream*/) const { return false; }

private:
    static ManualIO* getManualIO_(StoreType storeType);
    static ManualIO* getManualIO_(const QString& fileName);

    static void writeInfoToStream_(const InfoMap& infoMap, QTextStream& stream);

    virtual bool read_(Manual* manual, QFile& file) = 0;
    virtual bool write_(const Manual* manual, QFile& file) = 0;
};

class ManualIO_xqf : public ManualIO {

protected:
    using ManualIO::ManualIO;

    virtual bool read_(Manual* manual, QFile& file);
    virtual bool write_(const Manual* manual, QFile& file);
};

class ManualIO_bin : public ManualIO {

protected:
    using ManualIO::ManualIO;

    virtual bool read_(Manual* manual, QFile& file);
    virtual bool write_(const Manual* manual, QFile& file);
};

class ManualIO_json : public ManualIO {

protected:
    using ManualIO::ManualIO;

    virtual bool read_(Manual* manual, QFile& file);
    virtual bool write_(const Manual* manual, QFile& file);
};

class ManualIO_pgn : public ManualIO {

public:
    bool readString(Manual* manual, QString& pgnString);

protected:
    using ManualIO::ManualIO;

    virtual bool read_(Manual* manual, QFile& file);
    virtual bool write_(const Manual* manual, QFile& file);

    void readMove_pgn_iccszh_(Manual* manual, QTextStream& stream, bool isPGN_ZH);
    bool writeMove_pgn_iccszh_(const Manual* manual, QTextStream& stream, bool isPGN_ZH) const;

private:
    virtual bool read_(Manual* manual, QTextStream& stream);
    virtual bool write_(const Manual* manual, QTextStream& stream);
};

class ManualIO_pgn_iccs : public ManualIO_pgn {

protected:
    using ManualIO_pgn::ManualIO_pgn;

    virtual void readMove_(Manual* manual, QTextStream& stream);
    virtual bool writeMove_(const Manual* manual, QTextStream& stream) const;
};

class ManualIO_pgn_zh : public ManualIO_pgn {

protected:
    using ManualIO_pgn::ManualIO_pgn;

    virtual void readMove_(Manual* manual, QTextStream& stream);
    virtual bool writeMove_(const Manual* manual, QTextStream& stream) const;
};

class ManualIO_pgn_cc : public ManualIO_pgn {

protected:
    using ManualIO_pgn::ManualIO_pgn;

    virtual void readMove_(Manual* manual, QTextStream& stream);
    virtual bool writeMove_(const Manual* manual, QTextStream& stream) const;
};

#endif // MANUAL_H
