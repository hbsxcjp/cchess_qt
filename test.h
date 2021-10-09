#ifndef TEST_H
#define TEST_H
#include "piece.h"

#include <QtTest/QtTest>

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

#endif // TEST_H
