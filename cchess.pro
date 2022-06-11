QT += core gui network sql
QT += testlib
QT += concurrent
QT += multimedia
QT += core5compat

LIBS += -luser32
LIBS += -lGdi32

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
    src/boardpieces.cpp \
    src/boardscene.cpp \
    src/boardseats.cpp \
    src/boardview.cpp \
    src/command.cpp \
    src/common.cpp \
    src/database.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/manual.cpp \
    src/manualIO.cpp \
    src/manualmove.cpp \
    src/manualmoveiterator.cpp \
    src/manualsubwindow.cpp \
    src/move.cpp \
    src/moveitem.cpp \
    src/moveview.cpp \
    src/piece.cpp \
    src/piecebase.cpp \
    src/pieceitem.cpp \
    src/seat.cpp \
    src/seatbase.cpp \
    src/test.cpp \
    src/tools.cpp

HEADERS += \
    src/aspect.h \
    src/board.h \
    src/boardpieces.h \
    src/boardscene.h \
    src/boardseats.h \
    src/boardview.h \
    src/command.h \
    src/common.h \
    src/database.h \
    src/mainwindow.h \
    src/manual.h \
    src/manualIO.h \
    src/manualmove.h \
    src/manualmoveiterator.h \
    src/manualsubwindow.h \
    src/move.h \
    src/moveitem.h \
    src/moveview.h \
    src/piece.h \
    src/piecebase.h \
    src/pieceitem.h \
    src/seat.h \
    src/seatbase.h \
    src/test.h \
    src/tools.h

FORMS += \
    src/mainwindow.ui \
    src/manualsubwindow.ui

#TRANSLATIONS += \
#    cchess_zh_CN.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RC_ICONS = res/icon/chess.ico

RESOURCES += \
    res.qrc
