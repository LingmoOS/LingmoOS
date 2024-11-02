#include "cpudetailsmodel.h"

CpuDetailsModel::CpuDetailsModel(QObject *parent)
    : QAbstractTableModel(parent)
{

}
CpuDetailsModel::~CpuDetailsModel()
{

}

void CpuDetailsModel::onModelUpdated()
{
    QAbstractTableModel::dataChanged(index(0, 0), index(8, 1));
}
