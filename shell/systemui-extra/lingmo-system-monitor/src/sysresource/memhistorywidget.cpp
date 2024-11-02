#include "memhistorywidget.h"
#include "../macro.h"
#include "../util.h"

#include <QPainterPath>
#include <QPainter>
#include <QPalette>
#include <lingmosdk/lingmosdk-base/kyutils.h>

inline QString formatMemory(guint64 size)
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
        format = QObject::tr("KiB");
    } else if (size < factorList.at(G_INDEX)) {
        m_KDKVolumeBaseType = KDK_MEGABYTE;
        factor = factorList.at(M_INDEX);
        format = QObject::tr("MiB");
    } else if (size < factorList.at(T_INDEX)) {
        m_KDKVolumeBaseType = KDK_GIGABYTE;
        factor = factorList.at(G_INDEX);
        format = QObject::tr("GiB");
    } else {
        m_KDKVolumeBaseType = KDK_TERABYTE;
        factor = factorList.at(T_INDEX);
        format = QObject::tr("TiB");
    }

    if (size > 1024 && kdkVolumeBaseCharacterConvert(data, m_KDKVolumeBaseType, result_data) == KDK_NOERR){
        QString formatted_result = "";
        sscanf(result_data, "%lf", &m_speed);
        formatted_result.setNum(m_speed, 'f', 1);
        return formatted_result + format;
    } else {
        std::string formatted_result(make_string(g_strdup_printf("%.1f", size / (double)factor)));
        return QString::fromStdString(formatted_result) + format;
    }
}

MemHistoryWidget::MemHistoryWidget(QWidget *parent)
    : QWidget(parent)
{
    m_curFontSize = DEFAULT_FONT_SIZE;
    setAttribute(Qt::WA_TranslucentBackground);
    initUI();
    initConnections();
}

MemHistoryWidget::~MemHistoryWidget()
{
    if (m_styleSettings) {
        delete m_styleSettings;
        m_styleSettings = nullptr;
    }
}

void MemHistoryWidget::paintEvent(QPaintEvent *event)
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

void MemHistoryWidget::initUI()
{
    // 初始化布局
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setContentsMargins(0,0,0,0);
    m_mainLayout->setSpacing(0);
    m_memTitleLayout = new QHBoxLayout();
    m_memTitleLayout->setContentsMargins(12,10,12,0);
    m_memTitleLayout->setSpacing(0);
    m_memProgLayout = new QHBoxLayout();
    m_memProgLayout->setContentsMargins(12,0,12,0);
    m_memProgLayout->setSpacing(0);
    m_swapTitleLayout = new QHBoxLayout();
    m_swapTitleLayout->setContentsMargins(12,10,12,0);
    m_swapTitleLayout->setSpacing(0);
    m_swapProgLayout = new QHBoxLayout();
    m_swapProgLayout->setContentsMargins(12,0,12,0);
    m_swapProgLayout->setSpacing(0);
    m_chartLayout = new QHBoxLayout();
    m_chartLayout->setContentsMargins(12,12,12,12);
    m_chartLayout->setSpacing(0);

    m_colorBg = QColor("#F5F5F5");

    //初始化控件
    m_labelMemTitle = new KLabel();
    m_labelMemTitle->setText(tr("Memory"));
    m_labelMemTitle->setFixedWidth(80);
    m_labelMemTitle->setAlignment(Qt::AlignLeft);
    m_memTitleLayout->addWidget(m_labelMemTitle, 1, Qt::AlignLeft|Qt::AlignVCenter);
    m_labelMemValue = new KLabel();
    m_labelMemValue->setText("0.0KB/0.0KB");
    m_labelMemValue->setFixedWidth(80);

    m_labelSwapTitle = new KLabel();
    m_labelSwapTitle->setText(tr("Swap"));
    m_labelSwapTitle->setFixedWidth(80);
    m_labelSwapTitle->setAlignment(Qt::AlignLeft);
    m_swapTitleLayout->addWidget(m_labelSwapTitle, 1, Qt::AlignLeft|Qt::AlignVCenter);
    m_labelSwapValue = new KLabel();
    m_labelSwapValue->setText(tr("Not enabled"));
    m_labelSwapValue->setFixedWidth(80);


    // 维吾尔 哈萨克斯 柯尔克孜 语言控件翻转
    QString language = QLocale::system().name();
    if (language == "ug_CN" || language == "kk_KZ" || language == "ky_KG") {
        m_labelMemValue->setAlignment(Qt::AlignLeft);
        m_memTitleLayout->addWidget(m_labelMemValue, 1, Qt::AlignLeft|Qt::AlignVCenter);
        m_labelSwapValue->setAlignment(Qt::AlignLeft);
        m_swapTitleLayout->addWidget(m_labelSwapValue, 1, Qt::AlignLeft|Qt::AlignVCenter);
    } else {
        m_labelMemValue->setAlignment(Qt::AlignRight);
        m_memTitleLayout->addWidget(m_labelMemValue, 1, Qt::AlignRight|Qt::AlignVCenter);
        m_labelSwapValue->setAlignment(Qt::AlignRight);
        m_swapTitleLayout->addWidget(m_labelSwapValue, 1, Qt::AlignRight|Qt::AlignVCenter);
    }

    m_progressBarMem = new QProgressBar();
    m_progressBarMem->setTextVisible(false);
    m_progressBarMem->setMaximum(100);
    m_progressBarMem->setFixedSize(160, 4);
    QBrush mem(QColor("#52C429"));
    m_progressBarMem->setProperty("setContentHightColor", mem);
    m_progressBarMem->setProperty("setContentMidLightColor", mem);
    m_memProgLayout->addWidget(m_progressBarMem);

    m_progressBarSwap = new QProgressBar();
    m_progressBarSwap->setTextVisible(false);
    m_progressBarSwap->setMaximum(100);
    m_progressBarSwap->setFixedSize(160, 4);
    QBrush swap(QColor("#F68C27"));
    m_progressBarSwap->setProperty("setContentHightColor", swap);
    m_progressBarSwap->setProperty("setContentMidLightColor", swap);
    m_swapProgLayout->addWidget(m_progressBarSwap);
    m_progressBarSwap->setVisible(false);

    m_chartView = new KChartView();
    KChartAttr attr;
    attr.colorForeground = QColor("#52C429");
    attr.colorAlpha = 154;
    m_chartView->addItemAttr(attr);
    attr.colorForeground = QColor("#F68C27");
    attr.colorAlpha = 154;
    m_chartView->addItemAttr(attr);
    m_chartLayout->addWidget(m_chartView);

    m_mainLayout->addLayout(m_memTitleLayout);
    m_mainLayout->addLayout(m_memProgLayout);
    m_mainLayout->addLayout(m_swapTitleLayout);
    m_mainLayout->addLayout(m_swapProgLayout);
    m_mainLayout->addLayout(m_chartLayout);
    m_mainLayout->setSpacing(4);
    this->setLayout(m_mainLayout);
    initStyleTheme();
}

