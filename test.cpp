#include "test.h"
#include "board.h"
#include "move.h"
#include "piece.h"
#include "seat.h"
#include "tools.h"

const QList<QString> fens = {
    //    Pieces::FENStr,
    "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR",
    "5a3/4ak2r/6R2/8p/9/9/9/B4N2B/4K4/3c5"
};

const QString skipExplain { "Save the result to file." };

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
#ifdef OUTPUT_TESTFILE
    Tools::writeTxtFile(filename, testResult, QIODevice::WriteOnly);
    QSKIP(skipExplain.toUtf8());
#else
    QCOMPARE(testResult, Tools::readTxtFile(filename));
#endif
}

void TestSeat::toString_data()
{
    QTest::addColumn<int>("sn");
    QTest::addColumn<QString>("fen");

    for (int i = 0; i < fens.count(); ++i)
        QTest::newRow(fens.at(i).toUtf8()) << i << fens.at(i);
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

    QString filename { QString("TestSeats_toString_%1.txt").arg(sn) };
#ifdef OUTPUT_TESTFILE
    Tools::writeTxtFile(filename, testResult, QIODevice::WriteOnly);
    QSKIP(skipExplain.toUtf8());
#else
    QCOMPARE(testResult, Tools::readTxtFile(filename));
#endif
}

void TestSeat::FENString_data()
{
    QTest::addColumn<int>("sn");
    QTest::addColumn<QString>("fen");

    for (int i = 0; i < fens.count(); ++i)
        QTest::newRow(fens.at(i).toUtf8()) << i << fens.at(i);
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
            .append(QString("pieCharsToFEN_():%1\n").arg(Seats::pieCharsToFEN(testChars)))
            .append(QString("FENTopieChars_():%1\n").arg(testChars))
            .append(QString("   getPieChars():%1\n\n").arg(seats.getPieChars()));
    }

    QString filename { QString("TestSeats_FENString_%1.txt").arg(sn) };
#ifdef OUTPUT_TESTFILE
    Tools::writeTxtFile(filename, testResult, QIODevice::WriteOnly);
    QSKIP(skipExplain.toUtf8());
#else
    QCOMPARE(testResult, Tools::readTxtFile(filename));
#endif
}

void TestBoard::toString_data()
{
    QTest::addColumn<int>("sn");
    QTest::addColumn<QString>("fen");

    for (int i = 0; i < fens.count(); ++i)
        QTest::newRow(fens.at(i).toUtf8()) << i << fens.at(i);
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
#ifdef OUTPUT_TESTFILE
    Tools::writeTxtFile(filename, testResult, QIODevice::WriteOnly);
    QSKIP(skipExplain.toUtf8());
#else
    QCOMPARE(testResult, Tools::readTxtFile(filename));
#endif
}

void TestBoard::canMove_data()
{
    QTest::addColumn<int>("sn");
    QTest::addColumn<QString>("fen");

    for (int i = 0; i < fens.count(); ++i)
        QTest::newRow(fens.at(i).toUtf8()) << i << fens.at(i);
}

void TestBoard::canMove()
{
    QFETCH(int, sn);
    QFETCH(QString, fen);

    Board board {};
    board.setFEN(fen);

    QString testResult { board.toString() };
    //    for (Color color : { Color::RED, Color::BLACK }) {
    //        for (auto& seatPiece : board.getColorSeatPieceList(color)) {
    //            auto seatList = board.canMove(seatPiece.first);
    //            testResult.append(QString("(%1).canMove(%2):\n%3\n\n")
    //                                  .arg(seatPiece.second->toString())
    //                                  .arg(printSeat(seatPiece.first))
    //                                  .arg(printSeatList(seatList)));
    //        }
    //    }

    QString filename { QString("TestBoard_canMoveStr_%1.txt").arg(sn) };
#ifdef OUTPUT_TESTFILE
    Tools::writeTxtFile(filename, testResult, QIODevice::WriteOnly);
    QSKIP(skipExplain.toUtf8());
#else
    QCOMPARE(testResult, Tools::readTxtFile(filename));
#endif
}
