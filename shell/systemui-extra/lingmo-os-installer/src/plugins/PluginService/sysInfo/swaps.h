#ifndef SWAPS_H
#define SWAPS_H

#include <QList>
namespace KServer {

enum class SwapType {
    Partiton,
    File
};

struct  SwapItem
{
    QString filename;
    SwapType type;
    qint64 size;
    qint64 used;
    int priority;
};

typedef QList<SwapItem> SwapItemList;
SwapItemList parseSwaps();

}

#endif // SWAPS_H
