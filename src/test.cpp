#include "test.h"
#include "aspect.h"
#include "board.h"
#include "boardpieces.h"
#include "boardseats.h"
#include "database.h"
#include "manual.h"
#include "manualIO.h"
#include "manualmove.h"
#include "move.h"
#include "piece.h"
#include "piecebase.h"
#include "seat.h"
#include "seatbase.h"
#include "tools.h"

#include <QFileInfo>

static const QString outputDir { "./output" };

static void addFENs_data()
{
    const QList<QString> fens = {
        "5a3/4ak2r/6R2/8p/9/9/9/B4N2B/4K4/3c5",
        "5k3/9/9/9/9/9/4rp3/2R1C4/4K4/9",
        PieceBase::FENSTR
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
        "4四量拨千斤.XQF",
        "第09局.XQF",
        "布局陷阱--飞相局对金钩炮.XQF",
        "- 北京张强 (和) 上海胡荣华 (1993.4.27于南京).xqf",
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
        "棋谱文件/疑难文件.xqf",
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

    BoardPieces pieces {};
    QCOMPARE(pieces.toString(), result);
}

void TestPiece::putString_data()
{
    QTest::addColumn<int>("sn");
    QTest::addColumn<SeatSide>("homeSide");

    for (int index = 0; index < 2; ++index)
        QTest::newRow(QString("%1").arg(index).toUtf8()) << index << SeatSide(index);
}

void TestPiece::putString()
{
    QFETCH(int, sn);
    QFETCH(SeatSide, homeSide);

    BoardPieces pieces;
    QString testResult;
    for (auto& piece : pieces.getAllPieces()) {
        testResult.append(QString("(%1).put(%2):\n%3\n\n")
                              .arg(piece->toString())
                              .arg(int(homeSide))
                              .arg(getCoordListString(SeatBase::getCanPutCoords(piece->kind(), homeSide))));
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

    BoardSeats seats;
    BoardPieces pieces;
    seats.setFEN(&pieces, fen);

    QString testResult;
    for (ChangeType ct : { ChangeType::NOCHANGE, ChangeType::EXCHANGE,
             ChangeType::ROTATE, ChangeType::SYMMETRY_H }) {
        seats.changeLayout(&pieces, ct);
        testResult.append(seats.toString(PieceColor::RED, false))
            .append("  RedLiveSeat:\n" + getSeatListString(pieces.getLiveSeats(PieceColor::RED)))
            .append("\nBlackLiveSeat:\n" + getSeatListString(pieces.getLiveSeats(PieceColor::BLACK)) + "\n\n");
    }

    QString filename { QString("%1/TestSeat_%2_%3.txt").arg(outputDir).arg(__FUNCTION__).arg(sn) };
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

    BoardSeats seats;
    BoardPieces pieces;
    seats.setFEN(&pieces, fen);

    QString testResult;
    for (ChangeType ct : { ChangeType::NOCHANGE, ChangeType::EXCHANGE,
             ChangeType::ROTATE, ChangeType::SYMMETRY_H }) {
        seats.changeLayout(&pieces, ct);
        auto testFen = seats.getFEN();
        auto testChars = SeatBase::FENToPieChars(testFen);
        testResult.append(QString("ChangeType:%1\n        getFEN():%2\n").arg(int(ct)).arg(testFen))
            .append(QString("pieCharsToFEN_():%1\n").arg(SeatBase::pieCharsToFEN(testChars)));
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
             ChangeType::ROTATE, ChangeType::SYMMETRY_H }) {
        Q_ASSERT(board.changeLayout(ct));
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
    for (PieceColor color : PieceBase::ALLCOLORS) {
        testResult.append(QString("【%1色棋子】:\n").arg(color == PieceColor::RED ? "红" : "黑"));
        auto seatCoords = board.getColorCanMoveCoords(color);
        for (auto seat : seatCoords.keys()) {
            testResult.append(QString("%1: %2\n")
                                  .arg(seat->toString())
                                  .arg(getCoordListString(seatCoords[seat])));
        }

        for (auto& coord : board.getLiveSeatCoordList(color)) {
            QList<QList<Coord>> coordLists = board.getCanMoveCoordLists(coord);
            testResult.append(QString("(%1).canMove(%2):\n")
                                  .arg(board.getPiece(coord)->toString())
                                  .arg(getCoordString(coord)));

            // 1.可移动位置；2.规则已排除位置；3.同色已排除位置；4.将帅对面或被将军已排除位置
            QStringList caption { "可走", "规则", "同色", "被将" };
            for (int index = 0; index < coordLists.count(); ++index)
                testResult.append(QString("%1: %2\n")
                                      .arg(caption.at(index))
                                      .arg(getCoordListString(coordLists.at(index))));
        }

        testResult.append('\n');
    }

    QString filename { QString("%1/TestBoard_%2_%3.txt").arg(outputDir).arg(__FUNCTION__).arg(sn) };
#ifdef DEBUG
    Tools::writeTxtFile(filename, testResult, QIODevice::WriteOnly);
#endif

    QCOMPARE(testResult, Tools::readTxtFile(filename));
}

void TestManual::toString_data()
{
    addXqf_data();
}

void TestManual::toString()
{
    QFETCH(int, sn);
    QFETCH(QString, xqfFileName);

    Manual manual(xqfFileName);
    QString testResult { manual.toFullString() };
    for (auto ct : { ChangeType::EXCHANGE, ChangeType::ROTATE, ChangeType::SYMMETRY_H }) {
        Q_ASSERT(manual.changeLayout(ct));
        testResult.append(manual.toString(StoreType::PGN_CC) + '\n');
    }

    QString filename { QString("%1/TestInstance_%2_%3.txt").arg(outputDir).arg(__FUNCTION__).arg(sn) };
#ifdef DEBUG
    Tools::writeTxtFile(filename, testResult, QIODevice::WriteOnly);
#endif

    QCOMPARE(testResult, Tools::readTxtFile(filename));
}

void TestManual::toReadWriteFile_data()
{
    addXqf_data();
}

void TestManual::toReadWriteFile()
{
    QFETCH(int, sn);
    QFETCH(QString, xqfFileName);

    Q_UNUSED(sn)
    Manual manual(xqfFileName);
    QString xqfTestResult { manual.toString(StoreType::PGN_CC) },
        baseName { QFileInfo(xqfFileName).baseName() };

    //    Tools::writeTxtFile(outputDir + '/' + xqfFileName + ".pgn_cc", xqfTestResult, QIODevice::WriteOnly);
    for (StoreType storeType : { StoreType::BIN, StoreType::JSON,
             StoreType::PGN_ICCS, StoreType::PGN_ZH, StoreType::PGN_CC }) {
        QString ext = ManualIO::getSuffixName(storeType);
        QString toFileName = QString("%1/%2.%3")
                                 .arg(outputDir)
                                 .arg(baseName)
                                 .arg(ext);
        manual.write(toFileName);
        Manual toManual(toFileName);
        QString testResult { toManual.toString(StoreType::PGN_CC) };

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

    //    delete manual;
}

void TestManual::toReadWriteDir_data()
{
    addXqfDir_data();
}

void TestManual::toReadWriteDir()
{
    QString outFilename { QString("%1/TestInstance_%2.txt").arg(outputDir).arg(__FUNCTION__) };

    std::function<QString(const QString&, StoreType, StoreType)>
        replaceExt__ = [&](const QString& name, StoreType fromIndex, StoreType toIndex) {
            // 目录名和文件名的扩展名都替换
            return QString(name).replace(ManualIO::getSuffixName(fromIndex),
                ManualIO::getSuffixName(toIndex), Qt::CaseInsensitive);
        };

    std::function<void(const QString&, StoreType, StoreType)>
        transDir__ = [&](const QString& dirName, StoreType fromIndex, StoreType toIndex) {
            int fcount {}, dcount { 1 }, movCount {}, remCount {}, remLenMax {};

            std::function<void(const QString&, void*)>
                transFile__ = [&](const QString& fileName, void* odata) {
                    Manual manual(fileName);

                    Q_UNUSED(odata)
                    QString toFileName { replaceExt__(fileName, fromIndex, toIndex) };
                    manual.write(toFileName);

                    ++fcount;
                    if (dirName != QFileInfo(toFileName).absolutePath())
                        ++dcount;
                    movCount += manual.manualMove()->getMovCount();
                    remCount += manual.manualMove()->getRemCount();
                    if (remLenMax < manual.manualMove()->getRemLenMax())
                        remLenMax = manual.manualMove()->getRemLenMax();
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

    Q_UNUSED(sn)
    // 转换格式的起止序号, 可调节数据控制测试的覆盖面，综合考虑运行时间
    StoreType fromStart { StoreType::XQF }, fromEnd { StoreType::XQF },
        toStart { StoreType::BIN }, toEnd { StoreType::NOTSTORETYPE };
    for (StoreType fromIndex = fromStart; fromIndex != fromEnd; fromIndex = StoreType(int(fromIndex) + 1))
        for (StoreType toIndex = toStart; toIndex != toEnd; toIndex = StoreType(int(toIndex) + 1)) {
            if (toIndex == StoreType::XQF || toIndex == fromIndex)
                continue;

            transDir__(replaceExt__(xqfDirName, StoreType::XQF, fromIndex), fromIndex, toIndex);
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

    Manual manual(xqfFileName);
    Aspects aspects(manual);
    QString testResult = aspects.toString();

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

    Manual manual(xqfFileName);
    Aspects aspects(manual);
    QString filename { QString("%1/TestAspect_%2_%3.txt").arg(outputDir).arg(__FUNCTION__).arg(sn) };
    aspects.write(filename);

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
            Manual manual(fileName);
            ((Aspects*)aspects)->append(manual);
        };

    QFETCH(int, sn);
    QFETCH(QString, xqfDirName);

    Q_UNUSED(sn)
    Aspects aspects;
    //    Tools::operateDir(xqfDirName, readAspectFile__, &aspects, true);

    QString filename { QString("%1/TestAspect_%2.txt").arg(outputDir).arg(__FUNCTION__) };
#ifdef DEBUG
    Tools::writeTxtFile(filename, aspects.toString(), QIODevice::Append);
#endif
}

void TestInitEcco::initEcco()
{
    //    DataBase dataBase;
    //    dataBase.initEccoLib();

    //    dataBase.downAllXqbaseManual();
    //    dataBase.downSomeXqbaseManual();

    //    dataBase.setRowcolsXqbaseManual(false);
    //    dataBase.checkEccosnXqbaseManual(true);
}
