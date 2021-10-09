#include "test.h"
#include "board.h"
#include "piece.h"
#include "tools.h"

void TestPiece::toString_data()
{
    QTest::addColumn<PPiece>("piece");
    QTest::addColumn<QString>("result");

    QVector<QString> strVec {
        "红帅K", "红仕A", "红仕A", "红相B", "红相B", "红马N", "红马N",
        "红车R", "红车R", "红炮C", "红炮C", "红兵P", "红兵P", "红兵P", "红兵P", "红兵P",
        "黑将k", "黑士a", "黑士a", "黑象b", "黑象b", "黑馬n", "黑馬n",
        "黑車r", "黑車r", "黑砲c", "黑砲c", "黑卒p", "黑卒p", "黑卒p", "黑卒p", "黑卒p"
    };

    Pieces pieces {};
    auto allPiece = pieces.getAllPiece();
    for (int i = 0; i < allPiece.count(); ++i) {
        auto piece = allPiece.at(i);
        QString qstr = QString("%1 %2").arg(i).arg(piece->ch());
        QTest::newRow(qstr.toUtf8()) << piece << strVec[i];
    }
}

void TestPiece::toString()
{
    QFETCH(PPiece, piece);
    QFETCH(QString, result);

    QCOMPARE(piece->toString(), result);
}

