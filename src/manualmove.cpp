#include "manualmove.h"
#include "board.h"
#include "manualmoveiterator.h"
#include "move.h"
#include "piece.h"
#include "piecebase.h"
#include "seat.h"
#include "seatbase.h"

#ifdef DEBUG
#include "seat.h"
#include "tools.h"
#endif

ManualMove::ManualMove(const Board *board)
    : board_(board), rootMove_(new Move), curMove_(rootMove_) {}

ManualMove::~ManualMove() { Move::deleteMove(rootMove_); }

Move *ManualMove::append_coordPair(const CoordPair &coordPair,
                                   const QString &remark) {
  return append_seatPair(board_->getSeatPair(coordPair), remark);
}

Move *ManualMove::append_rowcols(const QString &rowcols,
                                 const QString &remark) {
  return append_coordPair(SeatBase::coordPair(rowcols), remark);
}

Move *ManualMove::append_iccs(const QString &iccs, const QString &remark) {
  CoordPair coordPair{
      {PieceBase::getRowFrom(iccs[1]), PieceBase::getColFrom(iccs[0])},
      {PieceBase::getRowFrom(iccs[3]), PieceBase::getColFrom(iccs[2])}};
  return append_coordPair(coordPair, remark);
}

Move *ManualMove::append_zhStr(const QString &zhStr, const QString &remark) {
  return append_seatPair({}, remark, zhStr);
}

void ManualMove::setNumValues() {
  movCount_ = remCount_ = remLenMax_ = maxRow_ = maxCol_ = 0;
  ManualMoveFirstNextIterator firstNextIter(this);
  while (firstNextIter.hasNext()) {
    Move *move = firstNextIter.next();
    move->setNextIndex();
    move->setOtherIndex();

    if (move->isOther())
      ++maxCol_;
    ++movCount_;
    maxCol_ = qMax(maxCol_, move->otherIndex());
    maxRow_ = qMax(maxRow_, move->nextIndex());
    move->setCC_ColIndex(maxCol_); // # 本着在视图中的列数
    if (!move->remark().isEmpty()) {
      ++remCount_;
      remLenMax_ = qMax(remLenMax_, move->remark().length());
    }
  }
}

Move *ManualMove::markDeleteCurMove(bool &isOther, Move *deletedMove) {
  if (curMove_->isRoot())
    return {};

  Move *oldCurMove = curMove_;
  if ((isOther = backOther())) {
    curMove_->setOtherMove(oldCurMove->otherMove());
    oldCurMove->setOtherMove(Q_NULLPTR);
  } else if (backNext())
    curMove_->setNextMove(deletedMove);

  return oldCurMove;
}

PieceColor ManualMove::firstColor() const {
  return rootMove_->hasNext() ? rootMove_->nextMove()->color()
                              : PieceColor::RED;
}

bool ManualMove::goNext() {
  if (!curMove_->hasNext())
    return false;

  curMove_ = curMove_->nextMove();
  curMove_->done();
  return true;
}

bool ManualMove::backNext() {
  if (!curMove_->isNext())
    return false;

  curMove_->undo();
  curMove_ = curMove_->preMove();
  return true;
}

bool ManualMove::goOther() {
  if (!curMove_->hasOther())
    return false;

  curMove_->undo();
  curMove_ = curMove_->otherMove();
  curMove_->done();
  return true;
}

bool ManualMove::backOther() {
  if (!curMove_->isOther())
    return false;

  curMove_->undo(); // 变着回退
  curMove_ = curMove_->preMove();
  curMove_->done(); // 前变执行
  return true;
}

bool ManualMove::goToOther(Move *otherMove) {
  goNext();
  while (curMove_ != otherMove && goOther())
    ;

  return curMove_ == otherMove;
}

bool ManualMove::backAllOtherNext() {
  while (backOther())
    ;

  return backNext();
}

bool ManualMove::backAllNextOther() {
  while (backNext())
    ;

  return backOther();
}

bool ManualMove::goEnd() {
  bool moved{curMove_->hasNext()};
  while (goNext())
    ;

  return moved;
}

