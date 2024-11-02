#include "nethistorywidget.h"
#include "../macro.h"
#include "../util.h"

#include <QPainterPath>
#include <QPainter>
#include <QPalette>
#include <lingmosdk/lingmosdk-base/kyutils.h>

inline QString formatNetworkBrandWidth(guint64 size, bool isTotal)
{
    enum {
        K_INDEX,
        M_INDEX,
        G_INDEX,
        T_INDEX
    };
    char data[128] = {0};
    sprintf(data,"%lu",size);
    char result_data[128] = {0};
    double m_speed = 0;
    KDKVolumeBaseType m_KDKVolumeBaseType;
    
    QList<guint64> factorList;
    factorList.append(G_GUINT64_CONSTANT(1) << 10);//KiB
    factorList.append(G_GUINT64_CONSTANT(1) << 20);//MiB
    factorList.append(G_GUINT64_CONSTANT(1) << 30);//GiB
    factorList.append(G_GUINT64_CONSTANT(1) << 40);//TiB

    guint64 factor;
    QString format;
    if (size < factorList.at(M_INDEX)) {
        m_KDKVolumeBaseType = KDK_KILOBYTE;
        factor = factorList.at(K_INDEX);
        if (isTotal)
            format = QObject::tr("KiB");
        else
            format = QObject::tr("KiB/s");
    }else if (size < factorList.at(G_INDEX)) {
        m_KDKVolumeBaseType = KDK_MEGABYTE;
        factor = factorList.at(M_INDEX);
        if (isTotal)
            format = QObject::tr("MiB");
        else
            format = QObject::tr("MiB/s");
    } else if (size < factorList.at(T_INDEX)) {
        m_KDKVolumeBaseType = KDK_GIGABYTE;
        factor = factorList.at(G_INDEX);
        if (isTotal)
            format = QObject::tr("GiB");
        else
            format = QObject::tr("GiB/s");
    } else {
        m_KDKVolumeBaseType = KDK_TERABYTE;
        factor = factorList.at(T_INDEX);
        if (isTotal)
            format = QObject::tr("TiB");
        else
            format = QObject::tr("TiB/s");
    }
    
    if (size > 1024 && kdkVolumeBaseCharacterConvert(data, m_KDKVolumeBaseType, result_data) == KDK_NOERR) {
        QString formatted_result = "";
        sscanf(result_data, "%lf", &m_speed);
        formatted_result.setNum(m_speed, 'f', 1);
        return formatted_result + format;
    } else {
        std::string formatted_result(make_string(g_strdup_printf("%.1f", size / (double)factor)));
        return QString::fromStdString(formatted_result) + format;
    }
}

NetHistoryWidget::NetHistoryWidget(QWidget *parent)
    : QWidget(parent)
{
    m_curFontSize = DEFAULT_FONT_SIZE;
    setAttribute(Qt::WA_TranslucentBackground);
    initUI();
    initConnections();
}

NetHistoryWidget::~NetHistoryWidget()
{
    if (m_styleSettings) {
        delete m_styleSettings;
        m_styleSettings = nullptr;
    }
}

void NetHistoryWidget::paintEvent(QPaintEvent *event)
{
    QPainterPath path;

    QPainter painter(this);
    painter.setOpacity(0.4);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setClipping(true);
    painter.setPen(Qt::transparent);

    path.addRoundedRect(this->rect(), 4, 4);
    path.setFillRule(Qt::WindingFill);
    painter.setBrush(QBrush(m_colorBg));
    painter.setPen(Qt::transparent);

    painter.drawPath(path);
    QWidget::paintEvent(event);
}

