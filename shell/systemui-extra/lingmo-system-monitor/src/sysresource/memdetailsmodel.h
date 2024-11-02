#ifndef MEMDETAILSMODEL_H
#define MEMDETAILSMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include <QPalette>

#include "lingmosdk/lingmosdk-base/kyutils.h"
#include "lingmosdk/lingmosdk-system/libkyrtinfo.h"

#include "commoninfo.h"
using namespace commom::format;

class MemDetailsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    MemDetailsModel(QObject *parent = nullptr);
    ~MemDetailsModel();

protected:
    int rowCount(const QModelIndex &) const
    {
        return 7;
    }

    int columnCount(const QModelIndex &) const
    {
        return 2;
    }

    QVariant data(const QModelIndex &index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        int row = index.row();
        int column = index.column();

        if (role == Qt::DisplayRole) {
            switch (row) {
            case 0:
                if (column == 0)
                    return tr("Used"); // 已使用
                else if (column == 1)
                    return tr("Available"); // 可用
                break;
            case 1:
                if (column == 0)
                    return tr("Shared"); // 共享内存
                else if (column == 1)
                    return tr("Cached"); // 高速缓存
                break;
            case 2:
                if (column == 0)
                    return tr("Buffers"); // 数据缓存
                else if (column == 1)
                    return tr("Cached swap"); // 交换缓存区
                break;
            case 3:
                if (column == 0)
                    return tr("Active"); // 活跃的缓冲文件
                else if (column == 1)
                    return tr("Inactive"); // 不活跃的缓冲文件
                break;
            case 4:
                if (column == 0)
                    return tr("Dirty"); // 脏页
                else if (column == 1)
                    return tr("Mapped"); // 映射大小
                break;
            case 5:
                if (column == 0)
                    return tr("Total swap"); // 交换空间大小
                else if (column == 1)
                    return tr("Free swap"); // 可用交换空间
                break;
            case 6:
                if (column == 0)
                    return tr("Slab"); // 内核数据结构缓存
                break;
            default:
                break;
            }
        } else if (role == Qt::UserRole) {
            switch (row) {
            case 0:
                if (column == 0)
                    return memInfo::getUsed();
                else if (column == 1)
                    return memInfo::getAvailable();
                break;
            case 1:
                if (column == 0)
                    return memInfo::getShared();
                else if (column == 1)
                    return memInfo::getCached();
                break;
            case 2:
                if (column == 0)
                    return memInfo::getBuffers();
                else if (column == 1)
                    return memInfo::getCachedSwap();
                break;
            case 3:
                if (column == 0)
                    return memInfo::getActive();
                else if (column == 1)
                    return memInfo::getInActive();
                break;
            case 4:
                if (column == 0)
                    return memInfo::getDirty();
                else if (column == 1)
                    return memInfo::getMapped();
                break;
            case 5:
                if (column == 0)
                    return memInfo::getTotalSwap();
                else if (column == 1)
                    return memInfo::getFreeSwap();
                break;
            case 6:
                if (column == 0)
                    return memInfo::getSlab();
                break;
            default:
                break;
            }
        } else if (role == Qt::TextColorRole) {
            const auto &palette = QPalette();
            return palette.color(QPalette::Text);
        }

        return QVariant();
    }

    Qt::ItemFlags flags(const QModelIndex &) const
    {
        return Qt::NoItemFlags;
    }

public slots:
    void onModelUpdated();
};

#endif // MEMDETAILSMODEL_H
