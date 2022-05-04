#include "piece.h"
#include "piecebase.h"

Piece::Piece(PieceColor color, PieceKind kind)
    : color_(color)
    , kind_(kind)
    , seat_(Q_NULLPTR)
{
}

QList<QList<QList<Piece*>>> Piece::creatPieces()
{
    QList<QList<QList<Piece*>>> pieces;
    for (PieceColor color : PieceBase::ALLCOLORS) {
        QList<QList<Piece*>> colorPieces;
        for (PieceKind kind : PieceBase::ALLKINDS) {
            int num { PieceBase::getKindNum(kind) };
            QList<Piece*> kindPieces;
            for (int i = 0; i < num; ++i)
                kindPieces.append(new Piece(color, kind));
            colorPieces.append(kindPieces);
        }
        pieces.append(colorPieces);
    }

    return pieces;
}

QChar Piece::ch() const
{
    return PieceBase::getCh(color_, kind_);
}

QChar Piece::name() const
{
    return PieceBase::getName(color_, kind_);
}

QChar Piece::printName() const
{
    return PieceBase::getPrintName(color_, kind_);
}

QString Piece::toString() const
{
    return QString("%1%2%3").arg(PieceBase::getColorName(color_)).arg(printName()).arg(ch());
}
