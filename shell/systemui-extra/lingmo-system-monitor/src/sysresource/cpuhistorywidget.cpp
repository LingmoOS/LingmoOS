#include "cpuhistorywidget.h"
#include "../macro.h"

#include <QPainterPath>
#include <QPainter>
#include <QPalette>

CpuHistoryWidget::CpuHistoryWidget(QWidget *parent)
    : QWidget(parent)
{
    m_curFontSize = DEFAULT_FONT_SIZE;
    setAttribute(Qt::WA_TranslucentBackground);
    initUI();
    initConnections();
}

CpuHistoryWidget::~CpuHistoryWidget()
{
    if (m_styleSettings) {
        delete m_styleSettings;
        m_styleSettings = nullptr;
    }
}

void CpuHistoryWidget::paintEvent(QPaintEvent *event)
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

void CpuHistoryWidget::initUI()
{
    // 初始化布局
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setContentsMargins(0,0,0,0);
    m_mainLayout->setSpacing(0);
    m_titleLayout = new QHBoxLayout();
    m_titleLayout->setContentsMargins(12,10,12,0);
    m_titleLayout->setSpacing(0);
    m_progLayout = new QHBoxLayout();
    m_progLayout->setContentsMargins(12,0,12,0);
    m_progLayout->setSpacing(0);
    m_chartLayout = new QHBoxLayout();
    m_chartLayout->setContentsMargins(12,8,12,12);
    m_chartLayout->setSpacing(0);

    m_colorBg = QColor("#F5F5F5");

    //初始化控件
    m_labelTitle = new KLabel();
    m_labelTitle->setText(tr("CPU"));
    m_labelTitle->setFixedWidth(80);
    m_labelTitle->setAlignment(Qt::AlignLeft);
    m_titleLayout->addWidget(m_labelTitle, 1, Qt::AlignLeft|Qt::AlignVCenter);
    m_labelValue = new KLabel();
    m_labelValue->setText("0.0%");
    m_labelValue->setFixedWidth(80);

    // 维吾尔 哈萨克斯 柯尔克孜 语言控件翻转
    QString language = QLocale::system().name();
    if (language == "ug_CN" || language == "kk_KZ" || language == "ky_KG") {
        m_labelValue->setAlignment(Qt::AlignLeft);
        m_titleLayout->addWidget(m_labelValue, 1, Qt::AlignLeft|Qt::AlignVCenter);
    } else {
        m_labelValue->setAlignment(Qt::AlignRight);
        m_titleLayout->addWidget(m_labelValue, 1, Qt::AlignRight|Qt::AlignVCenter);
    }


    m_progressBar = new QProgressBar();
    m_progressBar->setTextVisible(false);
    m_progressBar->setMaximum(100);
    m_progressBar->setFixedSize(160, 4);
    m_progLayout->addWidget(m_progressBar);

    m_chartView = new KChartView();
    KChartAttr attr;
    attr.colorForeground = QColor("#3790FA");
    attr.colorAlpha = 154;
    m_chartView->addItemAttr(attr);
    m_chartLayout->addWidget(m_chartView);

    m_mainLayout->addLayout(m_titleLayout);
    m_mainLayout->addLayout(m_progLayout);
    m_mainLayout->addLayout(m_chartLayout);
    m_mainLayout->setSpacing(4);
    this->setLayout(m_mainLayout);
    initStyleTheme();
}

void CpuHistoryWidget::initConnections()
{

}

void CpuHistoryWidget::initStyleTheme()
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

void CpuHistoryWidget::onThemeFontChange(float fFontSize)
{
    QFont fontContext;
    fontContext.setPointSize(fFontSize*0.8);
    QFont fontTitle = this->font();
    if (m_labelValue) {
        m_labelValue->setFont(fontContext);
    }
    if (m_labelTitle) {
        m_labelTitle->setFont(fontTitle);
    }
}

void CpuHistoryWidget::onThemeStyleChange(QString strStyleName)
{
    QPalette controlPE;
    if(strStyleName == "lingmo-dark" || strStyleName == "lingmo-black") {
        controlPE.setColor(QPalette::WindowText,QColor("#999999"));
        m_colorBg = QColor("#262626");
        m_labelTitle->setStyleSheet("QToolTip{border-radius:4px;background-color:rgb(56,56,56,100%);color:white;}");
        m_labelValue->setStyleSheet("QToolTip{border-radius:4px;background-color:rgb(56,56,56,100%);color:white;}");
    } else {
        controlPE.setColor(QPalette::WindowText,QColor("#757575"));
        m_colorBg = QColor("#F5F5F5");
        m_labelTitle->setStyleSheet("QToolTip{border-radius:4px;background-color:rgb(245,245,245,100%);color:black;}");
        m_labelValue->setStyleSheet("QToolTip{border-radius:4px;background-color:rgb(245,245,245,100%);color:black;}");
    }
    if (m_labelValue) {
        m_labelValue->setPalette(controlPE);
    }
    repaint();
}

void CpuHistoryWidget::onUpdateData(qreal cpuPercent)
{
    m_progressBar->setValue(cpuPercent);
    m_chartView->onUpdateData(cpuPercent, 0);
    QString showValue;
    showValue = QString::number(cpuPercent,'f',1)+"%";
    m_labelValue->setText(showValue);
}

QString CpuHistoryWidget::getCpuOccupancy()
{
    return m_labelValue->text();
}

void CpuHistoryWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        Q_EMIT clicked();
    }
    QWidget::mousePressEvent(event);
}
