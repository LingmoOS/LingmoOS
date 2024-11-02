#include "memdetailsmodel.h"

MemDetailsModel::MemDetailsModel(QObject *parent)
    : QAbstractTableModel(parent)
{

}
MemDetailsModel::~MemDetailsModel()
{

}

void MemDetailsModel::onModelUpdated()
{
    QAbstractTableModel::dataChanged(index(0, 0), index(6, 1));
}
