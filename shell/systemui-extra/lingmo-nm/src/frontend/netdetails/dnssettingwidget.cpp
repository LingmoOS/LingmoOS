#include "dnssettingwidget.h"
#include <QLayout>
#include <QFormLayout>
#include <QApplication>
#include <QGSettings>

#include "coninfo.h"

#define THEME_SCHAME "org.lingmo.style"
#define COLOR_THEME "styleName"

#define  BOTTOM_LAYOUT_MARGINS  24, 16, 24, 24
#define  LAYOUT_SPACING  16

DnsSettingWidget::DnsSettingWidget(QString timeout, QString retry, QString tactic, QWidget *parent)
    :m_timeout(timeout), m_retry(retry), m_tactic(tactic), QDialog(parent)
{
    this->setFixedSize(420, 420);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setProperty("useStyleWindowManager", false);
    initUi();
    initConnect();
    onPaletteChanged();
}

void DnsSettingWidget::initUi()
{
    m_titleWidget = new QWidget(this);
    m_centerWidget = new QWidget(this);
    m_bottomWidget = new QWidget(this);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setContentsMargins(24,14,0,0);
    m_titleLabel->setText(tr("DNS Server Advanced Settings"));

    m_tacticLabel = new QLabel(this);
    m_tacticLabel->setText(tr("Tactic"));

    m_timeoutLabel = new QLabel(this);
    m_timeoutLabel->setText(tr("Timeout"));

    m_retryLabel = new QLabel(this);
    m_retryLabel->setText(tr("Retry Count"));

    m_tacticComboBox = new QComboBox(this);
    m_tacticComboBox->addItem(tr("order"), "order");
    m_tacticComboBox->addItem(tr("rotate"), "rotate");
    m_tacticComboBox->addItem(tr("concurrency"), "concurrency");
    m_tacticComboBox->setCurrentIndex(m_tacticComboBox->findData(m_tactic));

    m_timeoutComboBox = new QComboBox(this);
    for (int i = 0; i < 30; ++i) {
        m_timeoutComboBox->insertItem(i, QString::number(i+1) + tr(" s"), QString::number(i+1));
    }
    m_timeoutComboBox->setCurrentIndex(m_timeoutComboBox->findData(m_timeout.toInt()));

    m_retryComboBox = new QComboBox(this);
    for (int i = 0; i < 4; ++i) {
        m_retryComboBox->insertItem(i, QString::number(i+1) + tr(" times"), QString::number(i+1));
    }
    m_retryComboBox->setCurrentIndex(m_retryComboBox->findData(m_retry.toInt()));

    m_bottomDivider = new Divider(false, this);

    m_closeBtn = new QPushButton(this);
    m_closeBtn->setFixedSize(32,32);
    m_closeBtn->setIcon(QIcon::fromTheme("application-exit-symbolic"));
    m_closeBtn->setProperty("useButtonPalette", true);
    m_closeBtn->setFlat(true);
    m_closeBtn->setToolTip(tr("Close"));

    m_cancelBtn = new QPushButton(this);
    m_cancelBtn->setText(tr("Cancel"));

    m_confirmBtn = new QPushButton(this);
    m_confirmBtn->setText(tr("Confirm"));


    QHBoxLayout* titleLayout = new QHBoxLayout(m_titleWidget);
    titleLayout->setContentsMargins(0,4,3,0);
    titleLayout->addStretch();
    titleLayout->addWidget(m_closeBtn);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_titleWidget);
    mainLayout->addWidget(m_titleLabel, Qt::AlignLeft);
    mainLayout->addWidget(m_centerWidget);
    mainLayout->addSpacing(115);
    mainLayout->addWidget(m_bottomDivider);
    mainLayout->addWidget(m_bottomWidget);
    this->setLayout(mainLayout);

    //中间页面
    QFormLayout* centerLayout = new QFormLayout(m_centerWidget);
    centerLayout->setContentsMargins(24,10,24,0);
    centerLayout->setSpacing(16);
    centerLayout->addRow(m_tacticLabel, m_tacticComboBox);
    centerLayout->addRow(m_timeoutLabel, m_timeoutComboBox);
    centerLayout->addRow(m_retryLabel, m_retryComboBox);

    //底部按钮
    QHBoxLayout* bottomLayout = new QHBoxLayout(m_bottomWidget);
    bottomLayout->setContentsMargins(BOTTOM_LAYOUT_MARGINS);
    bottomLayout->setSpacing(LAYOUT_SPACING);
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_cancelBtn);
    bottomLayout->addWidget(m_confirmBtn);

    this->setWindowFlags(Qt::Dialog);
}

void DnsSettingWidget::initConnect()
{
    connect(m_closeBtn, &QPushButton::released, this, [=](){
        reject();
    });
    connect(m_cancelBtn, &QPushButton::released, this, [=](){
        reject();
    });
    connect(m_confirmBtn, &QPushButton::released, this, [=](){
        m_timeout = m_timeoutComboBox->currentData().toString();
        m_tactic = m_tacticComboBox->currentData().toString();
        m_retry = m_retryComboBox->currentData().toString();
        accept();
    });
    connect(qApp, &QApplication::paletteChanged, this, &DnsSettingWidget::onPaletteChanged);
}

void DnsSettingWidget::onPaletteChanged()
{
    QPalette pal = qApp->palette();

    QGSettings * styleGsettings = nullptr;
    const QByteArray style_id(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(style_id)) {
       styleGsettings = new QGSettings(style_id);
       QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
       if(currentTheme == "lingmo-default"){
           pal = lightPalette(this);
       }
    }
    this->setPalette(pal);
    QList<QComboBox *> comboBoxList = this->findChildren<QComboBox *>();
    for (int i = 0; i < comboBoxList.count(); ++i) {
        comboBoxList.at(i)->setPalette(pal);
    if (comboBoxList.at(i)->view()) {
            comboBoxList.at(i)->view()->setPalette(pal);
        }
    }

    if (styleGsettings != nullptr) {
        delete styleGsettings;
        styleGsettings = nullptr;
    }
}
