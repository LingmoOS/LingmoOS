#include "netdetailsmodel.h"

NetInfoDetailItemDelegate::NetInfoDetailItemDelegate(QObject *parent): QStyledItemDelegate(parent)
{
}

NetInfoDetailItemDelegate::~NetInfoDetailItemDelegate()
{
}

void NetInfoDetailItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    auto palette = option.palette;
    QBrush background = palette.color(QPalette::Active, QPalette::Base);
    if (!(index.row() & 1)) background = palette.color(QPalette::Active, QPalette::AlternateBase);

    painter->save();
    QPainterPath clipPath;
    clipPath.addRoundedRect(option.widget->rect().adjusted(1, 1, -1, -1), 6, 6);
    painter->setClipPath(clipPath);

    painter->setPen(Qt::NoPen);
    painter->setBrush(background);
    painter->drawRect(option.rect);

    if (index.isValid()) {
        QRect textRect = option.rect;
        textRect.setX(textRect.x() + LEFTMARGIN);

        QPen forground;
        forground.setColor(palette.color(QPalette::Active, QPalette::Text));
        painter->setPen(forground);

        ShowInfo stInfo = index.data(Qt::UserRole).value<ShowInfo>();

        if (index.column() == 1 && stInfo.eType != ShowInfo::Normal) {
            // 绘制第2列IPV
            QStringList listKey;
            QStringList listValue = stInfo.strValue.split("/");

            if (stInfo.eType == ShowInfo::IPV4) {
                listKey << tr("IP address:") << tr("Netmask:") << tr("Broadcast:");
            } else {
                listKey << tr("IP address:") << tr("Prefixlen:") << tr("Scope:");
            }

            if ((listKey.count() == listValue.count()) && (listValue.count() == 3)) {
                // 获取key最宽的数值
                QFontMetrics fm(painter->font());
                int iMaxW = fm.width(listKey[0]);
                for (int i = 1; i < listKey.count(); ++i) {
                    if (iMaxW < fm.width(listKey[i]))
                        iMaxW = fm.width(listKey[i]);
                }

                // 绘制内容
                for (int i = 0; i < listKey.count(); ++i) {
                    // 绘制IPV标题
                    QRect titleRect;
                    titleRect.setX(textRect.x());
                    titleRect.setY(textRect.y() + TOPMARGIN + i * fm.height());
                    titleRect.setWidth(fm.width(listKey[i]));
                    titleRect.setHeight(fm.height());
                    painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, listKey[i]);

                    // 绘制IP
                    QRect valueRect;
                    valueRect.setX(textRect.x() + iMaxW + TEXTSPACING);
                    valueRect.setY(textRect.y() + TOPMARGIN + i * fm.height());
                    valueRect.setWidth(fm.width(listValue[i]));
                    valueRect.setHeight(fm.height());
                    painter->drawText(valueRect, Qt::AlignLeft | Qt::AlignVCenter, listValue[i]);
                }
            }

        } else if (index.column() == 0 && stInfo.eType != ShowInfo::Normal) {
            // 绘制第1列IPV
            textRect.setY(textRect.y() + TOPMARGIN);
            painter->drawText(textRect, Qt::AlignLeft | Qt::AlignTop, stInfo.strKey);
        } else {
            // 其余左对齐、垂直居中
            painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, index.data(Qt::DisplayRole).toString());
        }

    }
    painter->restore();
}

QSize NetInfoDetailItemDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &index) const
{
    const int item_width = 230;

    if (index.isValid()) {
        ShowInfo stInfo = index.data(Qt::UserRole).value<ShowInfo>();

        if (stInfo.eType != ShowInfo::Normal)
            return QSize(item_width, QFontMetrics(m_font).height() * 3 + 2 * TOPMARGIN);

        return QSize(item_width, 36);
    }

    return QSize(item_width, 36);
}


NetDetailsModel::NetDetailsModel(QObject *parent)
    : QAbstractTableModel(parent)
{

}

NetDetailsModel::~NetDetailsModel()
{

}

void NetDetailsModel::onModelUpdated()
{

}

