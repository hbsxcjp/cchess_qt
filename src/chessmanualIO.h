#ifndef CHESSMANUALIO_H
#define CHESSMANUALIO_H
// 中国象棋棋谱存储类型 by-cjp

#include <QFile>
#include <QTextStream>

class ChessManual;

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

class ChessManualIO {
public:
    static QString getInfoName(InfoIndex nameIndex);
    static const QStringList& getAllInfoName();
    //    static InfoMap getInitInfoMap();

    static const QStringList& getSuffixNames();
    static QString getSuffixName(StoreType suffixIndex);

    static bool read(ChessManual* manual, const QString& fileName);
    static bool read(ChessManual* manual, const InfoMap& infoMap, StoreType storeType = StoreType::PGN_ZH);
    static bool write(const ChessManual* manual, const QString& fileName);

    static QString getInfoString(const ChessManual* manual);
    static QString getMoveString(const ChessManual* manual, StoreType storeType = StoreType::PGN_ZH);
    static QString getString(const ChessManual* manual, StoreType storeType = StoreType::PGN_ZH);

protected:
    ChessManualIO() = default; // 允许子类创建实例
    virtual ~ChessManualIO() = default;

    static void readInfo_(ChessManual* manual, QTextStream& stream);
    static void writeInfo_(const ChessManual* manual, QTextStream& stream);

    virtual void readMove_(ChessManual* /*manual*/, QTextStream& /*stream*/) { }
    virtual bool writeMove_(const ChessManual* /*manual*/, QTextStream& /*stream*/) const { return false; }

private:
    static ChessManualIO* getChessManualIO_(StoreType storeType);
    static ChessManualIO* getChessManualIO_(const QString& fileName);

    static void writeInfoToStream_(const InfoMap& infoMap, QTextStream& stream);

    virtual bool read_(ChessManual* manual, QFile& file) = 0;
    virtual bool write_(const ChessManual* manual, QFile& file) = 0;
};

class ChessManualIO_xqf : public ChessManualIO {

protected:
    using ChessManualIO::ChessManualIO;

    virtual bool read_(ChessManual* manual, QFile& file);
    virtual bool write_(const ChessManual* manual, QFile& file);
};

class ChessManualIO_bin : public ChessManualIO {

protected:
    using ChessManualIO::ChessManualIO;

    virtual bool read_(ChessManual* manual, QFile& file);
    virtual bool write_(const ChessManual* manual, QFile& file);
};

class ChessManualIO_json : public ChessManualIO {

protected:
    using ChessManualIO::ChessManualIO;

    virtual bool read_(ChessManual* manual, QFile& file);
    virtual bool write_(const ChessManual* manual, QFile& file);
};

class ChessManualIO_pgn : public ChessManualIO {

public:
    bool readString(ChessManual* manual, QString& pgnString);

protected:
    using ChessManualIO::ChessManualIO;

    virtual bool read_(ChessManual* manual, QFile& file);
    virtual bool write_(const ChessManual* manual, QFile& file);

    void readMove_pgn_iccszh_(ChessManual* manual, QTextStream& stream, bool isPGN_ZH);
    bool writeMove_pgn_iccszh_(const ChessManual* manual, QTextStream& stream, bool isPGN_ZH) const;

private:
    virtual bool read_(ChessManual* manual, QTextStream& stream);
    virtual bool write_(const ChessManual* manual, QTextStream& stream);
};

class ChessManualIO_pgn_iccs : public ChessManualIO_pgn {

protected:
    using ChessManualIO_pgn::ChessManualIO_pgn;

    virtual void readMove_(ChessManual* manual, QTextStream& stream);
    virtual bool writeMove_(const ChessManual* manual, QTextStream& stream) const;
};

class ChessManualIO_pgn_zh : public ChessManualIO_pgn {

protected:
    using ChessManualIO_pgn::ChessManualIO_pgn;

    virtual void readMove_(ChessManual* manual, QTextStream& stream);
    virtual bool writeMove_(const ChessManual* manual, QTextStream& stream) const;
};

class ChessManualIO_pgn_cc : public ChessManualIO_pgn {

protected:
    using ChessManualIO_pgn::ChessManualIO_pgn;

    virtual void readMove_(ChessManual* manual, QTextStream& stream);
    virtual bool writeMove_(const ChessManual* manual, QTextStream& stream) const;
};

#endif // CHESSMANUAL_H