void TestPiece::putString_data()
{
    QTest::addColumn<PPiece>("piece");
    QTest::addColumn<Seatside>("homeSide");
    QTest::addColumn<QString>("result");

    QVector<QString> strVec { "(红帅K).put(0):"
                              "<0,3><0,4><0,5><1,3><1,4><1,5><2,3><2,4><2,5>【9】",
        "(红仕A).put(0):"
        "<0,3><0,5><1,4><2,3><2,5>【5】",
        "(红相B).put(0):"
        "<0,2><0,6><2,0><2,4><2,8><4,2><4,6>【7】",
        "(红马N).put(0):"
        "<0,0><0,1><0,2><0,3><0,4><0,5><0,6><0,7><0,8>"
        "<1,0><1,1><1,2><1,3><1,4><1,5><1,6><1,7><1,8>"
        "<2,0><2,1><2,2><2,3><2,4><2,5><2,6><2,7><2,8>"
        "<3,0><3,1><3,2><3,3><3,4><3,5><3,6><3,7><3,8>"
        "<4,0><4,1><4,2><4,3><4,4><4,5><4,6><4,7><4,8>"
        "<5,0><5,1><5,2><5,3><5,4><5,5><5,6><5,7><5,8>"
        "<6,0><6,1><6,2><6,3><6,4><6,5><6,6><6,7><6,8>"
        "<7,0><7,1><7,2><7,3><7,4><7,5><7,6><7,7><7,8>"
        "<8,0><8,1><8,2><8,3><8,4><8,5><8,6><8,7><8,8>"
        "<9,0><9,1><9,2><9,3><9,4><9,5><9,6><9,7><9,8>【90】",
        "(红车R).put(0):"
        "<0,0><0,1><0,2><0,3><0,4><0,5><0,6><0,7><0,8>"
        "<1,0><1,1><1,2><1,3><1,4><1,5><1,6><1,7><1,8>"
        "<2,0><2,1><2,2><2,3><2,4><2,5><2,6><2,7><2,8>"
        "<3,0><3,1><3,2><3,3><3,4><3,5><3,6><3,7><3,8>"
        "<4,0><4,1><4,2><4,3><4,4><4,5><4,6><4,7><4,8>"
        "<5,0><5,1><5,2><5,3><5,4><5,5><5,6><5,7><5,8>"
        "<6,0><6,1><6,2><6,3><6,4><6,5><6,6><6,7><6,8>"
        "<7,0><7,1><7,2><7,3><7,4><7,5><7,6><7,7><7,8>"
        "<8,0><8,1><8,2><8,3><8,4><8,5><8,6><8,7><8,8>"
        "<9,0><9,1><9,2><9,3><9,4><9,5><9,6><9,7><9,8>【90】",
        "(红炮C).put(0):"
        "<0,0><0,1><0,2><0,3><0,4><0,5><0,6><0,7><0,8>"
        "<1,0><1,1><1,2><1,3><1,4><1,5><1,6><1,7><1,8>"
        "<2,0><2,1><2,2><2,3><2,4><2,5><2,6><2,7><2,8>"
        "<3,0><3,1><3,2><3,3><3,4><3,5><3,6><3,7><3,8>"
        "<4,0><4,1><4,2><4,3><4,4><4,5><4,6><4,7><4,8>"
        "<5,0><5,1><5,2><5,3><5,4><5,5><5,6><5,7><5,8>"
        "<6,0><6,1><6,2><6,3><6,4><6,5><6,6><6,7><6,8>"
        "<7,0><7,1><7,2><7,3><7,4><7,5><7,6><7,7><7,8>"
        "<8,0><8,1><8,2><8,3><8,4><8,5><8,6><8,7><8,8>"
        "<9,0><9,1><9,2><9,3><9,4><9,5><9,6><9,7><9,8>【90】",
        "(红兵P).put(0):"
        "<3,0><3,2><3,4><3,6><3,8><4,0><4,1><4,2><4,3>"
        "<4,4><4,5><4,6><4,7><4,8><5,0><5,1><5,2><5,3>"
        "<5,4><5,5><5,6><5,7><5,8><6,0><6,1><6,2><6,3>"
        "<6,4><6,5><6,6><6,7><6,8><7,0><7,1><7,2><7,3>"
        "<7,4><7,5><7,6><7,7><7,8><8,0><8,1><8,2><8,3>"
        "<8,4><8,5><8,6><8,7><8,8><9,0><9,1><9,2><9,3>"
        "<9,4><9,5><9,6><9,7><9,8>【59】",
        "(黑将k).put(0):"
        "<0,3><0,4><0,5><1,3><1,4><1,5><2,3><2,4><2,5>【9】",
        "(黑士a).put(0):"
        "<0,3><0,5><1,4><2,3><2,5>【5】",
        "(黑象b).put(0):"
        "<0,2><0,6><2,0><2,4><2,8><4,2><4,6>【7】",
        "(黑馬n).put(0):"
        "<0,0><0,1><0,2><0,3><0,4><0,5><0,6><0,7><0,8>"
        "<1,0><1,1><1,2><1,3><1,4><1,5><1,6><1,7><1,8>"
        "<2,0><2,1><2,2><2,3><2,4><2,5><2,6><2,7><2,8>"
        "<3,0><3,1><3,2><3,3><3,4><3,5><3,6><3,7><3,8>"
        "<4,0><4,1><4,2><4,3><4,4><4,5><4,6><4,7><4,8>"
        "<5,0><5,1><5,2><5,3><5,4><5,5><5,6><5,7><5,8>"
        "<6,0><6,1><6,2><6,3><6,4><6,5><6,6><6,7><6,8>"
        "<7,0><7,1><7,2><7,3><7,4><7,5><7,6><7,7><7,8>"
        "<8,0><8,1><8,2><8,3><8,4><8,5><8,6><8,7><8,8>"
        "<9,0><9,1><9,2><9,3><9,4><9,5><9,6><9,7><9,8>【90】",
        "(黑車r).put(0):"
        "<0,0><0,1><0,2><0,3><0,4><0,5><0,6><0,7><0,8>"
        "<1,0><1,1><1,2><1,3><1,4><1,5><1,6><1,7><1,8>"
        "<2,0><2,1><2,2><2,3><2,4><2,5><2,6><2,7><2,8>"
        "<3,0><3,1><3,2><3,3><3,4><3,5><3,6><3,7><3,8>"
        "<4,0><4,1><4,2><4,3><4,4><4,5><4,6><4,7><4,8>"
        "<5,0><5,1><5,2><5,3><5,4><5,5><5,6><5,7><5,8>"
        "<6,0><6,1><6,2><6,3><6,4><6,5><6,6><6,7><6,8>"
        "<7,0><7,1><7,2><7,3><7,4><7,5><7,6><7,7><7,8>"
        "<8,0><8,1><8,2><8,3><8,4><8,5><8,6><8,7><8,8>"
        "<9,0><9,1><9,2><9,3><9,4><9,5><9,6><9,7><9,8>【90】",
        "(黑砲c).put(0):"
        "<0,0><0,1><0,2><0,3><0,4><0,5><0,6><0,7><0,8>"
        "<1,0><1,1><1,2><1,3><1,4><1,5><1,6><1,7><1,8>"
        "<2,0><2,1><2,2><2,3><2,4><2,5><2,6><2,7><2,8>"
        "<3,0><3,1><3,2><3,3><3,4><3,5><3,6><3,7><3,8>"
        "<4,0><4,1><4,2><4,3><4,4><4,5><4,6><4,7><4,8>"
        "<5,0><5,1><5,2><5,3><5,4><5,5><5,6><5,7><5,8>"
        "<6,0><6,1><6,2><6,3><6,4><6,5><6,6><6,7><6,8>"
        "<7,0><7,1><7,2><7,3><7,4><7,5><7,6><7,7><7,8>"
        "<8,0><8,1><8,2><8,3><8,4><8,5><8,6><8,7><8,8>"
        "<9,0><9,1><9,2><9,3><9,4><9,5><9,6><9,7><9,8>【90】",
        "(黑卒p).put(0):"
        "<3,0><3,2><3,4><3,6><3,8><4,0><4,1><4,2><4,3>"
        "<4,4><4,5><4,6><4,7><4,8><5,0><5,1><5,2><5,3>"
        "<5,4><5,5><5,6><5,7><5,8><6,0><6,1><6,2><6,3>"
        "<6,4><6,5><6,6><6,7><6,8><7,0><7,1><7,2><7,3>"
        "<7,4><7,5><7,6><7,7><7,8><8,0><8,1><8,2><8,3>"
        "<8,4><8,5><8,6><8,7><8,8><9,0><9,1><9,2><9,3>"
        "<9,4><9,5><9,6><9,7><9,8>【59】",
        "(红帅K).put(1):"
        "<9,5><9,4><9,3><8,5><8,4><8,3><7,5><7,4><7,3>【9】",
        "(红仕A).put(1):"
        "<9,5><9,3><8,4><7,5><7,3>【5】",
        "(红相B).put(1):"
        "<9,6><9,2><7,8><7,4><7,0><5,6><5,2>【7】",
        "(红马N).put(1):"
        "<9,8><9,7><9,6><9,5><9,4><9,3><9,2><9,1><9,0>"
        "<8,8><8,7><8,6><8,5><8,4><8,3><8,2><8,1><8,0>"
        "<7,8><7,7><7,6><7,5><7,4><7,3><7,2><7,1><7,0>"
        "<6,8><6,7><6,6><6,5><6,4><6,3><6,2><6,1><6,0>"
        "<5,8><5,7><5,6><5,5><5,4><5,3><5,2><5,1><5,0>"
        "<4,8><4,7><4,6><4,5><4,4><4,3><4,2><4,1><4,0>"
        "<3,8><3,7><3,6><3,5><3,4><3,3><3,2><3,1><3,0>"
        "<2,8><2,7><2,6><2,5><2,4><2,3><2,2><2,1><2,0>"
        "<1,8><1,7><1,6><1,5><1,4><1,3><1,2><1,1><1,0>"
        "<0,8><0,7><0,6><0,5><0,4><0,3><0,2><0,1><0,0>【90】",
        "(红车R).put(1):"
        "<9,8><9,7><9,6><9,5><9,4><9,3><9,2><9,1><9,0>"
        "<8,8><8,7><8,6><8,5><8,4><8,3><8,2><8,1><8,0>"
        "<7,8><7,7><7,6><7,5><7,4><7,3><7,2><7,1><7,0>"
        "<6,8><6,7><6,6><6,5><6,4><6,3><6,2><6,1><6,0>"
        "<5,8><5,7><5,6><5,5><5,4><5,3><5,2><5,1><5,0>"
        "<4,8><4,7><4,6><4,5><4,4><4,3><4,2><4,1><4,0>"
        "<3,8><3,7><3,6><3,5><3,4><3,3><3,2><3,1><3,0>"
        "<2,8><2,7><2,6><2,5><2,4><2,3><2,2><2,1><2,0>"
        "<1,8><1,7><1,6><1,5><1,4><1,3><1,2><1,1><1,0>"
        "<0,8><0,7><0,6><0,5><0,4><0,3><0,2><0,1><0,0>【90】",
        "(红炮C).put(1):"
        "<9,8><9,7><9,6><9,5><9,4><9,3><9,2><9,1><9,0>"
        "<8,8><8,7><8,6><8,5><8,4><8,3><8,2><8,1><8,0>"
        "<7,8><7,7><7,6><7,5><7,4><7,3><7,2><7,1><7,0>"
        "<6,8><6,7><6,6><6,5><6,4><6,3><6,2><6,1><6,0>"
        "<5,8><5,7><5,6><5,5><5,4><5,3><5,2><5,1><5,0>"
        "<4,8><4,7><4,6><4,5><4,4><4,3><4,2><4,1><4,0>"
        "<3,8><3,7><3,6><3,5><3,4><3,3><3,2><3,1><3,0>"
        "<2,8><2,7><2,6><2,5><2,4><2,3><2,2><2,1><2,0>"
        "<1,8><1,7><1,6><1,5><1,4><1,3><1,2><1,1><1,0>"
        "<0,8><0,7><0,6><0,5><0,4><0,3><0,2><0,1><0,0>【90】",
        "(红兵P).put(1):"
        "<6,8><6,6><6,4><6,2><6,0><5,8><5,7><5,6><5,5>"
        "<5,4><5,3><5,2><5,1><5,0><4,8><4,7><4,6><4,5>"
        "<4,4><4,3><4,2><4,1><4,0><3,8><3,7><3,6><3,5>"
        "<3,4><3,3><3,2><3,1><3,0><2,8><2,7><2,6><2,5>"
        "<2,4><2,3><2,2><2,1><2,0><1,8><1,7><1,6><1,5>"
        "<1,4><1,3><1,2><1,1><1,0><0,8><0,7><0,6><0,5>"
        "<0,4><0,3><0,2><0,1><0,0>【59】",
        "(黑将k).put(1):"
        "<9,5><9,4><9,3><8,5><8,4><8,3><7,5><7,4><7,3>【9】",
        "(黑士a).put(1):"
        "<9,5><9,3><8,4><7,5><7,3>【5】",
        "(黑象b).put(1):"
        "<9,6><9,2><7,8><7,4><7,0><5,6><5,2>【7】",
        "(黑馬n).put(1):"
        "<9,8><9,7><9,6><9,5><9,4><9,3><9,2><9,1><9,0>"
        "<8,8><8,7><8,6><8,5><8,4><8,3><8,2><8,1><8,0>"
        "<7,8><7,7><7,6><7,5><7,4><7,3><7,2><7,1><7,0>"
        "<6,8><6,7><6,6><6,5><6,4><6,3><6,2><6,1><6,0>"
        "<5,8><5,7><5,6><5,5><5,4><5,3><5,2><5,1><5,0>"
        "<4,8><4,7><4,6><4,5><4,4><4,3><4,2><4,1><4,0>"
        "<3,8><3,7><3,6><3,5><3,4><3,3><3,2><3,1><3,0>"
        "<2,8><2,7><2,6><2,5><2,4><2,3><2,2><2,1><2,0>"
        "<1,8><1,7><1,6><1,5><1,4><1,3><1,2><1,1><1,0>"
        "<0,8><0,7><0,6><0,5><0,4><0,3><0,2><0,1><0,0>【90】",
        "(黑車r).put(1):"
        "<9,8><9,7><9,6><9,5><9,4><9,3><9,2><9,1><9,0>"
        "<8,8><8,7><8,6><8,5><8,4><8,3><8,2><8,1><8,0>"
        "<7,8><7,7><7,6><7,5><7,4><7,3><7,2><7,1><7,0>"
        "<6,8><6,7><6,6><6,5><6,4><6,3><6,2><6,1><6,0>"
        "<5,8><5,7><5,6><5,5><5,4><5,3><5,2><5,1><5,0>"
        "<4,8><4,7><4,6><4,5><4,4><4,3><4,2><4,1><4,0>"
        "<3,8><3,7><3,6><3,5><3,4><3,3><3,2><3,1><3,0>"
        "<2,8><2,7><2,6><2,5><2,4><2,3><2,2><2,1><2,0>"
        "<1,8><1,7><1,6><1,5><1,4><1,3><1,2><1,1><1,0>"
        "<0,8><0,7><0,6><0,5><0,4><0,3><0,2><0,1><0,0>【90】",
        "(黑砲c).put(1):"
        "<9,8><9,7><9,6><9,5><9,4><9,3><9,2><9,1><9,0>"
        "<8,8><8,7><8,6><8,5><8,4><8,3><8,2><8,1><8,0>"
        "<7,8><7,7><7,6><7,5><7,4><7,3><7,2><7,1><7,0>"
        "<6,8><6,7><6,6><6,5><6,4><6,3><6,2><6,1><6,0>"
        "<5,8><5,7><5,6><5,5><5,4><5,3><5,2><5,1><5,0>"
        "<4,8><4,7><4,6><4,5><4,4><4,3><4,2><4,1><4,0>"
        "<3,8><3,7><3,6><3,5><3,4><3,3><3,2><3,1><3,0>"
        "<2,8><2,7><2,6><2,5><2,4><2,3><2,2><2,1><2,0>"
        "<1,8><1,7><1,6><1,5><1,4><1,3><1,2><1,1><1,0>"
        "<0,8><0,7><0,6><0,5><0,4><0,3><0,2><0,1><0,0>【90】",
        "(黑卒p).put(1):"
        "<6,8><6,6><6,4><6,2><6,0><5,8><5,7><5,6><5,5>"
        "<5,4><5,3><5,2><5,1><5,0><4,8><4,7><4,6><4,5>"
        "<4,4><4,3><4,2><4,1><4,0><3,8><3,7><3,6><3,5>"
        "<3,4><3,3><3,2><3,1><3,0><2,8><2,7><2,6><2,5>"
        "<2,4><2,3><2,2><2,1><2,0><1,8><1,7><1,6><1,5>"
        "<1,4><1,3><1,2><1,1><1,0><0,8><0,7><0,6><0,5>"
        "<0,4><0,3><0,2><0,1><0,0>【59】" };
    static int n = 0;
    int strIndex = 0;
    Pieces pieces;
    auto allPiece = pieces.getAllPiece(true);
    for (int s = 0; s < 2; ++s)
        for (int i = 0; i < allPiece.count(); ++i) {
            auto piece = allPiece.at(i);
            QString qstr = QString("%1 %2,%3").arg(++n).arg(piece->ch()).arg(s);
            QTest::newRow(qstr.toUtf8()) << piece << Seatside(s) << strVec[strIndex++];
        }
}

