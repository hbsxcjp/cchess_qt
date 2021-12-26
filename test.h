#ifndef TEST_H
#define TEST_H

#include <QtTest/QtTest>

class TestPiece : public QObject {
    Q_OBJECT
private slots:
    void toString_data();
    void toString();

    void putString_data();
    void putString();
};

class TestSeat : public QObject {
    Q_OBJECT
private slots:
    void toString_data();
    void toString();

    void FENString_data();
    void FENString();
};

class TestBoard : public QObject {
    Q_OBJECT
private slots:
    void toString_data();
    void toString();

    void canMove_data();
    void canMove();
};

class TestInstance : public QObject {
    Q_OBJECT
private slots:
    void toString_data();
    void toString();

    void toReadWriteFile_data();
    void toReadWriteFile();

    void toReadWriteDir_data();
    void toReadWriteDir();
};

class TestAspect : public QObject {
    Q_OBJECT
private slots:
    void toString_data();
    void toString();

    void readFile_data();
    void readFile();

    void readDir_data();
    void readDir();
};

#endif // TEST_H