void NetDetailsModel::refreshNetifInfo(const QString &strKey)
{
    std::string str = strKey.toStdString();
    const char *NCName = str.c_str();
    m_listInfo.clear();

    beginResetModel();

    ShowInfo stInfo;

    // 处理IPV4
    QList<INet4Addr> ipv4list = netInfo::getIPv4AddressList(NCName);
    for (int i = 0; i < ipv4list.count(); i++) {
        stInfo.eType = ShowInfo::IPV4;
        if (ipv4list.count() == 1) {
            stInfo.strKey = tr("IPv4");
        } else {
            stInfo.strKey = tr("IPv4") + " " + QString::number(i + 1);
        }
        stInfo.strValue =  QString("%1/%2/%3").arg(ipv4list[i].addr).arg(ipv4list[i].mask).arg(ipv4list[i].bcast);
        m_listInfo << stInfo;
    }

    // 处理IPV6
    QList<INet6Addr> ipv6list = netInfo::getIPv6AddressList(NCName);
    for (int i = 0; i < ipv6list.count(); i++) {
        stInfo.eType = ShowInfo::IPV6;
        if (ipv6list.count() == 1) {
            stInfo.strKey = tr("IPv6");
        } else {
            stInfo.strKey = tr("IPv6") + " " + QString::number(i + 1);
        }
        stInfo.strValue = QString("%1/%2/%3").arg(ipv6list[i].addr).arg(ipv6list[i].prefixlen).arg(ipv6list[i].scope);
        m_listInfo << stInfo;
    }

    // 处理连接类型
    stInfo.eType = ShowInfo::Normal;
    stInfo.strKey = tr("Connection type");
    stInfo.strValue = netInfo::getConnectType(NCName);
    m_listInfo << stInfo;

    // 是否是无线网 是否连接
    if (kdk_nc_is_wireless(NCName) == 1 && netInfo::getUpCards().contains(NCName)) {
        // 服务器别号
        stInfo.strKey = tr("ESSID");
        stInfo.strValue = netInfo::getNetName(NCName);
        m_listInfo << stInfo;

        // 信号质量
        stInfo.strKey = tr("Link quality");
        stInfo.strValue = netInfo::getSignalQuality(NCName);
        m_listInfo << stInfo;

        // 信号强度
        stInfo.strKey = tr("Signal strength");
        stInfo.strValue = netInfo::getSignalStrength(NCName);
        m_listInfo << stInfo;

        // 底噪
        stInfo.strKey = tr("Noise level");
        stInfo.strValue = netInfo::getNoiseLevel(NCName);
        m_listInfo << stInfo;
    }

    // Mac地址
    stInfo.strKey = tr("MAC");
    stInfo.strValue = netInfo::getMacAddress(NCName);
    m_listInfo << stInfo;

    // 速率
    stInfo.strKey = tr("Bandwidth");
    stInfo.strValue = netInfo::getSpeed(NCName);
    m_listInfo << stInfo;

    // 接收包数量
    stInfo.strKey = tr("RX packets");
    stInfo.strValue = netInfo::getRecvPackets(NCName);
    m_listInfo << stInfo;

    // 总计接收
    stInfo.strKey = tr("RX bytes");
    stInfo.strValue = netInfo::getRecvTotal(NCName);
    m_listInfo << stInfo;

    // 接收错误包
    stInfo.strKey = tr("RX errors");
    stInfo.strValue = netInfo::getRecvErrors(NCName);
    m_listInfo << stInfo;

    // 接收丢包数
    stInfo.strKey = tr("RX dropped");
    stInfo.strValue = netInfo::getRecvDropped(NCName);
    m_listInfo << stInfo;

    // 接收FIFO
    stInfo.strKey = tr("RX overruns");
    stInfo.strValue = netInfo::getRecvFIFO(NCName);
    m_listInfo << stInfo;

    // 分组帧错误
    stInfo.strKey = tr("RX frame");
    stInfo.strValue = netInfo::getFrameErrors(NCName);
    m_listInfo << stInfo;

    // 发送包数量
    stInfo.strKey = tr("TX packets");
    stInfo.strValue = netInfo::getSendPackets(NCName);
    m_listInfo << stInfo;

    // 总计发送
    stInfo.strKey = tr("TX bytes");
    stInfo.strValue = netInfo::getSendTotal(NCName);
    m_listInfo << stInfo;

    // 发送错误包
    stInfo.strKey = tr("TX errors");
    stInfo.strValue = netInfo::getSendErrors(NCName);
    m_listInfo << stInfo;

    // 发送丢包数
    stInfo.strKey = tr("TX dropped");
    stInfo.strValue = netInfo::getSendDropped(NCName);
    m_listInfo << stInfo;

    // 发送FIFO
    stInfo.strKey = tr("TX overruns");
    stInfo.strValue = netInfo::getSendFIFO(NCName);
    m_listInfo << stInfo;

    // 载波损耗
    stInfo.strKey = tr("TX carrier");
    stInfo.strValue = netInfo::getCarrierLoss(NCName);
    m_listInfo << stInfo;

    endResetModel();
}