void MemHistoryWidget::initConnections()
{

}

void MemHistoryWidget::initStyleTheme()
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

void MemHistoryWidget::onThemeFontChange(float fFontSize)
{
    QFont fontContext;
    fontContext.setPointSize(fFontSize*0.8);
    QFont fontTitle = this->font();;
    if (m_labelMemValue) {
        m_labelMemValue->setFont(fontContext);
    }
    if (m_labelSwapValue) {
        m_labelSwapValue->setFont(fontContext);
    }
    if (m_labelMemTitle) {
        m_labelMemTitle->setFont(fontTitle);
    }
    if (m_labelSwapTitle) {
        m_labelSwapTitle->setFont(fontTitle);
    }
}

void MemHistoryWidget::onThemeStyleChange(QString strStyleName)
{
    QPalette controlPE;
    if(strStyleName == "lingmo-dark" || strStyleName == "lingmo-black") {
        controlPE.setColor(QPalette::WindowText,QColor("#999999"));
        m_colorBg = QColor("#262626");
        m_labelMemTitle->setStyleSheet("QToolTip{border-radius:4px;background-color:rgb(56,56,56,100%);color:white;}");
        m_labelMemValue->setStyleSheet("QToolTip{border-radius:4px;background-color:rgb(56,56,56,100%);color:white;}");
        m_labelSwapTitle->setStyleSheet("QToolTip{border-radius:4px;background-color:rgb(56,56,56,100%);color:white;}");
        m_labelSwapValue->setStyleSheet("QToolTip{border-radius:4px;background-color:rgb(56,56,56,100%);color:white;}");
    } else {
        controlPE.setColor(QPalette::WindowText,QColor("#757575"));
        m_colorBg = QColor("#F5F5F5");
        m_labelMemTitle->setStyleSheet("QToolTip{border-radius:4px;background-color:rgb(245,245,245,100%);color:black;}");
        m_labelMemValue->setStyleSheet("QToolTip{border-radius:4px;background-color:rgb(245,245,245,100%);color:black;}");
        m_labelSwapTitle->setStyleSheet("QToolTip{border-radius:4px;background-color:rgb(245,245,245,100%);color:black;}");
        m_labelSwapValue->setStyleSheet("QToolTip{border-radius:4px;background-color:rgb(245,245,245,100%);color:black;}");
    }
    if (m_labelMemValue) {
        m_labelMemValue->setPalette(controlPE);
    }
    if (m_labelSwapValue) {
        m_labelSwapValue->setPalette(controlPE);
    }
    repaint();
}

void MemHistoryWidget::onUpdateMemData(qreal memUsed, qreal memTotal)
{
    qreal memPercent = (memTotal ? memUsed  / memTotal : 0.0f) * 100.0;
    m_progressBarMem->setValue(memPercent);
    m_chartView->onUpdateData(memPercent, 0);
    const QString infoMemory = QString("%1/%2").arg(formatMemory(memUsed)).arg(formatMemory(memTotal));
    m_labelMemValue->setText(infoMemory);
}

void MemHistoryWidget::onUpdateSwapData(qreal swapUsed, qreal swapTotal)
{
    qreal swapPercent = (swapTotal ? swapUsed  / swapTotal : 0.0f) * 100.0;
    m_progressBarSwap->setValue(swapPercent);
    m_chartView->onUpdateData(swapPercent, 1);
    const QString infoMemory = QString("%1/%2").arg(formatMemory(swapUsed)).arg(formatMemory(swapTotal));
    if (qAbs(swapTotal) < 1e-15) {
        m_labelSwapValue->setText(tr("Not enabled"));
        if (m_progressBarSwap->isVisible()) {
            m_progressBarSwap->setVisible(false);
        }
    } else {
        m_labelSwapValue->setText(infoMemory);
        if (!m_progressBarSwap->isVisible()) {
            m_progressBarSwap->setVisible(true);
        }
    }
    return;
}

QString MemHistoryWidget::getMemOccupancy()
{
    return QString::number(m_progressBarMem->value(), 'f', 1) + "%";
}

void MemHistoryWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        Q_EMIT clicked();
    }
    QWidget::mousePressEvent(event);
}
