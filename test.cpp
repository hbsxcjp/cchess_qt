#include "test.h"
#include "board.h"
#include "instance.h"
#include "move.h"
#include "piece.h"
#include "seat.h"
#include "tools.h"

const QString skipExplain { "Save the result to file." };

static void addFENs_data()
{
    const QList<QString> fens = {
        Pieces::FENStr,
        "5a3/4ak2r/6R2/8p/9/9/9/B4N2B/4K4/3c5"
    };

    QTest::addColumn<int>("sn");
    QTest::addColumn<QString>("fen");

    for (int i = 0; i < fens.count(); ++i)
        QTest::newRow(fens.at(i).toUtf8()) << i << fens.at(i);
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

    QString filename { QString("TestPiece_putString_%1.txt").arg(sn) };
#ifdef DEBUG
    Tools::writeTxtFile(filename, testResult, QIODevice::WriteOnly);
#endif

    QCOMPARE(testResult, Tools::readTxtFile(filename));
}

void TestSeatsPieces::toString_data()
{
    addFENs_data();
}

void TestSeatsPieces::toString()
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

    QString filename { QString("TestSeats_toString_%1.txt").arg(sn) };
#ifdef DEBUG
    Tools::writeTxtFile(filename, testResult, QIODevice::WriteOnly);
#endif

    QCOMPARE(testResult, Tools::readTxtFile(filename));
}

void TestSeatsPieces::FENString_data()
{
    addFENs_data();
}

void TestSeatsPieces::FENString()
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
            .append(QString("pieCharsToFEN_():%1\n").arg(Seats::pieCharsToFEN(testChars)))
            .append(QString("FENTopieChars_():%1\n").arg(testChars))
            .append(QString("   getPieChars():%1\n\n").arg(seats.getPieChars()));
    }

    QString filename { QString("TestSeats_FENString_%1.txt").arg(sn) };
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

    QString filename { QString("TestBoard_toString_%1.txt").arg(sn) };
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

    QString filename { QString("TestBoard_canMoveStr_%1.txt").arg(sn) };
#ifdef DEBUG
    Tools::writeTxtFile(filename, testResult, QIODevice::WriteOnly);
#endif

    QCOMPARE(testResult, Tools::readTxtFile(filename));
}

void TestInstance::toString_data()
{
    const QList<QString> filenames = {
        "01.XQF",
        "4四量拨千斤.XQF",
        "- 北京张强 (和) 上海胡荣华 (1993.4.27于南京).xqf",
        "布局陷阱--飞相局对金钩炮.XQF"
    };

    QTest::addColumn<int>("sn");
    QTest::addColumn<QString>("xqffilename");

    for (int i = 0; i < filenames.count(); ++i)
        QTest::newRow(filenames.at(i).toUtf8()) << i << filenames.at(i);
}

void TestInstance::toString()
{
    QFETCH(int, sn);
    QFETCH(QString, xqffilename);

    Instance ins(xqffilename);
    QString testResult { ins.toString() }; // ins.toFullString()
    for (auto ct : { ChangeType::EXCHANGE, ChangeType::ROTATE, ChangeType::SYMMETRY }) {
        ins.changeLayout(ct);
        testResult.append(ins.toString() + '\n');
    }

    QString filename { QString("TestInstance_toString_%1.txt").arg(sn) };
#ifdef DEBUG
    Tools::writeTxtFile(filename, testResult, QIODevice::WriteOnly);
//    Tools::writeTxtFile(filename + ".txt", Tools::readTxtFile(filename), QIODevice::WriteOnly);
#endif

    QCOMPARE(testResult, Tools::readTxtFile(filename));
}