void NetHistoryWidget::initUI()
{
    // 初始化布局
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setContentsMargins(0,0,0,0);
    m_mainLayout->setSpacing(0);
    m_topTitleLayout = new QHBoxLayout();
    m_topTitleLayout->setContentsMargins(12,10,12,0);
    m_topTitleLayout->setSpacing(0);
    m_midContentLayout = new QVBoxLayout();
    m_midContentLayout->setContentsMargins(12,0,12,0);
    m_midContentLayout->setSpacing(4);
    m_sendLayout = new QHBoxLayout();
    m_sendLayout->setContentsMargins(0,0,12,0);
    m_sendLayout->setSpacing(3);
    m_recvLayout = new QHBoxLayout();
    m_recvLayout->setContentsMargins(0,0,12,0);
    m_recvLayout->setSpacing(3);
    m_chartLayout = new QHBoxLayout();
    m_chartLayout->setContentsMargins(12,12,12,12);
    m_chartLayout->setSpacing(0);

    m_colorBg = QColor("#F5F5F5");

    //初始化控件
    m_labelTitle = new KLabel();
    m_labelTitle->setText(tr("Network History"));
    m_labelTitle->setFixedWidth(160);
    m_labelTitle->setAlignment(Qt::AlignLeft);
    m_topTitleLayout->addWidget(m_labelTitle);

    m_iconSend = new QLabel();
    m_iconSend->setPixmap(QIcon::fromTheme(":/img/network_send.png").pixmap(14,14));
    m_sendLayout->addWidget(m_iconSend, 0, Qt::AlignLeft|Qt::AlignVCenter);
    m_labelSendTitle = new KLabel();
    m_labelSendTitle->setText(tr("Send"));
    m_labelSendTitle->setFixedWidth(60);
    m_labelSendTitle->setAlignment(Qt::AlignLeft);
    m_sendLayout->addWidget(m_labelSendTitle, 1, Qt::AlignLeft|Qt::AlignVCenter);
    m_labelSendValue = new KLabel();
    m_labelSendValue->setText("128MB/s");
    m_labelSendValue->setFixedWidth(80);

    m_iconRecv = new QLabel();
    m_iconRecv->setPixmap(QIcon::fromTheme(":/img/network_recv.png").pixmap(14,14));
    m_recvLayout->addWidget(m_iconRecv, 0, Qt::AlignLeft|Qt::AlignVCenter);
    m_labelRecvTitle = new KLabel();
    m_labelRecvTitle->setText(tr("Receive"));
    m_labelRecvTitle->setFixedWidth(60);
    m_labelRecvTitle->setAlignment(Qt::AlignLeft);
    m_recvLayout->addWidget(m_labelRecvTitle, 1, Qt::AlignLeft|Qt::AlignVCenter);
    m_labelRecvValue = new KLabel();
    m_labelRecvValue->setText("128MB/s");
    m_labelRecvValue->setFixedWidth(80);

    // 维吾尔 哈萨克斯 柯尔克孜 语言控件翻转
    QString language = QLocale::system().name();
    if (language == "ug_CN" || language == "kk_KZ" || language == "ky_KG") {
        m_labelSendValue->setAlignment(Qt::AlignLeft);
        m_sendLayout->addWidget(m_labelSendValue, 1, Qt::AlignLeft|Qt::AlignVCenter);
        m_labelRecvValue->setAlignment(Qt::AlignLeft);
        m_recvLayout->addWidget(m_labelRecvValue, 1, Qt::AlignLeft|Qt::AlignVCenter);
    } else {
        m_labelSendValue->setAlignment(Qt::AlignRight);
        m_sendLayout->addWidget(m_labelSendValue, 1, Qt::AlignRight|Qt::AlignVCenter);
        m_labelRecvValue->setAlignment(Qt::AlignRight);
        m_recvLayout->addWidget(m_labelRecvValue, 1, Qt::AlignRight|Qt::AlignVCenter);
    }

    m_chartView = new KChartView();
    KChartAttr attr;
    attr.colorForeground = QColor("#B37FEB");
    attr.colorAlpha = 154;
    m_chartView->addItemAttr(attr);
    attr.colorForeground = QColor("#FF7875");
    attr.colorAlpha = 154;
    m_chartView->addItemAttr(attr);
    m_chartLayout->addWidget(m_chartView);

    m_mainLayout->addLayout(m_topTitleLayout);
    m_mainLayout->addSpacing(10);
    m_midContentLayout->addLayout(m_recvLayout);
    m_midContentLayout->addLayout(m_sendLayout);
    m_mainLayout->addLayout(m_midContentLayout);
    m_mainLayout->addLayout(m_chartLayout);
    this->setLayout(m_mainLayout);
    initStyleTheme();
}