bool ManualMove::backStart() {
  if (curMove_->isRoot())
    return false;

  while (backAllOtherNext())
    ;

  return true;
}

bool ManualMove::goTo(Move *move) {
  if (curMove_ == move)
    return false;

  backStart();
  for (auto &move : move->getPrevMoves())
    move->done();

  curMove_ = move;
  return true;
}

bool ManualMove::goIs(bool isOther) { return isOther ? goOther() : goNext(); }

bool ManualMove::backIs(bool isOther) {
  return isOther ? backOther() : backNext();
}

bool ManualMove::go() { return goIs(!curMove_->hasNext()); }

bool ManualMove::back() { return backIs(curMove_->isOther()); }

bool ManualMove::goInc(int inc) {
  bool success{false};
  while (inc-- && goNext())
    success = true;

  return success;
}

bool ManualMove::backInc(int inc) {
  bool success{false};
  while (inc-- && backNext())
    success = true;

  return success;
}

bool ManualMove::isCurMove(Move *move) const { return curMove_ == move; }

const QString &ManualMove::getCurRemark() const { return curMove_->remark(); }

void ManualMove::setCurRemark(const QString &remark) const {
  curMove_->setRemark(remark);
}

SeatPair ManualMove::curSeatPair() const { return curMove_->seatPair(); }

CoordPair ManualMove::curCoordPair() const { return curMove_->coordPair(); }

QString ManualMove::moveInfo() const {
  return QString("【着法深度：%1, 视图宽度：%2, 着法数量：%3, 注解数量：%4, "
                 "注解最长：%5】\n")
      .arg(maxRow())
      .arg(maxCol())
      .arg(getMovCount())
      .arg(getRemCount())
      .arg(getRemLenMax());
}

QString ManualMove::curZhStr() const { return curMove_->zhStr(); }

Move *ManualMove::append_seatPair(SeatPair seatPair, const QString &remark,
                                  QString zhStr) {
  bool isOther{true};
  if (zhStr.isEmpty()) {
    if (seatPair.first->hasPiece())
      isOther = curColorIs(seatPair.first->piece()->color());
  } else
    isOther = curColorIs(PieceBase::getColorFromZh(zhStr.back()));

  if (isOther)
    curMove_->undo();

  if (zhStr.isEmpty()) {
    zhStr = board_->getZhStr(seatPair);
    if (zhStr.isEmpty())
      return Q_NULLPTR;
  } else
    seatPair = board_->getSeatPair(zhStr);

  // 疑难文件通不过下面的检验，需注释
  if (!board_->canMove(seatPair))
    return Q_NULLPTR;

#ifdef DEBUG
  //*// 观察棋盘局面与着法
  Tools::writeTxtFile("test.txt",
                      (seatPair.first->toString() +
                       seatPair.second->toString() + zhStr +
                       (isOther ? " isOther.\n" : "\n")),
                      QIODevice::Append);
  //*/

  // 疑难文件通不过下面的检验，需注释此行
  bool canMove = board_->isCanMove(seatPair);
  if (!canMove) {
    Tools::writeTxtFile("test.txt",
                        QString("失败：\n%1%2%3 %4\n%5\n")
                            .arg(seatPair.first->toString())
                            .arg(seatPair.second->toString())
                            .arg(zhStr)
                            .arg(isOther ? "isOther." : "")
                            .arg(board_->toString()),
                        QIODevice::Append);

    //        qDebug() << __FILE__ << __LINE__;
    //        return Q_NULLPTR;
  }
  Q_ASSERT(canMove);
#endif

  if (isOther)
    curMove_->done();

  Move *move = new Move(curMove_, seatPair, zhStr, remark, isOther);
  goIs(isOther);

#ifdef DEBUG
  //*// 观察棋盘局面与着法
  Tools::writeTxtFile("test.txt",
                      (board_->toString() + curMove_->toString() + '\n'),
                      QIODevice::Append);
  //*/
#endif

  return move;
}

bool ManualMove::curColorIs(PieceColor color) const {
  return curMove_->isRoot() ? false : curMove_->color_done() == color;
}
