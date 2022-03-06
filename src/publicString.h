#ifndef PUBLICSTRING_H
#define PUBLICSTRING_H

#include <QStringList>

const QStringList stringLiterals {
    QStringLiteral("学象棋"),
    QStringLiteral("mainWindow"),
    QStringLiteral("geometry"),
    QStringLiteral("viewmode"),
    QStringLiteral("splitter"),
    QStringLiteral("navIndex"),
    QStringLiteral("recentFileList"),
    QStringLiteral("file"),
    QStringLiteral("activeFileNames"),

    QStringLiteral("leftShow"),
    QStringLiteral("rightShow"),
    QStringLiteral("downShow"),
    QStringLiteral("rightTabIndex"),
    QStringLiteral("downTabIndex"),
    QStringLiteral("winGeometry"),

    QStringLiteral("sceneOption"),
    QStringLiteral("leavePieceOrder"),
    QStringLiteral("backImageFile"),
    QStringLiteral("pieceImageDir"),
    QStringLiteral("moveAnimated"),
    QStringLiteral("moveSound")
};

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
    RIGHTTABINDEX,
    DOWNTABINDEX,
    WINGEOMETRY,

    // 棋盘场景
    SCENEOPTION,
    LEAVEPIECEORDER,
    BACKIMAGEFILE,
    PIECEIMAGEDIR,
    MOVEANIMATED,
    MOVESOUND
};

#endif // PUBLICSTRING_H