void TestPiece::putString()
{
    QFETCH(PPiece, piece);
    QFETCH(Seatside, homeSide);
    QFETCH(QString, result);

    QString testResult { piece->putString(homeSide) };
#ifdef CREATE_TESTPIECE_TEXT
    //    QFile file("TestPiece_putString.txt");
    //    file.open(QIODevice::Append);
    //    file.write(testResult.toUtf8());
    //    file.close();
    Tools::writeTxtFile("TestPiece_putString.txt", testResult, QIODevice::Append);
#endif

    QCOMPARE(testResult, result);
}

void TestPiece::moveString_data()
{
    QTest::addColumn<PPiece>("piece");
    QTest::addColumn<Seat>("seat");
    QTest::addColumn<Seatside>("homeSide");
    QTest::addColumn<QString>("result");

    QVector<QString> strVec { "(红帅K).move(<0,3>,0):"
                              "<1,3><0,4>【2】",
        "(红帅K).move(<0,4>,0):"
        "<1,4><0,3><0,5>【3】",
        "(红帅K).move(<0,5>,0):"
        "<1,5><0,4>【2】",
        "(红帅K).move(<1,3>,0):"
        "<2,3><0,3><1,4>【3】",
        "(红帅K).move(<1,4>,0):"
        "<2,4><0,4><1,3><1,5>【4】",
        "(红帅K).move(<1,5>,0):"
        "<2,5><0,5><1,4>【3】",
        "(红帅K).move(<2,3>,0):"
        "<1,3><2,4>【2】",
        "(红帅K).move(<2,4>,0):"
        "<1,4><2,3><2,5>【3】",
        "(红帅K).move(<2,5>,0):"
        "<1,5><2,4>【2】",
        "(红仕A).move(<0,3>,0):"
        "<1,4>【1】",
        "(红仕A).move(<0,5>,0):"
        "<1,4>【1】",
        "(红仕A).move(<1,4>,0):"
        "<0,3><0,5><2,3><2,5>【4】",
        "(红仕A).move(<2,3>,0):"
        "<1,4>【1】",
        "(红仕A).move(<2,5>,0):"
        "<1,4>【1】",
        "(红相B).move(<0,2>,0):"
        "<2,4><2,0>【2】",
        "(红相B).move(<0,6>,0):"
        "<2,8><2,4>【2】",
        "(红相B).move(<2,0>,0):"
        "<4,2><0,2>【2】",
        "(红相B).move(<2,4>,0):"
        "<4,6><4,2><0,6><0,2>【4】",
        "(红相B).move(<2,8>,0):"
        "<4,6><0,6>【2】",
        "(红相B).move(<4,2>,0):"
        "<2,4><2,0>【2】",
        "(红相B).move(<4,6>,0):"
        "<2,8><2,4>【2】",
        "(红马N).move(<0,0>,0):"
        "<1,2><2,1>【2】",
        "(红马N).move(<0,5>,0):"
        "<1,3><1,7><2,4><2,6>【4】",
        "(红马N).move(<1,1>,0):"
        "<0,3><2,3><3,0><3,2>【4】",
        "(红马N).move(<1,6>,0):"
        "<0,4><0,8><2,4><2,8><3,5><3,7>【6】",
        "(红马N).move(<2,2>,0):"
        "<0,1><0,3><1,0><1,4><3,0><3,4><4,1><4,3>【8】",
        "(红马N).move(<2,7>,0):"
        "<0,6><0,8><1,5><3,5><4,6><4,8>【6】",
        "(红马N).move(<3,3>,0):"
        "<1,2><1,4><2,1><2,5><4,1><4,5><5,2><5,4>【8】",
        "(红马N).move(<3,8>,0):"
        "<1,7><2,6><4,6><5,7>【4】",
        "(红马N).move(<4,4>,0):"
        "<2,3><2,5><3,2><3,6><5,2><5,6><6,3><6,5>【8】",
        "(红车R).move(<0,0>,0):"
        "<1,0><2,0><3,0><4,0><5,0><6,0><7,0><8,0><9,0>"
        "<0,1><0,2><0,3><0,4><0,5><0,6><0,7><0,8>【17】",
        "(红车R).move(<0,5>,0):"
        "<1,5><2,5><3,5><4,5><5,5><6,5><7,5><8,5><9,5>"
        "<0,0><0,1><0,2><0,3><0,4><0,6><0,7><0,8>【17】",
        "(红车R).move(<1,1>,0):"
        "<0,1><2,1><3,1><4,1><5,1><6,1><7,1><8,1><9,1>"
        "<1,0><1,2><1,3><1,4><1,5><1,6><1,7><1,8>【17】",
        "(红车R).move(<1,6>,0):"
        "<0,6><2,6><3,6><4,6><5,6><6,6><7,6><8,6><9,6>"
        "<1,0><1,1><1,2><1,3><1,4><1,5><1,7><1,8>【17】",
        "(红车R).move(<2,2>,0):"
        "<0,2><1,2><3,2><4,2><5,2><6,2><7,2><8,2><9,2>"
        "<2,0><2,1><2,3><2,4><2,5><2,6><2,7><2,8>【17】",
        "(红车R).move(<2,7>,0):"
        "<0,7><1,7><3,7><4,7><5,7><6,7><7,7><8,7><9,7>"
        "<2,0><2,1><2,2><2,3><2,4><2,5><2,6><2,8>【17】",
        "(红车R).move(<3,3>,0):"
        "<0,3><1,3><2,3><4,3><5,3><6,3><7,3><8,3><9,3>"
        "<3,0><3,1><3,2><3,4><3,5><3,6><3,7><3,8>【17】",
        "(红车R).move(<3,8>,0):"
        "<0,8><1,8><2,8><4,8><5,8><6,8><7,8><8,8><9,8>"
        "<3,0><3,1><3,2><3,3><3,4><3,5><3,6><3,7>【17】",
        "(红车R).move(<4,4>,0):"
        "<0,4><1,4><2,4><3,4><5,4><6,4><7,4><8,4><9,4>"
        "<4,0><4,1><4,2><4,3><4,5><4,6><4,7><4,8>【17】",
        "(红炮C).move(<0,0>,0):"
        "<1,0><2,0><3,0><4,0><5,0><6,0><7,0><8,0><9,0>"
        "<0,1><0,2><0,3><0,4><0,5><0,6><0,7><0,8>【17】",
        "(红炮C).move(<0,5>,0):"
        "<1,5><2,5><3,5><4,5><5,5><6,5><7,5><8,5><9,5>"
        "<0,0><0,1><0,2><0,3><0,4><0,6><0,7><0,8>【17】",
        "(红炮C).move(<1,1>,0):"
        "<0,1><2,1><3,1><4,1><5,1><6,1><7,1><8,1><9,1>"
        "<1,0><1,2><1,3><1,4><1,5><1,6><1,7><1,8>【17】",
        "(红炮C).move(<1,6>,0):"
        "<0,6><2,6><3,6><4,6><5,6><6,6><7,6><8,6><9,6>"
        "<1,0><1,1><1,2><1,3><1,4><1,5><1,7><1,8>【17】",
        "(红炮C).move(<2,2>,0):"
        "<0,2><1,2><3,2><4,2><5,2><6,2><7,2><8,2><9,2>"
        "<2,0><2,1><2,3><2,4><2,5><2,6><2,7><2,8>【17】",
        "(红炮C).move(<2,7>,0):"
        "<0,7><1,7><3,7><4,7><5,7><6,7><7,7><8,7><9,7>"
        "<2,0><2,1><2,2><2,3><2,4><2,5><2,6><2,8>【17】",
        "(红炮C).move(<3,3>,0):"
        "<0,3><1,3><2,3><4,3><5,3><6,3><7,3><8,3><9,3>"
        "<3,0><3,1><3,2><3,4><3,5><3,6><3,7><3,8>【17】",
        "(红炮C).move(<3,8>,0):"
        "<0,8><1,8><2,8><4,8><5,8><6,8><7,8><8,8><9,8>"
        "<3,0><3,1><3,2><3,3><3,4><3,5><3,6><3,7>【17】",
        "(红炮C).move(<4,4>,0):"
        "<0,4><1,4><2,4><3,4><5,4><6,4><7,4><8,4><9,4>"
        "<4,0><4,1><4,2><4,3><4,5><4,6><4,7><4,8>【17】",
        "(红兵P).move(<3,0>,0):"
        "<4,0>【1】",
        "(红兵P).move(<4,0>,0):"
        "<5,0>【1】",
        "(红兵P).move(<4,5>,0):"
        "<5,5>【1】",
        "(红兵P).move(<5,1>,0):"
        "<6,1><5,0><5,2>【3】",
        "(红兵P).move(<5,6>,0):"
        "<6,6><5,5><5,7>【3】",
        "(红兵P).move(<6,2>,0):"
        "<7,2><6,1><6,3>【3】",
        "(红兵P).move(<6,7>,0):"
        "<7,7><6,6><6,8>【3】",
        "(红兵P).move(<7,3>,0):"
        "<8,3><7,2><7,4>【3】",
        "(红兵P).move(<7,8>,0):"
        "<8,8><7,7>【2】",
        "(黑将k).move(<0,3>,0):"
        "<1,3><0,4>【2】",
        "(黑将k).move(<0,4>,0):"
        "<1,4><0,3><0,5>【3】",
        "(黑将k).move(<0,5>,0):"
        "<1,5><0,4>【2】",
        "(黑将k).move(<1,3>,0):"
        "<2,3><0,3><1,4>【3】",
        "(黑将k).move(<1,4>,0):"
        "<2,4><0,4><1,3><1,5>【4】",
        "(黑将k).move(<1,5>,0):"
        "<2,5><0,5><1,4>【3】",
        "(黑将k).move(<2,3>,0):"
        "<1,3><2,4>【2】",
        "(黑将k).move(<2,4>,0):"
        "<1,4><2,3><2,5>【3】",
        "(黑将k).move(<2,5>,0):"
        "<1,5><2,4>【2】",
        "(黑士a).move(<0,3>,0):"
        "<1,4>【1】",
        "(黑士a).move(<0,5>,0):"
        "<1,4>【1】",
        "(黑士a).move(<1,4>,0):"
        "<0,3><0,5><2,3><2,5>【4】",
        "(黑士a).move(<2,3>,0):"
        "<1,4>【1】",
        "(黑士a).move(<2,5>,0):"
        "<1,4>【1】",
        "(黑象b).move(<0,2>,0):"
        "<2,4><2,0>【2】",
        "(黑象b).move(<0,6>,0):"
        "<2,8><2,4>【2】",
        "(黑象b).move(<2,0>,0):"
        "<4,2><0,2>【2】",
        "(黑象b).move(<2,4>,0):"
        "<4,6><4,2><0,6><0,2>【4】",
        "(黑象b).move(<2,8>,0):"
        "<4,6><0,6>【2】",
        "(黑象b).move(<4,2>,0):"
        "<2,4><2,0>【2】",
        "(黑象b).move(<4,6>,0):"
        "<2,8><2,4>【2】",
        "(黑馬n).move(<0,0>,0):"
        "<1,2><2,1>【2】",
        "(黑馬n).move(<0,5>,0):"
        "<1,3><1,7><2,4><2,6>【4】",
        "(黑馬n).move(<1,1>,0):"
        "<0,3><2,3><3,0><3,2>【4】",
        "(黑馬n).move(<1,6>,0):"
        "<0,4><0,8><2,4><2,8><3,5><3,7>【6】",
        "(黑馬n).move(<2,2>,0):"
        "<0,1><0,3><1,0><1,4><3,0><3,4><4,1><4,3>【8】",
        "(黑馬n).move(<2,7>,0):"
        "<0,6><0,8><1,5><3,5><4,6><4,8>【6】",
        "(黑馬n).move(<3,3>,0):"
        "<1,2><1,4><2,1><2,5><4,1><4,5><5,2><5,4>【8】",
        "(黑馬n).move(<3,8>,0):"
        "<1,7><2,6><4,6><5,7>【4】",
        "(黑馬n).move(<4,4>,0):"
        "<2,3><2,5><3,2><3,6><5,2><5,6><6,3><6,5>【8】",
        "(黑車r).move(<0,0>,0):"
        "<1,0><2,0><3,0><4,0><5,0><6,0><7,0><8,0><9,0>"
        "<0,1><0,2><0,3><0,4><0,5><0,6><0,7><0,8>【17】",
        "(黑車r).move(<0,5>,0):"
        "<1,5><2,5><3,5><4,5><5,5><6,5><7,5><8,5><9,5>"
        "<0,0><0,1><0,2><0,3><0,4><0,6><0,7><0,8>【17】",
        "(黑車r).move(<1,1>,0):"
        "<0,1><2,1><3,1><4,1><5,1><6,1><7,1><8,1><9,1>"
        "<1,0><1,2><1,3><1,4><1,5><1,6><1,7><1,8>【17】",
        "(黑車r).move(<1,6>,0):"
        "<0,6><2,6><3,6><4,6><5,6><6,6><7,6><8,6><9,6>"
        "<1,0><1,1><1,2><1,3><1,4><1,5><1,7><1,8>【17】",
        "(黑車r).move(<2,2>,0):"
        "<0,2><1,2><3,2><4,2><5,2><6,2><7,2><8,2><9,2>"
        "<2,0><2,1><2,3><2,4><2,5><2,6><2,7><2,8>【17】",
        "(黑車r).move(<2,7>,0):"
        "<0,7><1,7><3,7><4,7><5,7><6,7><7,7><8,7><9,7>"
        "<2,0><2,1><2,2><2,3><2,4><2,5><2,6><2,8>【17】",
        "(黑車r).move(<3,3>,0):"
        "<0,3><1,3><2,3><4,3><5,3><6,3><7,3><8,3><9,3>"
        "<3,0><3,1><3,2><3,4><3,5><3,6><3,7><3,8>【17】",
        "(黑車r).move(<3,8>,0):"
        "<0,8><1,8><2,8><4,8><5,8><6,8><7,8><8,8><9,8>"
        "<3,0><3,1><3,2><3,3><3,4><3,5><3,6><3,7>【17】",
        "(黑車r).move(<4,4>,0):"
        "<0,4><1,4><2,4><3,4><5,4><6,4><7,4><8,4><9,4>"
        "<4,0><4,1><4,2><4,3><4,5><4,6><4,7><4,8>【17】",
        "(黑砲c).move(<0,0>,0):"
        "<1,0><2,0><3,0><4,0><5,0><6,0><7,0><8,0><9,0>"
        "<0,1><0,2><0,3><0,4><0,5><0,6><0,7><0,8>【17】",
        "(黑砲c).move(<0,5>,0):"
        "<1,5><2,5><3,5><4,5><5,5><6,5><7,5><8,5><9,5>"
        "<0,0><0,1><0,2><0,3><0,4><0,6><0,7><0,8>【17】",
        "(黑砲c).move(<1,1>,0):"
        "<0,1><2,1><3,1><4,1><5,1><6,1><7,1><8,1><9,1>"
        "<1,0><1,2><1,3><1,4><1,5><1,6><1,7><1,8>【17】",
        "(黑砲c).move(<1,6>,0):"
        "<0,6><2,6><3,6><4,6><5,6><6,6><7,6><8,6><9,6>"
        "<1,0><1,1><1,2><1,3><1,4><1,5><1,7><1,8>【17】",
        "(黑砲c).move(<2,2>,0):"
        "<0,2><1,2><3,2><4,2><5,2><6,2><7,2><8,2><9,2>"
        "<2,0><2,1><2,3><2,4><2,5><2,6><2,7><2,8>【17】",
        "(黑砲c).move(<2,7>,0):"
        "<0,7><1,7><3,7><4,7><5,7><6,7><7,7><8,7><9,7>"
        "<2,0><2,1><2,2><2,3><2,4><2,5><2,6><2,8>【17】",
        "(黑砲c).move(<3,3>,0):"
        "<0,3><1,3><2,3><4,3><5,3><6,3><7,3><8,3><9,3>"
        "<3,0><3,1><3,2><3,4><3,5><3,6><3,7><3,8>【17】",
        "(黑砲c).move(<3,8>,0):"
        "<0,8><1,8><2,8><4,8><5,8><6,8><7,8><8,8><9,8>"
        "<3,0><3,1><3,2><3,3><3,4><3,5><3,6><3,7>【17】",
        "(黑砲c).move(<4,4>,0):"
        "<0,4><1,4><2,4><3,4><5,4><6,4><7,4><8,4><9,4>"
        "<4,0><4,1><4,2><4,3><4,5><4,6><4,7><4,8>【17】",
        "(黑卒p).move(<3,0>,0):"
        "<4,0>【1】",
        "(黑卒p).move(<4,0>,0):"
        "<5,0>【1】",
        "(黑卒p).move(<4,5>,0):"
        "<5,5>【1】",
        "(黑卒p).move(<5,1>,0):"
        "<6,1><5,0><5,2>【3】",
        "(黑卒p).move(<5,6>,0):"
        "<6,6><5,5><5,7>【3】",
        "(黑卒p).move(<6,2>,0):"
        "<7,2><6,1><6,3>【3】",
        "(黑卒p).move(<6,7>,0):"
        "<7,7><6,6><6,8>【3】",
        "(黑卒p).move(<7,3>,0):"
        "<8,3><7,2><7,4>【3】",
        "(黑卒p).move(<7,8>,0):"
        "<8,8><7,7>【2】",
        "(红帅K).move(<9,5>,1):"
        "<8,5><9,4>【2】",
        "(红帅K).move(<9,4>,1):"
        "<8,4><9,3><9,5>【3】",
        "(红帅K).move(<9,3>,1):"
        "<8,3><9,4>【2】",
        "(红帅K).move(<8,5>,1):"
        "<9,5><7,5><8,4>【3】",
        "(红帅K).move(<8,4>,1):"
        "<9,4><7,4><8,3><8,5>【4】",
        "(红帅K).move(<8,3>,1):"
        "<9,3><7,3><8,4>【3】",
        "(红帅K).move(<7,5>,1):"
        "<8,5><7,4>【2】",
        "(红帅K).move(<7,4>,1):"
        "<8,4><7,3><7,5>【3】",
        "(红帅K).move(<7,3>,1):"
        "<8,3><7,4>【2】",
        "(红仕A).move(<9,5>,1):"
        "<8,4>【1】",
        "(红仕A).move(<9,3>,1):"
        "<8,4>【1】",
        "(红仕A).move(<8,4>,1):"
        "<7,3><7,5><9,3><9,5>【4】",
        "(红仕A).move(<7,5>,1):"
        "<8,4>【1】",
        "(红仕A).move(<7,3>,1):"
        "<8,4>【1】",
        "(红相B).move(<9,6>,1):"
        "<7,8><7,4>【2】",
        "(红相B).move(<9,2>,1):"
        "<7,4><7,0>【2】",
        "(红相B).move(<7,8>,1):"
        "<9,6><5,6>【2】",
        "(红相B).move(<7,4>,1):"
        "<9,6><9,2><5,6><5,2>【4】",
        "(红相B).move(<7,0>,1):"
        "<9,2><5,2>【2】",
        "(红相B).move(<5,6>,1):"
        "<7,8><7,4>【2】",
        "(红相B).move(<5,2>,1):"
        "<7,4><7,0>【2】",
        "(红马N).move(<9,8>,1):"
        "<7,7><8,6>【2】",
        "(红马N).move(<9,3>,1):"
        "<7,2><7,4><8,1><8,5>【4】",
        "(红马N).move(<8,7>,1):"
        "<6,6><6,8><7,5><9,5>【4】",
        "(红马N).move(<8,2>,1):"
        "<6,1><6,3><7,0><7,4><9,0><9,4>【6】",
        "(红马N).move(<7,6>,1):"
        "<5,5><5,7><6,4><6,8><8,4><8,8><9,5><9,7>【8】",
        "(红马N).move(<7,1>,1):"
        "<5,0><5,2><6,3><8,3><9,0><9,2>【6】",
        "(红马N).move(<6,5>,1):"
        "<4,4><4,6><5,3><5,7><7,3><7,7><8,4><8,6>【8】",
        "(红马N).move(<6,0>,1):"
        "<4,1><5,2><7,2><8,1>【4】",
        "(红马N).move(<5,4>,1):"
        "<3,3><3,5><4,2><4,6><6,2><6,6><7,3><7,5>【8】",
        "(红车R).move(<9,8>,1):"
        "<0,8><1,8><2,8><3,8><4,8><5,8><6,8><7,8><8,8>"
        "<9,0><9,1><9,2><9,3><9,4><9,5><9,6><9,7>【17】",
        "(红车R).move(<9,3>,1):"
        "<0,3><1,3><2,3><3,3><4,3><5,3><6,3><7,3><8,3>"
        "<9,0><9,1><9,2><9,4><9,5><9,6><9,7><9,8>【17】",
        "(红车R).move(<8,7>,1):"
        "<0,7><1,7><2,7><3,7><4,7><5,7><6,7><7,7><9,7>"
        "<8,0><8,1><8,2><8,3><8,4><8,5><8,6><8,8>【17】",
        "(红车R).move(<8,2>,1):"
        "<0,2><1,2><2,2><3,2><4,2><5,2><6,2><7,2><9,2>"
        "<8,0><8,1><8,3><8,4><8,5><8,6><8,7><8,8>【17】",
        "(红车R).move(<7,6>,1):"
        "<0,6><1,6><2,6><3,6><4,6><5,6><6,6><8,6><9,6>"
        "<7,0><7,1><7,2><7,3><7,4><7,5><7,7><7,8>【17】",
        "(红车R).move(<7,1>,1):"
        "<0,1><1,1><2,1><3,1><4,1><5,1><6,1><8,1><9,1>"
        "<7,0><7,2><7,3><7,4><7,5><7,6><7,7><7,8>【17】",
        "(红车R).move(<6,5>,1):"
        "<0,5><1,5><2,5><3,5><4,5><5,5><7,5><8,5><9,5>"
        "<6,0><6,1><6,2><6,3><6,4><6,6><6,7><6,8>【17】",
        "(红车R).move(<6,0>,1):"
        "<0,0><1,0><2,0><3,0><4,0><5,0><7,0><8,0><9,0>"
        "<6,1><6,2><6,3><6,4><6,5><6,6><6,7><6,8>【17】",
        "(红车R).move(<5,4>,1):"
        "<0,4><1,4><2,4><3,4><4,4><6,4><7,4><8,4><9,4>"
        "<5,0><5,1><5,2><5,3><5,5><5,6><5,7><5,8>【17】",
        "(红炮C).move(<9,8>,1):"
        "<0,8><1,8><2,8><3,8><4,8><5,8><6,8><7,8><8,8>"
        "<9,0><9,1><9,2><9,3><9,4><9,5><9,6><9,7>【17】",
        "(红炮C).move(<9,3>,1):"
        "<0,3><1,3><2,3><3,3><4,3><5,3><6,3><7,3><8,3>"
        "<9,0><9,1><9,2><9,4><9,5><9,6><9,7><9,8>【17】",
        "(红炮C).move(<8,7>,1):"
        "<0,7><1,7><2,7><3,7><4,7><5,7><6,7><7,7><9,7>"
        "<8,0><8,1><8,2><8,3><8,4><8,5><8,6><8,8>【17】",
        "(红炮C).move(<8,2>,1):"
        "<0,2><1,2><2,2><3,2><4,2><5,2><6,2><7,2><9,2>"
        "<8,0><8,1><8,3><8,4><8,5><8,6><8,7><8,8>【17】",
        "(红炮C).move(<7,6>,1):"
        "<0,6><1,6><2,6><3,6><4,6><5,6><6,6><8,6><9,6>"
        "<7,0><7,1><7,2><7,3><7,4><7,5><7,7><7,8>【17】",
        "(红炮C).move(<7,1>,1):"
        "<0,1><1,1><2,1><3,1><4,1><5,1><6,1><8,1><9,1>"
        "<7,0><7,2><7,3><7,4><7,5><7,6><7,7><7,8>【17】",
        "(红炮C).move(<6,5>,1):"
        "<0,5><1,5><2,5><3,5><4,5><5,5><7,5><8,5><9,5>"
        "<6,0><6,1><6,2><6,3><6,4><6,6><6,7><6,8>【17】",
        "(红炮C).move(<6,0>,1):"
        "<0,0><1,0><2,0><3,0><4,0><5,0><7,0><8,0><9,0>"
        "<6,1><6,2><6,3><6,4><6,5><6,6><6,7><6,8>【17】",
        "(红炮C).move(<5,4>,1):"
        "<0,4><1,4><2,4><3,4><4,4><6,4><7,4><8,4><9,4>"
        "<5,0><5,1><5,2><5,3><5,5><5,6><5,7><5,8>【17】",
        "(红兵P).move(<6,8>,1):"
        "<5,8>【1】",
        "(红兵P).move(<5,8>,1):"
        "<4,8>【1】",
        "(红兵P).move(<5,3>,1):"
        "<4,3>【1】",
        "(红兵P).move(<4,7>,1):"
        "<3,7><4,6><4,8>【3】",
        "(红兵P).move(<4,2>,1):"
        "<3,2><4,1><4,3>【3】",
        "(红兵P).move(<3,6>,1):"
        "<2,6><3,5><3,7>【3】",
        "(红兵P).move(<3,1>,1):"
        "<2,1><3,0><3,2>【3】",
        "(红兵P).move(<2,5>,1):"
        "<1,5><2,4><2,6>【3】",
        "(红兵P).move(<2,0>,1):"
        "<1,0><2,1>【2】",
        "(黑将k).move(<9,5>,1):"
        "<8,5><9,4>【2】",
        "(黑将k).move(<9,4>,1):"
        "<8,4><9,3><9,5>【3】",
        "(黑将k).move(<9,3>,1):"
        "<8,3><9,4>【2】",
        "(黑将k).move(<8,5>,1):"
        "<9,5><7,5><8,4>【3】",
        "(黑将k).move(<8,4>,1):"
        "<9,4><7,4><8,3><8,5>【4】",
        "(黑将k).move(<8,3>,1):"
        "<9,3><7,3><8,4>【3】",
        "(黑将k).move(<7,5>,1):"
        "<8,5><7,4>【2】",
        "(黑将k).move(<7,4>,1):"
        "<8,4><7,3><7,5>【3】",
        "(黑将k).move(<7,3>,1):"
        "<8,3><7,4>【2】",
        "(黑士a).move(<9,5>,1):"
        "<8,4>【1】",
        "(黑士a).move(<9,3>,1):"
        "<8,4>【1】",
        "(黑士a).move(<8,4>,1):"
        "<7,3><7,5><9,3><9,5>【4】",
        "(黑士a).move(<7,5>,1):"
        "<8,4>【1】",
        "(黑士a).move(<7,3>,1):"
        "<8,4>【1】",
        "(黑象b).move(<9,6>,1):"
        "<7,8><7,4>【2】",
        "(黑象b).move(<9,2>,1):"
        "<7,4><7,0>【2】",
        "(黑象b).move(<7,8>,1):"
        "<9,6><5,6>【2】",
        "(黑象b).move(<7,4>,1):"
        "<9,6><9,2><5,6><5,2>【4】",
        "(黑象b).move(<7,0>,1):"
        "<9,2><5,2>【2】",
        "(黑象b).move(<5,6>,1):"
        "<7,8><7,4>【2】",
        "(黑象b).move(<5,2>,1):"
        "<7,4><7,0>【2】",
        "(黑馬n).move(<9,8>,1):"
        "<7,7><8,6>【2】",
        "(黑馬n).move(<9,3>,1):"
        "<7,2><7,4><8,1><8,5>【4】",
        "(黑馬n).move(<8,7>,1):"
        "<6,6><6,8><7,5><9,5>【4】",
        "(黑馬n).move(<8,2>,1):"
        "<6,1><6,3><7,0><7,4><9,0><9,4>【6】",
        "(黑馬n).move(<7,6>,1):"
        "<5,5><5,7><6,4><6,8><8,4><8,8><9,5><9,7>【8】",
        "(黑馬n).move(<7,1>,1):"
        "<5,0><5,2><6,3><8,3><9,0><9,2>【6】",
        "(黑馬n).move(<6,5>,1):"
        "<4,4><4,6><5,3><5,7><7,3><7,7><8,4><8,6>【8】",
        "(黑馬n).move(<6,0>,1):"
        "<4,1><5,2><7,2><8,1>【4】",
        "(黑馬n).move(<5,4>,1):"
        "<3,3><3,5><4,2><4,6><6,2><6,6><7,3><7,5>【8】",
        "(黑車r).move(<9,8>,1):"
        "<0,8><1,8><2,8><3,8><4,8><5,8><6,8><7,8><8,8>"
        "<9,0><9,1><9,2><9,3><9,4><9,5><9,6><9,7>【17】",
        "(黑車r).move(<9,3>,1):"
        "<0,3><1,3><2,3><3,3><4,3><5,3><6,3><7,3><8,3>"
        "<9,0><9,1><9,2><9,4><9,5><9,6><9,7><9,8>【17】",
        "(黑車r).move(<8,7>,1):"
        "<0,7><1,7><2,7><3,7><4,7><5,7><6,7><7,7><9,7>"
        "<8,0><8,1><8,2><8,3><8,4><8,5><8,6><8,8>【17】",
        "(黑車r).move(<8,2>,1):"
        "<0,2><1,2><2,2><3,2><4,2><5,2><6,2><7,2><9,2>"
        "<8,0><8,1><8,3><8,4><8,5><8,6><8,7><8,8>【17】",
        "(黑車r).move(<7,6>,1):"
        "<0,6><1,6><2,6><3,6><4,6><5,6><6,6><8,6><9,6>"
        "<7,0><7,1><7,2><7,3><7,4><7,5><7,7><7,8>【17】",
        "(黑車r).move(<7,1>,1):"
        "<0,1><1,1><2,1><3,1><4,1><5,1><6,1><8,1><9,1>"
        "<7,0><7,2><7,3><7,4><7,5><7,6><7,7><7,8>【17】",
        "(黑車r).move(<6,5>,1):"
        "<0,5><1,5><2,5><3,5><4,5><5,5><7,5><8,5><9,5>"
        "<6,0><6,1><6,2><6,3><6,4><6,6><6,7><6,8>【17】",
        "(黑車r).move(<6,0>,1):"
        "<0,0><1,0><2,0><3,0><4,0><5,0><7,0><8,0><9,0>"
        "<6,1><6,2><6,3><6,4><6,5><6,6><6,7><6,8>【17】",
        "(黑車r).move(<5,4>,1):"
        "<0,4><1,4><2,4><3,4><4,4><6,4><7,4><8,4><9,4>"
        "<5,0><5,1><5,2><5,3><5,5><5,6><5,7><5,8>【17】",
        "(黑砲c).move(<9,8>,1):"
        "<0,8><1,8><2,8><3,8><4,8><5,8><6,8><7,8><8,8>"
        "<9,0><9,1><9,2><9,3><9,4><9,5><9,6><9,7>【17】",
        "(黑砲c).move(<9,3>,1):"
        "<0,3><1,3><2,3><3,3><4,3><5,3><6,3><7,3><8,3>"
        "<9,0><9,1><9,2><9,4><9,5><9,6><9,7><9,8>【17】",
        "(黑砲c).move(<8,7>,1):"
        "<0,7><1,7><2,7><3,7><4,7><5,7><6,7><7,7><9,7>"
        "<8,0><8,1><8,2><8,3><8,4><8,5><8,6><8,8>【17】",
        "(黑砲c).move(<8,2>,1):"
        "<0,2><1,2><2,2><3,2><4,2><5,2><6,2><7,2><9,2>"
        "<8,0><8,1><8,3><8,4><8,5><8,6><8,7><8,8>【17】",
        "(黑砲c).move(<7,6>,1):"
        "<0,6><1,6><2,6><3,6><4,6><5,6><6,6><8,6><9,6>"
        "<7,0><7,1><7,2><7,3><7,4><7,5><7,7><7,8>【17】",
        "(黑砲c).move(<7,1>,1):"
        "<0,1><1,1><2,1><3,1><4,1><5,1><6,1><8,1><9,1>"
        "<7,0><7,2><7,3><7,4><7,5><7,6><7,7><7,8>【17】",
        "(黑砲c).move(<6,5>,1):"
        "<0,5><1,5><2,5><3,5><4,5><5,5><7,5><8,5><9,5>"
        "<6,0><6,1><6,2><6,3><6,4><6,6><6,7><6,8>【17】",
        "(黑砲c).move(<6,0>,1):"
        "<0,0><1,0><2,0><3,0><4,0><5,0><7,0><8,0><9,0>"
        "<6,1><6,2><6,3><6,4><6,5><6,6><6,7><6,8>【17】",
        "(黑砲c).move(<5,4>,1):"
        "<0,4><1,4><2,4><3,4><4,4><6,4><7,4><8,4><9,4>"
        "<5,0><5,1><5,2><5,3><5,5><5,6><5,7><5,8>【17】",
        "(黑卒p).move(<6,8>,1):"
        "<5,8>【1】",
        "(黑卒p).move(<5,8>,1):"
        "<4,8>【1】",
        "(黑卒p).move(<5,3>,1):"
        "<4,3>【1】",
        "(黑卒p).move(<4,7>,1):"
        "<3,7><4,6><4,8>【3】",
        "(黑卒p).move(<4,2>,1):"
        "<3,2><4,1><4,3>【3】",
        "(黑卒p).move(<3,6>,1):"
        "<2,6><3,5><3,7>【3】",
        "(黑卒p).move(<3,1>,1):"
        "<2,1><3,0><3,2>【3】",
        "(黑卒p).move(<2,5>,1):"
        "<1,5><2,4><2,6>【3】",
        "(黑卒p).move(<2,0>,1):"
        "<1,0><2,1>【2】" };
    static int n = 0;
    int strIndex = 0;
    Pieces pieces;
    auto allPiece = pieces.getAllPiece(true);
    for (int s = 0; s < 2; ++s)
        for (int p = 0; p < allPiece.count(); ++p) {
            auto piece = allPiece.at(p);
            auto seatList = piece->put(Seatside(s));
            int step = seatList.count() > 9 ? 5 : 1,
                num = qMin(seatList.count(), 9);
            for (int i = 0; i < num; ++i) {
                auto& seat = seatList.at(i * step);
                QString qstr = QString("%1 %2,%3,%4").arg(++n).arg(piece->ch()).arg(printSeat(seat)).arg(s);
                QTest::newRow(qstr.toUtf8()) << piece << seat << Seatside(s) << strVec[strIndex++];
            }
        }
}

