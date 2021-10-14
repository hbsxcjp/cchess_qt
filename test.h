#ifndef TEST_H
#define TEST_H

#include <QtTest/QtTest>

//#define OUTPUT_TESTFILE

class TestPiece : public QObject {
    Q_OBJECT
private slots:
    void toString_data();
    void toString();

    void putString_data();
    void putString();

    void moveString_data();
    void moveString();
};

class TestBoard : public QObject {
    Q_OBJECT
private slots:
    void FENString_data();
    void FENString();

    void toString_data();
    void toString();

    void canMove_data();
    void canMove();
};

#endif // TEST_H