QT       += core gui network sql
QT += testlib
QT += concurrent
QT += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/aspect.cpp \
    src/board.cpp \
    src/boardgraphicsscene.cpp \
    src/chessform.cpp \
    src/database.cpp \
    src/instance.cpp \
    src/instanceio.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/move.cpp \
    src/movegraphicsitem.cpp \
    src/movegraphicsscene.cpp \
    src/piece.cpp \
    src/piecegraphicsitem.cpp \
    src/seat.cpp \
    src/test.cpp \
    src/tools.cpp

HEADERS += \
    src/aspect.h \
    src/board.h \
    src/boardgraphicsscene.h \
    src/chessform.h \
    src/database.h \
    src/instance.h \
    src/instanceio.h \
    src/mainwindow.h \
    src/move.h \
    src/movegraphicsitem.h \
    src/movegraphicsscene.h \
    src/piece.h \
    src/piecegraphicsitem.h \
    src/publicString.h \
    src/seat.h \
    src/test.h \
    src/tools.h

FORMS += \
    src/chessform.ui \
    src/mainwindow.ui

#TRANSLATIONS += \
#    cchess_zh_CN.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RC_ICONS = res/icon/chess_64.ico

RESOURCES += \
    res.qrc