void TestPiece::moveString()
{
    QFETCH(PPiece, piece);
    QFETCH(Seat, seat);
    QFETCH(Seatside, homeSide);
    QFETCH(QString, result);

    QString testResult { piece->moveString(seat, homeSide) };
#ifdef CREATE_TESTPIECE_TEXT
    //    QFile file("TestPiece_moveString.txt");
    //    file.open(QIODevice::Append);
    //    file.write(testResult.toUtf8());
    //    file.close();
    Tools::writeTxtFile("TestPiece_moveString.txt", testResult, QIODevice::Append);
#endif

    QCOMPARE(testResult, result);
}

void TestBoard::FENString_data()
{
    QTest::addColumn<QString>("fen");
    QTest::addColumn<QString>("chars");

    QVector<QString> fens {
        FEN,
        "5a3/4ak2r/6R2/8p/9/9/9/B4N2B/4K4/3c5"
    },
        pieceChars {
            "RNBAKABNR__________C_____C_P_P_P_P_P__________________p_p_p_p_p_c_____c__________rnbakabnr",
            "___c_________K____B____N__B___________________________________p______R______ak__r_____a___", "", ""
        };

    for (int i = 0; i < fens.count(); ++i) {
        QString qstr = QString("%1").arg(i);
        QTest::newRow(qstr.toUtf8()) << fens.at(i) << pieceChars.at(i);
    }
}

