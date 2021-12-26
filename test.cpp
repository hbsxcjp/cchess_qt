#include "test.h"
#include "aspect.h"
#include "board.h"
#include "instance.h"
#include "instanceio.h"
#include "move.h"
#include "piece.h"
#include "seat.h"
#include "tools.h"
#include <QFileInfo>

static const QString outputDir { "./output" };

static void addFENs_data()
{
    const QList<QString> fens = {
        "5a3/4ak2r/6R2/8p/9/9/9/B4N2B/4K4/3c5",
        "5k3/9/9/9/9/9/4rp3/2R1C4/4K4/9",
        Pieces::FENStr
    };

    QTest::addColumn<int>("sn");
    QTest::addColumn<QString>("fen");

    for (int i = 0; i < fens.count(); ++i)
        QTest::newRow(fens.at(i).toUtf8()) << i << fens.at(i);
}

static void addXqf_data()
{
    const QList<QString> filenames = {
        "01.XQF",
        "第09局.XQF",
        //        "4四量拨千斤.XQF",
        //        "布局陷阱--飞相局对金钩炮.XQF",
        //        "- 北京张强 (和) 上海胡荣华 (1993.4.27于南京).xqf",
    };

    QTest::addColumn<int>("sn");
    QTest::addColumn<QString>("xqfFileName");

    for (int i = 0; i < filenames.count(); ++i)
        QTest::newRow(QString::number(i).toUtf8()) << i << filenames.at(i);
}

static void addXqfDir_data()
{
    QList<QString> dirfroms {
        "棋谱文件/示例文件.xqf",
        "棋谱文件/象棋杀着大全.xqf",
        //                "棋谱文件/疑难文件.xqf",
        // "棋谱文件/中国象棋棋谱大全.xqf"
    };

    QTest::addColumn<int>("sn");
    QTest::addColumn<QString>("xqfDirName");

    for (int i = 0; i < dirfroms.count(); ++i)
        QTest::newRow(QString::number(i).toUtf8()) << i << dirfroms.at(i);
}

void TestPiece::toString_data()
{
    QTest::addColumn<QString>("result");

    QList<QString> strVec {
        "红帅K", "红仕A", "红仕A", "红相B", "红相B", "红马N", "红马N",
        "红车R", "红车R", "红炮C", "红炮C", "红兵P", "红兵P", "红兵P", "红兵P", "红兵P",
        "黑将k", "黑士a", "黑士a", "黑象b", "黑象b", "黑馬n", "黑馬n",
        "黑車r", "黑車r", "黑砲c", "黑砲c", "黑卒p", "黑卒p", "黑卒p", "黑卒p", "黑卒p"
    };

    QTest::newRow("32 piece") << strVec.join("");
}

void TestPiece::toString()
{
    QFETCH(QString, result);

    Pieces pieces {};
    QCOMPARE(pieces.toString(), result);
}

void TestPiece::putString_data()
{
    QTest::addColumn<int>("sn");
    QTest::addColumn<Side>("homeSide");

    for (int index = 0; index < 2; ++index)
        QTest::newRow(QString("%1").arg(index).toUtf8()) << index << Side(index);
}

void TestPiece::putString()
{
    QFETCH(int, sn);
    QFETCH(Side, homeSide);

    Pieces pieces;
    QString testResult;
    for (auto piece : pieces.getAllPiece(true)) {
        testResult.append(QString("(%1).put(%2):\n%3\n\n")
                              .arg(piece->toString())
                              .arg(int(homeSide))
                              .arg(printSeatCoordList(piece->putSeatCoord(homeSide))));
    }

    QString filename { QString("%1/TestPiece_%2_%3.txt").arg(outputDir).arg(__FUNCTION__).arg(sn) };
#ifdef DEBUG
    Tools::writeTxtFile(filename, testResult, QIODevice::WriteOnly);
#endif

    QCOMPARE(testResult, Tools::readTxtFile(filename));
}

void TestSeat::toString_data()
{
    addFENs_data();
}

