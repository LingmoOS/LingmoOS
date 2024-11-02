#ifndef NETDETAILSMODEL_H
#define NETDETAILSMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include <QPalette>
#include <QVariant>

#include <QStyledItemDelegate>
#include <QPainter>
#include <QPainterPath>

#include "lingmosdk/lingmosdk-system/libkync.h"
#include "commoninfo.h"

#define LEFTMARGIN  15  // 左边距
#define TOPMARGIN   10  // 上边距
#define TEXTSPACING 30  // 内容文字间距

struct ShowInfo {
    enum InfoType {
        Normal,     // 正常数据
        IPV4,       // IPV4
        IPV6        // IPV6
    };

    InfoType eType = Normal;
    QString strKey;     // 第一列内容
    QString strValue;   // 第二列值
//    bool isIPV = false;  // 是否是IPV数据
};

Q_DECLARE_METATYPE(ShowInfo)

class NetInfoDetailItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit NetInfoDetailItemDelegate(QObject *parent = nullptr);
    virtual ~NetInfoDetailItemDelegate();

    void setFont(const QFont &font)
    {
        m_font = font;
    }
protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QFont  m_font;
};


class NetDetailsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    NetDetailsModel(QObject *parent = nullptr);
    ~NetDetailsModel();

    /**
     * @brief refreshNetifInfo  刷新网络信息
     * @param stNetifInfo       网络信息
     */
    void refreshNetifInfo(const QString &strKey);

private:
    QList<ShowInfo> m_listInfo;

protected:
    int rowCount(const QModelIndex &) const
    {
        return m_listInfo.count();
    }

    int columnCount(const QModelIndex &) const
    {
        return 2;
    }

    QVariant data(const QModelIndex &index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        int iRow = index.row();
        int iColumn = index.column();

        if (iRow >= m_listInfo.count() || iRow < 0)
            return QVariant();

        if (role == Qt::UserRole) {
            return QVariant::fromValue(m_listInfo[iRow]);
        } else if (role == Qt::DisplayRole) {
            switch (iColumn) {
            case 0:
                return m_listInfo[iRow].strKey;
            case 1:
                return m_listInfo[iRow].strValue;
            default:
                break;
            }
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


#endif // NETDETAILSMODEL_H
