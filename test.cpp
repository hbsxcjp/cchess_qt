#include "test.h"
#include "board.h"
#include "piece.h"
#include "tools.h"

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
    QString toString;
    for (auto& piece : pieces.getAllPiece())
        toString.append(piece->toString());

    QCOMPARE(toString, result);
}

void TestPiece::putString_data()
{
    QTest::addColumn<int>("sn");
    QTest::addColumn<SeatManager::Seatside>("homeSide");

    for (int s = 0; s < 2; ++s)
        QTest::newRow(QString("%1").arg(s).toUtf8()) << s << SeatManager::Seatside(s);
}

void TestPiece::putString()
{
    QFETCH(int, sn);
    QFETCH(SeatManager::Seatside, homeSide);

    Pieces pieces;
    auto allPiece = pieces.getAllPiece(true);
    QString testResult;
    for (int i = 0; i < allPiece.count(); ++i) {
        auto piece = allPiece.at(i);
        testResult.append(QString("(%1).put(%2):\n%3\n\n")
                              .arg(piece->toString())
                              .arg(homeSide)
                              .arg(SeatManager::printSeatList(piece->put(homeSide))));
    }

    QString filename { QString("TestPiece_putString_%1.txt").arg(sn) };
#ifdef OUTPUT_TESTFILE
    Tools::writeTxtFile(filename, testResult, QIODevice::WriteOnly);
    QSKIP(skipExplain.toUtf8());
#else
    QCOMPARE(testResult, Tools::readTxtFile(filename));
#endif
}

void TestPiece::moveString_data()
{
    QTest::addColumn<int>("sn");
    QTest::addColumn<SeatManager::Seatside>("homeSide");

    for (int s = 0; s < 2; ++s)
        QTest::newRow(QString("%1").arg(s).toUtf8()) << s << SeatManager::Seatside(s);
}

void TestPiece::moveString()
{
    QFETCH(int, sn);
    QFETCH(SeatManager::Seatside, homeSide);

    Pieces pieces;
    auto allPiece = pieces.getAllPiece(true);
    QString testResult;
    for (int p = 0; p < allPiece.count(); ++p) {
        auto piece = allPiece.at(p);
        auto seatList = piece->put(homeSide);
        int step = seatList.count() > 9 ? 5 : 1,
            num = qMin(seatList.count(), 9);
        for (int i = 0; i < num; ++i) {
            auto& seat = seatList.at(i * step);
            testResult.append(QString("(%1).move(%2,%3):\n%4\n\n")
                                  .arg(piece->toString())
                                  .arg(SeatManager::printSeat(seat))
                                  .arg(homeSide)
                                  .arg(SeatManager::printSeatList(piece->move(seat, homeSide))));
        }
    }

    QString filename { QString("TestPiece_moveString_%1.txt").arg(sn) };
#ifdef OUTPUT_TESTFILE
    Tools::writeTxtFile(filename, testResult, QIODevice::WriteOnly);
    QSKIP(skipExplain.toUtf8());
#else
    QCOMPARE(testResult, Tools::readTxtFile(filename));
#endif
}

void TestBoard::FENString_data()
{
    QTest::addColumn<int>("sn");
    QTest::addColumn<QString>("fen");

    for (int i = 0; i < fens.count(); ++i)
        QTest::newRow(fens.at(i).toUtf8()) << i << fens.at(i);
}

void TestBoard::FENString()
{
    QFETCH(int, sn);
    QFETCH(QString, fen);

    Board board {};
    board.setFEN(fen);

    QString testResult;
    for (SeatManager::ChangeType ct : { SeatManager::ChangeType::NOCHANGE, SeatManager::ChangeType::EXCHANGE,
             SeatManager::ChangeType::ROTATE, SeatManager::ChangeType::SYMMETRY }) {
        board.changeSide(ct);
        auto testFen = board.getFEN();
        auto testChars = board.FENTopieChars(testFen);
        testResult.append(QString("SeatManager::ChangeType:%1\ngetFEN():%2\n").arg(int(ct)).arg(testFen))
            .append(QString("FENTopieChars_():%1\n").arg(testChars))
            .append(QString("pieCharsToFEN_():%1\n\n").arg(board.pieCharsToFEN(testChars)));
    }

    QString filename { QString("TestBoard_FENString_%1.txt").arg(sn) };
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
    for (SeatManager::ChangeType ct : { SeatManager::ChangeType::NOCHANGE, SeatManager::ChangeType::EXCHANGE,
             SeatManager::ChangeType::ROTATE, SeatManager::ChangeType::SYMMETRY }) {
        board.changeSide(ct);
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
    for (Piece::Color color : { Piece::Color::RED, Piece::Color::BLACK }) {
        for (auto& seatPiece : board.getColorSeatPieceList(color)) {
            auto seatList = board.canMove(seatPiece.first);
            testResult.append(QString("(%1).canMove(%2):\n%3\n\n")
                                  .arg(seatPiece.second->toString())
                                  .arg(SeatManager::printSeat(seatPiece.first))
                                  .arg(SeatManager::printSeatList(seatList)));
        }
    }

    QString filename { QString("TestBoard_canMoveStr_%1.txt").arg(sn) };
#ifdef OUTPUT_TESTFILE
    Tools::writeTxtFile(filename, testResult, QIODevice::WriteOnly);
    QSKIP(skipExplain.toUtf8());
#else
    QCOMPARE(testResult, Tools::readTxtFile(filename));
#endif
}