void TestSeat::toString()
{
    QFETCH(int, sn);
    QFETCH(QString, fen);

    Seats seats;
    Pieces pieces;
    seats.setFEN(&pieces, fen);

    QString testResult;
    for (ChangeType ct : { ChangeType::NOCHANGE, ChangeType::EXCHANGE,
             ChangeType::ROTATE, ChangeType::SYMMETRY }) {
        seats.changeLayout(&pieces, ct);
        testResult.append(seats.toString())
            .append("  RedLiveSeat:\n" + printSeatList(pieces.getLiveSeatList(Color::RED)))
            .append("\nBlackLiveSeat:\n" + printSeatList(pieces.getLiveSeatList(Color::BLACK)) + "\n\n");
    }

    QString filename { QString("%1/TestSeats_%2_%3.txt").arg(outputDir).arg(__FUNCTION__).arg(sn) };
#ifdef DEBUG
    Tools::writeTxtFile(filename, testResult, QIODevice::WriteOnly);
#endif

    QCOMPARE(testResult, Tools::readTxtFile(filename));
}

void TestSeat::FENString_data()
{
    addFENs_data();
}

void TestSeat::FENString()
{
    QFETCH(int, sn);
    QFETCH(QString, fen);

    Seats seats;
    Pieces pieces;
    seats.setFEN(&pieces, fen);

    QString testResult;
    for (ChangeType ct : { ChangeType::NOCHANGE, ChangeType::EXCHANGE,
             ChangeType::ROTATE, ChangeType::SYMMETRY }) {
        seats.changeLayout(&pieces, ct);
        auto testFen = seats.getFEN();
        auto testChars = Seats::FENToPieChars(testFen);
        testResult.append(QString("ChangeType:%1\n        getFEN():%2\n").arg(int(ct)).arg(testFen))
            .append(QString("pieCharsToFEN_():%1\n").arg(Seats::pieCharsToFEN(testChars)));
    }

    QString filename { QString("%1/TestSeats_%2_%3.txt").arg(outputDir).arg(__FUNCTION__).arg(sn) };
#ifdef DEBUG
    Tools::writeTxtFile(filename, testResult, QIODevice::WriteOnly);
#endif

    QCOMPARE(testResult, Tools::readTxtFile(filename));
}

void TestBoard::toString_data()
{
    addFENs_data();
}

void TestBoard::toString()
{
    QFETCH(int, sn);
    QFETCH(QString, fen);

    Board board {};
    board.setFEN(fen);

    QString testResult;
    for (ChangeType ct : { ChangeType::NOCHANGE, ChangeType::EXCHANGE,
             ChangeType::ROTATE, ChangeType::SYMMETRY }) {
        board.changeLayout(ct);
        testResult.append(board.toString(true)).append('\n');
    }

    QString filename { QString("%1/TestBoard_%2_%3.txt").arg(outputDir).arg(__FUNCTION__).arg(sn) };
#ifdef DEBUG
    Tools::writeTxtFile(filename, testResult, QIODevice::WriteOnly);
#endif

    QCOMPARE(testResult, Tools::readTxtFile(filename));
}

void TestBoard::canMove_data()
{
    addFENs_data();
}

void TestBoard::canMove()
{
    QFETCH(int, sn);
    QFETCH(QString, fen);

    Board board {};
    board.setFEN(fen);

    QString testResult { board.toString() };
    for (Color color : Pieces::allColorList) {
        testResult.append(QString("【%1色棋子】:\n").arg(color == Color::RED ? "红" : "黑"));
        auto seat_seatCoordList = board.allCanMove(color);
        for (auto seat : seat_seatCoordList.keys()) {
            testResult.append(QString("%1: %2\n")
                                  .arg(seat->toString())
                                  .arg(printSeatCoordList(seat_seatCoordList[seat])));
        }

        for (auto& seat : board.getLiveSeatList(color)) {
            QList<QList<SeatCoord>> seatCoordLists = board.canMove(seat->seatCoord());
            testResult.append(QString("(%1).canMove(%2):\n")
                                  .arg(seat->getPiece()->toString())
                                  .arg(printSeatCoord(seat->seatCoord())));

            // 1.可移动位置；2.规则已排除位置；3.同色已排除位置；4.将帅对面或被将军已排除位置
            QStringList caption { "可走", "规则", "同色", "被将" };
            for (int index = 0; index < seatCoordLists.count(); ++index)
                testResult.append(QString("%1: %2\n")
                                      .arg(caption.at(index))
                                      .arg(printSeatCoordList(seatCoordLists.at(index))));
        }

        testResult.append('\n');
    }

    QString filename { QString("%1/TestBoard_%2_%3.txt").arg(outputDir).arg(__FUNCTION__).arg(sn) };
#ifdef DEBUG
    Tools::writeTxtFile(filename, testResult, QIODevice::WriteOnly);
#endif

    QCOMPARE(testResult, Tools::readTxtFile(filename));
}