void NetHistoryWidget::initConnections()
{

}

void NetHistoryWidget::initStyleTheme()
{
    const QByteArray idd(THEME_QT_SCHEMA);
    if(QGSettings::isSchemaInstalled(idd)) {
        m_styleSettings = new QGSettings(idd);
    }
    if (m_styleSettings) {
        //监听主题改变
        connect(m_styleSettings, &QGSettings::changed, this, [=](const QString &key)
        {
            if (key == "styleName") {
                auto variant = m_styleSettings->get("styleName");
                if (variant.isValid()) {
                    onThemeStyleChange(variant.toString());
                }
            } else if ("iconThemeName" == key) {
            } else if("systemFont" == key || "systemFontSize" == key) {
                auto variant = m_styleSettings->get(FONT_SIZE);
                if (variant.isValid()) {
                    m_curFontSize = m_styleSettings->get(FONT_SIZE).toString().toFloat();
                    onThemeFontChange(m_curFontSize);
                }
            }
        });
        auto variant = m_styleSettings->get(FONT_SIZE);
        if (variant.isValid()) {
            m_curFontSize = m_styleSettings->get(FONT_SIZE).toString().toFloat();
            onThemeFontChange(m_curFontSize);
        }
        variant = m_styleSettings->get("styleName");
        if (variant.isValid()) {
            onThemeStyleChange(variant.toString());
        }
    }
}

void NetHistoryWidget::onThemeFontChange(float fFontSize)
{
    QFont fontContext;
    fontContext.setPointSize(fFontSize*0.8);
    QFont fontTitle = this->font();
    if (m_labelSendValue) {
        m_labelSendValue->setFont(fontContext);
    }
    if (m_labelRecvValue) {
        m_labelRecvValue->setFont(fontContext);
    }
    if (m_labelTitle) {
        m_labelTitle->setFont(fontTitle);
    }
    if (m_labelSendTitle) {
        m_labelSendTitle->setFont(fontTitle);
    }
    if (m_labelRecvTitle) {
        m_labelRecvTitle->setFont(fontTitle);
    }
}

void NetHistoryWidget::onThemeStyleChange(QString strStyleName)
{
    QPalette controlPE;
    if(strStyleName == "lingmo-dark" || strStyleName == "lingmo-black") {
        controlPE.setColor(QPalette::WindowText,QColor("#999999"));
        m_colorBg = QColor("#262626");
        m_labelSendTitle->setStyleSheet("QToolTip{border-radius:4px;background-color:rgb(56,56,56,100%);color:white;}");
        m_labelSendValue->setStyleSheet("QToolTip{border-radius:4px;background-color:rgb(56,56,56,100%);color:white;}");
        m_labelRecvTitle->setStyleSheet("QToolTip{border-radius:4px;background-color:rgb(56,56,56,100%);color:white;}");
        m_labelRecvValue->setStyleSheet("QToolTip{border-radius:4px;background-color:rgb(56,56,56,100%);color:white;}");
    } else {
        controlPE.setColor(QPalette::WindowText,QColor("#757575"));
        m_colorBg = QColor("#F5F5F5");
        m_labelSendTitle->setStyleSheet("QToolTip{border-radius:4px;background-color:rgb(245,245,245,100%);color:black;}");
        m_labelSendValue->setStyleSheet("QToolTip{border-radius:4px;background-color:rgb(245,245,245,100%);color:black;}");
        m_labelRecvTitle->setStyleSheet("QToolTip{border-radius:4px;background-color:rgb(245,245,245,100%);color:black;}");
        m_labelRecvValue->setStyleSheet("QToolTip{border-radius:4px;background-color:rgb(245,245,245,100%);color:black;}");
    }
    if (m_labelSendTitle) {
        m_labelSendTitle->setPalette(controlPE);
    }
    if (m_labelSendValue) {
        m_labelSendValue->setPalette(controlPE);
    }
    if (m_labelRecvTitle) {
        m_labelRecvTitle->setPalette(controlPE);
    }
    if (m_labelRecvValue) {
        m_labelRecvValue->setPalette(controlPE);
    }
    repaint();
}

