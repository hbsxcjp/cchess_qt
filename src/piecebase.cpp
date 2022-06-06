#include "piecebase.h"
#include "piece.h"
#include "seatbase.h"

const QList<PieceColor> PieceBase::ALLCOLORS{PieceColor::RED,
                                             PieceColor::BLACK};

const QList<PieceKind> PieceBase::ALLKINDS{
    PieceKind::KING, PieceKind::ADVISOR, PieceKind::BISHOP, PieceKind::KNIGHT,
    PieceKind::ROOK, PieceKind::CANNON,  PieceKind::PAWN};

static const QString PRECHARS("前中后");
static const QString MOVCHARS("退平进");

static const QStringList CHARS{"KABNRCP", "kabnrcp"};

static const QStringList NAMECHARS{"帅仕相马车炮兵", "将士象马车炮卒"};

static const QStringList NUMCHARS{"一二三四五六七八九", "１２３４５６７８９"};

static const QString ICCSCOLCHARS{"abcdefghi"};
static const QString ICCSROWCHARS{"0123456789"};

const QString PieceBase::FENSTR{
    "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR"};
const QChar PieceBase::NULLCHAR{'_'};

static bool isKind(QChar name, const QList<PieceKind> &kinds);
static const QString getPreChars(int length);

QString PieceBase::getZhChars() {
  return (PRECHARS + NAMECHARS.join("") + MOVCHARS + NUMCHARS.join(""));
}

QString PieceBase::getIccsChars() { return ICCSROWCHARS + ICCSCOLCHARS; }

bool PieceBase::isLinePiece(QChar name) {
  return isKind(name, {PieceKind::KING, PieceKind::ROOK, PieceKind::CANNON,
                       PieceKind::PAWN});
}

bool PieceBase::isPawnPiece(QChar name) {
  return isKind(name, {PieceKind::PAWN});
}

bool PieceBase::isStrongePiece(QChar name) {
  return isKind(name, {PieceKind::KNIGHT, PieceKind::ROOK, PieceKind::CANNON,
                       PieceKind::PAWN});
}

bool PieceBase::isAdvisorBishopPiece(QChar name) {
  return isKind(name, {PieceKind::ADVISOR, PieceKind::BISHOP});
}

bool PieceBase::isPiece(QChar name) { return isKind(name, ALLKINDS); }

int PieceBase::getKindNum(PieceKind kind) {
  switch (kind) {
  case PieceKind::KING:
    return 1;
  case PieceKind::ADVISOR:
    return 2;
  case PieceKind::BISHOP:
    return 2;
  case PieceKind::KNIGHT:
    return 2;
  case PieceKind::ROOK:
    return 2;
  case PieceKind::CANNON:
    return 2;
  default: // PieceKind::PAWN:
    return 5;
  }
}

int PieceBase::getRowFrom(QChar ch) { return ICCSROWCHARS.indexOf(ch); }

int PieceBase::getColFrom(QChar ch) { return ICCSCOLCHARS.indexOf(ch); }

QChar PieceBase::getCh(PieceColor color, PieceKind kind) {
  return CHARS[int(color)][int(kind)];
}

QChar PieceBase::getName(PieceColor color, PieceKind kind) {
  return NAMECHARS[int(color)][int(kind)];
}

QChar PieceBase::getPrintName(PieceColor color, PieceKind kind) {
  if (color == PieceColor::BLACK) {
    if (kind == PieceKind::KNIGHT)
      return QChar(L'馬');
    else if (kind == PieceKind::ROOK)
      return QChar(L'車');
    else if (kind == PieceKind::CANNON)
      return QChar(L'砲');
  }

  return getName(color, kind);
}

QChar PieceBase::getColorName(PieceColor color) {
  return color == PieceColor::RED ? QChar(L'红') : QChar(L'黑');
}

PieceKind PieceBase::getKind(QChar ch) {
  int index = CHARS.at(0).indexOf(ch.toUpper());
  Q_ASSERT(index > -1);

  return PieceKind(index);
}

PieceKind PieceBase::getKind(PieceColor color, QChar name) {
  int index = NAMECHARS[int(color)].indexOf(name);
  Q_ASSERT(index > -1);

  return PieceKind(index);
}

PieceColor PieceBase::getColor(QChar ch) {
  return CHARS.at(int(PieceColor::RED)).contains(ch) ? PieceColor::RED
                                                     : PieceColor::BLACK;
}

PieceColor PieceBase::getOtherColor(PieceColor color) {
  return color == PieceColor::RED ? PieceColor::BLACK : PieceColor::RED;
}

PieceColor PieceBase::getColorFromZh(QChar numZh) {
  return NUMCHARS[int(PieceColor::RED)].indexOf(numZh) >= 0 ? PieceColor::RED
                                                            : PieceColor::BLACK;
}

int PieceBase::getIndex(const int seatsLen, const bool isBottom,
                        QChar preChar) {
  int index = getPreChars(seatsLen).indexOf(preChar);
  return isBottom ? seatsLen - 1 - index : index;
}

QChar PieceBase::getIndexChar(int seatsLen, bool isBottom, int index) {
  return getPreChars(seatsLen).at(isBottom ? seatsLen - 1 - index : index);
}

int PieceBase::getMovNum(bool isBottom, QChar movChar) {
  return (MOVCHARS.indexOf(movChar) - 1) * (isBottom ? 1 : -1);
}

QChar PieceBase::getMovChar(bool isSameRow, bool isBottom, bool isLowToUp) {
  return MOVCHARS.at(isSameRow ? 1 : (isBottom == isLowToUp ? 2 : 0));
}

int PieceBase::getNum(PieceColor color, QChar numChar) {
  return NUMCHARS[int(color)].indexOf(numChar) + 1;
}

QChar PieceBase::getNumChar(PieceColor color, int num) {
  return NUMCHARS[int(color)].at(num - 1);
}

QChar PieceBase::getIccsChar(int col) { return ICCSCOLCHARS.at(col); }

int PieceBase::getCol(bool isBottom, int num) {
  return isBottom ? SeatBase::symmetryCol(num - 1) : num - 1;
}

QChar PieceBase::getColChar(PieceColor color, bool isBottom, int col) {
  return NUMCHARS[int(color)].at(isBottom ? SeatBase::symmetryCol(col) : col);
}

bool isKind(QChar name, const QList<PieceKind> &kinds) {
  for (auto &color : PieceBase::ALLCOLORS)
    for (auto &kind : kinds)
      if (NAMECHARS[int(color)][int(kind)] == name)
        return true;

  return false;
}

const QString getPreChars(int length) {
  if (length == 2)
    return QString(PRECHARS).remove(1, 1); // "前后"

  if (length == 3)
    return PRECHARS; // "前中后"

  return NUMCHARS[int(PieceColor::RED)].left(5); // "一二三四五";
}