void TestInstance::toString_data()
{
    addXqf_data();
}

void TestInstance::toString()
{
    QFETCH(int, sn);
    QFETCH(QString, xqfFileName);

    Instance* ins = InstanceIO::read(xqfFileName);
    if (!ins)
        return;

    QString testResult { ins->toFullString() }; // ins.toFullString()
    for (auto ct : { ChangeType::EXCHANGE, ChangeType::ROTATE, ChangeType::SYMMETRY }) {
        ins->changeLayout(ct);
        testResult.append(ins->toString() + '\n');
    }
    delete ins;

    QString filename { QString("%1/TestInstance_%2_%3.txt").arg(outputDir).arg(__FUNCTION__).arg(sn) };
#ifdef DEBUG
    Tools::writeTxtFile(filename, testResult, QIODevice::WriteOnly);
#endif

    QCOMPARE(testResult, Tools::readTxtFile(filename));
}

void TestInstance::toReadWriteFile_data()
{
    addXqf_data();
}

void TestInstance::toReadWriteFile()
{
    QFETCH(int, sn);
    QFETCH(QString, xqfFileName);

    Q_UNUSED(sn);
    Instance* ins = InstanceIO::read(xqfFileName);
    if (!ins)
        return;

    QString xqfTestResult { ins->toString() },
        baseName { QFileInfo(xqfFileName).baseName() };

    //    Tools::writeTxtFile(outputDir + '/' + xqfFileName + ".pgn_cc", xqfTestResult, QIODevice::WriteOnly);
    for (auto& ext : InstanceIO::fileSuffixNames()) {
        if (ext == "xqf")
            continue;

        QString toFileName = QString("%1/%2.%3")
                                 .arg(outputDir)
                                 .arg(baseName)
                                 .arg(ext);
        InstanceIO::write(ins, toFileName);

        Instance* toIns = InstanceIO::read(toFileName);
        if (!toIns)
            return;

        QString testResult { toIns->toString() };
        delete toIns;

        QString filename { QString("%1/TestInstance_%2_%3_%4.txt")
                               .arg(outputDir)
                               .arg(__FUNCTION__)
                               .arg(sn)
                               .arg(ext) };
#ifdef DEBUG
        Tools::writeTxtFile(filename, testResult, QIODevice::WriteOnly);
#endif

        QCOMPARE(xqfTestResult, testResult);
    }

    delete ins;
}

void TestInstance::toReadWriteDir_data()
{
    addXqfDir_data();
}

