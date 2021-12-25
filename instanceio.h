#ifndef INSTANCEIO_H
#define INSTANCEIO_H
// 中国象棋棋谱存储类型 by-cjp

#include <QFile>
#include <QTextStream>

class Instance;

class InstanceIO {
public:
    static QStringList getFileSuffixNames();
    static Instance* getInstance(const QString& fileName);

    static void write(const Instance* ins, const QString& fileName);
    static QString getInstanceString(const Instance* ins);

protected:
    InstanceIO() = default; // 允许子类创建实例
    static InstanceIO* getInstanceIO_(const QString& fileName);

    virtual void read_(Instance* ins, QFile& file) = 0;
    virtual void write_(const Instance* ins, QFile& file) = 0;

    static const QString FILETAG_;
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

protected:
    using InstanceIO::InstanceIO;

    virtual void read_(Instance* ins, QFile& file);
    virtual void write_(const Instance* ins, QFile& file);

    void readInfo_(Instance* ins, QTextStream& stream);
    void writeInfo_(const Instance* ins, QTextStream& stream) const;

    virtual void readMove_(Instance* ins, QTextStream& stream) = 0;
    void readMove_pgn_iccszh_(Instance* ins, QTextStream& stream, bool isPGN_ZH);

    virtual void writeMove_(const Instance* ins, QTextStream& stream) const = 0;
    void writeMove_pgn_iccszh_(const Instance* ins, QTextStream& stream, bool isPGN_ZH) const;
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
