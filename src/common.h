#ifndef COMMON_H
#define COMMON_H

#include <QStringList>

enum StringIndex {
    // 主窗口
    WINDOWTITLE,
    MAINWINDOW,
    GEOMETRY,
    VIEWMODE,
    SPLITTER,
    NAVINDEX,
    RECENTFILELIST,
    FILEKEY,
    ACTIVEFILENAMES,

    // 子窗口
    LEFTSHOW,
    RIGHTSHOW,
    DOWNSHOW,
    MOVEINFOTABINDEX,
    MOVETABINDEX,
    MOVEPGNTEXTINDEX,
    STUDYTABINDEX,
    WINGEOMETRY,

    // 棋盘场景
    SCENEOPTION,
    LEAVEPIECEORDER,
    BACKIMAGEFILE,
    PIECEIMAGEDIR,
    MOVEANIMATED,
    MOVESOUND,
    MOVESOUNDDIR,
};

extern const QStringList stringLiterals;

#endif // COMMON_H