void TestInstance::toReadWriteDir()
{
    QString outFilename { QString("%1/TestInstance_%2.txt").arg(outputDir).arg(__FUNCTION__) };

    std::function<QString(const QString&, int, int)>
        replaceExt__ = [&](const QString& name, int fromIndex, int toIndex) {
            // 目录名和文件名的扩展名都替换
            return QString(name).replace(InstanceIO::fileSuffixNames().at(fromIndex),
                InstanceIO::fileSuffixNames().at(toIndex), Qt::CaseInsensitive);
        };

    std::function<void(const QString&, int, int)>
        transDir__ = [&](const QString& dirName, int fromIndex, int toIndex) {
            int fcount {}, dcount { 1 }, movCount {}, remCount {}, remLenMax {};

            std::function<void(const QString&, void*)>
                transFile__ = [&](const QString& fileName, void* odata) {
                    Instance* ins = InstanceIO::read(fileName);
                    if (!ins)
                        return;

                    Q_UNUSED(odata);
                    QString toFileName { replaceExt__(fileName, fromIndex, toIndex) };
                    InstanceIO::write(ins, toFileName);

                    ++fcount;
                    if (dirName != QFileInfo(toFileName).absolutePath())
                        ++dcount;
                    movCount += ins->getMovCount();
                    remCount += ins->getRemCount();
                    if (remLenMax < ins->getRemLenMax())
                        remLenMax = ins->getRemLenMax();
                    delete ins;
#ifdef DEBUG
                    Tools::writeTxtFile(outFilename, fileName + "\n" + toFileName + "\n", QIODevice::Append);
#endif
                };

            QDir fdir(dirName);
            QString fromDirName { fdir.absolutePath() },
                toDirName { replaceExt__(fromDirName, fromIndex, toIndex) };
            QDir tdir(toDirName);
            if (!tdir.exists())
                tdir.mkpath(toDirName);

            Tools::operateDir(fromDirName, transFile__, nullptr, true);
            QString qstr { QString("%1 =>%2: %3 files, %4 dirs.\n   movCount: %5, remCount: %6, remLenMax: %7\n\n")
                               .arg(dirName)
                               .arg(tdir.canonicalPath())
                               .arg(fcount)
                               .arg(dcount)
                               .arg(movCount)
                               .arg(remCount)
                               .arg(remLenMax) };
#ifdef DEBUG
            Tools::writeTxtFile(outFilename, qstr, QIODevice::Append);
#endif
        };

    QFETCH(int, sn);
    QFETCH(QString, xqfDirName);

    Q_UNUSED(sn);
    // 转换格式的起止序号, 可调节数据控制测试的覆盖面，综合考虑运行时间
    int fromStart { 0 }, fromEnd { 1 }, toStart { 1 }, toEnd { 2 };
    for (int fromIndex = fromStart; fromIndex != fromEnd; ++fromIndex)
        for (int toIndex = toStart; toIndex != toEnd; ++toIndex) {
            if (toIndex == 0 || toIndex == fromIndex)
                continue;

            transDir__(replaceExt__(xqfDirName, 0, fromIndex), fromIndex, toIndex);
        }
}

void TestAspect::toString_data()
{
    addXqf_data();
}

void TestAspect::toString()
{
    QFETCH(int, sn);
    QFETCH(QString, xqfFileName);

    Instance* ins = InstanceIO::read(xqfFileName);
    if (!ins)
        return;

    Aspects aspects(*ins);
    QString testResult = aspects.toString();
    delete ins;

    QString filename { QString("%1/TestAspect_%2_%3.txt").arg(outputDir).arg(__FUNCTION__).arg(sn) };
#ifdef DEBUG
    Tools::writeTxtFile(filename, testResult, QIODevice::WriteOnly);
#endif

    QCOMPARE(testResult, Tools::readTxtFile(filename));
}

void TestAspect::readFile_data()
{
    addXqf_data();
}

void TestAspect::readFile()
{
    QFETCH(int, sn);
    QFETCH(QString, xqfFileName);

    Instance* ins = InstanceIO::read(xqfFileName);
    if (!ins)
        return;

    Aspects aspects(*ins);
    QString filename { QString("%1/TestAspect_%2_%3.txt").arg(outputDir).arg(__FUNCTION__).arg(sn) };
    aspects.write(filename);
    delete ins;

    Aspects toAspects(filename);
    QCOMPARE(aspects.toString(), toAspects.toString());
}

void TestAspect::readDir_data()
{
    addXqfDir_data();
}

void TestAspect::readDir()
{
    std::function<void(const QString&, void*)>
        readAspectFile__ = [](const QString& fileName, void* aspects) {
            Instance* ins = InstanceIO::read(fileName);
            if (!ins)
                return;

            ((Aspects*)aspects)->append(*ins);
            delete ins;
        };

    QFETCH(int, sn);
    QFETCH(QString, xqfDirName);

    Q_UNUSED(sn);
    Aspects aspects;
    Tools::operateDir(xqfDirName, readAspectFile__, &aspects, true);

    QString filename { QString("%1/TestAspect_%2.txt").arg(outputDir).arg(__FUNCTION__) };
#ifdef DEBUG
    Tools::writeTxtFile(filename, aspects.toString(), QIODevice::Append);
#endif
}
