#include "boardview.h"
#include "boardscene.h"
#include "instance.h"
#include "piece.h"
#include "pieceitem.h"
#include "publicString.h"
#include "seat.h"
#include <QSettings>

BoardView::BoardView(QWidget* parent)
    : QGraphicsView(parent)
{
}

BoardView::~BoardView()
{
}

void BoardView::setInstance(Instance* instance)
{
    setScene(new BoardScene(instance, this));
}

void BoardView::updatePieceItemShow()
{
    static_cast<BoardScene*>(scene())->updatePieceItemShow();
}