void NetHistoryWidget::onUpdateData(qreal sendTotalBytes, qreal sendRate, qreal recvTotalBytes, qreal recvRate)
{
    Q_UNUSED(sendTotalBytes);
    Q_UNUSED(recvTotalBytes);
    qreal newSendMaxData = m_chartView->getRealMaxData(sendRate, 1);
    qreal newRecvMaxData = m_chartView->getRealMaxData(recvRate, 0);
    qreal newMaxData = chkNewMaxData(newSendMaxData>newRecvMaxData?newSendMaxData:newRecvMaxData);
    m_chartView->setMaxData(newMaxData);
    m_chartView->onUpdateData(sendRate, 1);
    m_chartView->onUpdateData(recvRate, 0);
    const QString uploadRate = formatNetworkBrandWidth(sendRate, false);
    m_labelSendValue->setText(uploadRate);
    const QString downloadRate = formatNetworkBrandWidth(recvRate, false);
    m_labelRecvValue->setText(downloadRate);
}

qreal NetHistoryWidget::chkNewMaxData(qreal qNewMaxData)
{
    qreal newMaxData = 0;
    if (qNewMaxData < 20 * 1024) {
        newMaxData = 20 * 1024;
    } else if (qNewMaxData >= 20 *1024 && qNewMaxData < 100 * 1024) {
        newMaxData = 100 * 1024;
    } else if (qNewMaxData >= 100 *1024 && qNewMaxData < 500 * 1024) {
        newMaxData = 500 * 1024;
    } else if (qNewMaxData >= 500 *1024 && qNewMaxData < 1 * 1024 * 1024) {
        newMaxData = 1 * 1024 * 1024;
    } else if (qNewMaxData >= 1 * 1024 * 1024 && qNewMaxData < 2 * 1024 * 1024) {
        newMaxData = 2 * 1024 * 1024;
    } else if (qNewMaxData >= 2 * 1024 * 1024 && qNewMaxData < 4 * 1024 * 1024) {
        newMaxData = 4 * 1024 * 1024;
    } else if (qNewMaxData >= 4 * 1024 * 1024 && qNewMaxData < 8 * 1024 * 1024) {
        newMaxData = 8 * 1024 * 1024;
    } else if (qNewMaxData >= 8 * 1024 * 1024 && qNewMaxData < 16 * 1024 * 1024) {
        newMaxData = 16 * 1024 * 1024;
    } else if (qNewMaxData >= 16 * 1024 * 1024 && qNewMaxData < 32 * 1024 * 1024) {
        newMaxData = 32 * 1024 * 1024;
    } else if (qNewMaxData >= 32 * 1024 * 1024 && qNewMaxData < 64 * 1024 * 1024) {
        newMaxData = 64 * 1024 * 1024;
    } else if (qNewMaxData >= 64 * 1024 * 1024 && qNewMaxData < 128 * 1024 * 1024) {
        newMaxData = 128 * 1024 * 1024;
    } else if (qNewMaxData >= 128 * 1024 * 1024 && qNewMaxData < 256 * 1024 * 1024) {
        newMaxData = 256 * 1024 * 1024;
    } else if (qNewMaxData >= 256 * 1024 * 1024 && qNewMaxData < 512 * 1024 * 1024) {
        newMaxData = 512 * 1024 * 1024;
    } else if (qNewMaxData >= 512 * 1024 * 1024 && qNewMaxData < 1024 * 1024 * 1024) {
        newMaxData = (quint64)1024 * 1024 * 1024;
    } else {
        newMaxData = (quint64)2 * 1024 * 1024 * 1024;
    }
    return newMaxData;
}

QString NetHistoryWidget::getNetSendSpeed()
{
    return "↑" + m_labelSendValue->text();
}

QString NetHistoryWidget::getNetLoadSpeed()
{
    return "↓" + m_labelRecvValue->text();
}

void NetHistoryWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        Q_EMIT clicked();
    }
    QWidget::mousePressEvent(event);
}