void TestBoard::FENString()
{
    QFETCH(QString, fen);
    QFETCH(QString, chars);

    Board board {};
    QCOMPARE(board.FENTopieChars_(fen), chars);
    QCOMPARE(board.pieCharsToFEN_(chars), fen);

    board.setFEN(fen);
    QCOMPARE(board.getFEN(), fen);
}

void TestBoard::toString_data()
{
    QTest::addColumn<QString>("fen");
    QTest::addColumn<QString>("txtboard");
    QTest::addColumn<QString>("txtboard_f");

    QVector<QString> fens {
        FEN,
        "5a3/4ak2r/6R2/8p/9/9/9/B4N2B/4K4/3c5"
    },
        txtboard {
            "車━馬━象━士━将━士━象━馬━車\n"
            "┃　│　│　│╲│╱│　│　│　┃\n"
            "┠─┼─┼─┼─╳─┼─┼─┼─┨\n"
            "┃　│　│　│╱│╲│　│　│　┃\n"
            "┠─砲─┼─┼─┼─┼─┼─砲─┨\n"
            "┃　│　│　│　│　│　│　│　┃\n"
            "卒─┼─卒─┼─卒─┼─卒─┼─卒\n"
            "┃　│　│　│　│　│　│　│　┃\n"
            "┠─┴─┴─┴─┴─┴─┴─┴─┨\n"
            "┃　　　　　　　　　　　　　　　┃\n"
            "┠─┬─┬─┬─┬─┬─┬─┬─┨\n"
            "┃　│　│　│　│　│　│　│　┃\n"
            "兵─┼─兵─┼─兵─┼─兵─┼─兵\n"
            "┃　│　│　│　│　│　│　│　┃\n"
            "┠─炮─┼─┼─┼─┼─┼─炮─┨\n"
            "┃　│　│　│╲│╱│　│　│　┃\n"
            "┠─┼─┼─┼─╳─┼─┼─┼─┨\n"
            "┃　│　│　│╱│╲│　│　│　┃\n"
            "车━马━相━仕━帅━仕━相━马━车\n",
            "┏━┯━┯━┯━┯━士━┯━┯━┓\n"
            "┃　│　│　│╲│╱│　│　│　┃\n"
            "┠─┼─┼─┼─士─将─┼─┼─車\n"
            "┃　│　│　│╱│╲│　│　│　┃\n"
            "┠─╬─┼─┼─┼─┼─车─╬─┨\n"
            "┃　│　│　│　│　│　│　│　┃\n"
            "┠─┼─╬─┼─╬─┼─╬─┼─卒\n"
            "┃　│　│　│　│　│　│　│　┃\n"
            "┠─┴─┴─┴─┴─┴─┴─┴─┨\n"
            "┃　　　　　　　　　　　　　　　┃\n"
            "┠─┬─┬─┬─┬─┬─┬─┬─┨\n"
            "┃　│　│　│　│　│　│　│　┃\n"
            "┠─┼─╬─┼─╬─┼─╬─┼─┨\n"
            "┃　│　│　│　│　│　│　│　┃\n"
            "相─╬─┼─┼─┼─马─┼─╬─相\n"
            "┃　│　│　│╲│╱│　│　│　┃\n"
            "┠─┼─┼─┼─帅─┼─┼─┼─┨\n"
            "┃　│　│　│╱│╲│　│　│　┃\n"
            "┗━┷━┷━砲━┷━┷━┷━┷━┛\n"
        },
        txtboard_f {
            "　　　　　　　黑　方　　　　　　　\n"
            "１　２　３　４　５　６　７　８　９\n"
            "車━馬━象━士━将━士━象━馬━車\n"
            "┃　│　│　│╲│╱│　│　│　┃\n"
            "┠─┼─┼─┼─╳─┼─┼─┼─┨\n"
            "┃　│　│　│╱│╲│　│　│　┃\n"
            "┠─砲─┼─┼─┼─┼─┼─砲─┨\n"
            "┃　│　│　│　│　│　│　│　┃\n"
            "卒─┼─卒─┼─卒─┼─卒─┼─卒\n"
            "┃　│　│　│　│　│　│　│　┃\n"
            "┠─┴─┴─┴─┴─┴─┴─┴─┨\n"
            "┃　　　　　　　　　　　　　　　┃\n"
            "┠─┬─┬─┬─┬─┬─┬─┬─┨\n"
            "┃　│　│　│　│　│　│　│　┃\n"
            "兵─┼─兵─┼─兵─┼─兵─┼─兵\n"
            "┃　│　│　│　│　│　│　│　┃\n"
            "┠─炮─┼─┼─┼─┼─┼─炮─┨\n"
            "┃　│　│　│╲│╱│　│　│　┃\n"
            "┠─┼─┼─┼─╳─┼─┼─┼─┨\n"
            "┃　│　│　│╱│╲│　│　│　┃\n"
            "车━马━相━仕━帅━仕━相━马━车\n"
            "九　八　七　六　五　四　三　二　一\n"
            "　　　　　　　红　方　　　　　　　\n",
            "　　　　　　　黑　方　　　　　　　\n"
            "１　２　３　４　５　６　７　８　９\n"
            "┏━┯━┯━┯━┯━士━┯━┯━┓\n"
            "┃　│　│　│╲│╱│　│　│　┃\n"
            "┠─┼─┼─┼─士─将─┼─┼─車\n"
            "┃　│　│　│╱│╲│　│　│　┃\n"
            "┠─╬─┼─┼─┼─┼─车─╬─┨\n"
            "┃　│　│　│　│　│　│　│　┃\n"
            "┠─┼─╬─┼─╬─┼─╬─┼─卒\n"
            "┃　│　│　│　│　│　│　│　┃\n"
            "┠─┴─┴─┴─┴─┴─┴─┴─┨\n"
            "┃　　　　　　　　　　　　　　　┃\n"
            "┠─┬─┬─┬─┬─┬─┬─┬─┨\n"
            "┃　│　│　│　│　│　│　│　┃\n"
            "┠─┼─╬─┼─╬─┼─╬─┼─┨\n"
            "┃　│　│　│　│　│　│　│　┃\n"
            "相─╬─┼─┼─┼─马─┼─╬─相\n"
            "┃　│　│　│╲│╱│　│　│　┃\n"
            "┠─┼─┼─┼─帅─┼─┼─┼─┨\n"
            "┃　│　│　│╱│╲│　│　│　┃\n"
            "┗━┷━┷━砲━┷━┷━┷━┷━┛\n"
            "九　八　七　六　五　四　三　二　一\n"
            "　　　　　　　红　方　　　　　　　\n"
        };

    for (int i = 0; i < fens.count(); ++i) {
        QString qstr = QString("%1").arg(i);
        QTest::newRow(qstr.toUtf8()) << fens.at(i)
                                     << txtboard.at(i) << txtboard_f.at(i);
    }
}

void TestBoard::toString()
{
    QFETCH(QString, fen);
    QFETCH(QString, txtboard);
    QFETCH(QString, txtboard_f);

    Board board {};
    board.setFEN(fen);
    QCOMPARE(board.toString(), txtboard);
    QCOMPARE(board.toString(true), txtboard_f);
}
