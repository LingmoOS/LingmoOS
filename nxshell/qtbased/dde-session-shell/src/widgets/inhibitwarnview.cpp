// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inhibitwarnview.h"
#include "framedatabind.h"

#include <QHBoxLayout>
#include <QPushButton>

const int ButtonIconSize = 28;
const int ButtonWidth = 200;
const int ButtonHeight = 64;

InhibitorRow::InhibitorRow(const QString &who, const QString &why, const QIcon &icon, QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout;
    QLabel *whoLabel = new QLabel(who);
    QLabel *whyLabel = new QLabel("-" + why);
    whoLabel->setStyleSheet("color: white; font: bold 12px;");
    whyLabel->setStyleSheet("color: white;");

    layout->addStretch();

    if (!icon.isNull()) {
        QLabel *iconLabel = new QLabel(this);
        QPixmap pixmap = icon.pixmap(topLevelWidget()->windowHandle(), QSize(48, 48));
        iconLabel->setPixmap(pixmap);
        iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        layout->addWidget(iconLabel);
    }

    layout->addWidget(whoLabel);
    layout->addWidget(whyLabel);
    layout->addStretch();
    this->setFixedHeight(ButtonHeight);
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->setLayout(layout);
}

InhibitorRow::~InhibitorRow()
{

}

void InhibitorRow::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 25));
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawRoundedRect(this->rect(), 18, 18);
}

InhibitWarnView::InhibitWarnView(SessionBaseModel::PowerAction inhibitType, QWidget *parent)
    : WarningView(parent)
    , m_inhibitType(inhibitType)
{
    m_acceptBtn = new QPushButton(QString());
    m_acceptBtn->setObjectName("AcceptButton");
    m_acceptBtn->setIconSize(QSize(ButtonIconSize, ButtonIconSize));
    m_acceptBtn->setFixedSize(ButtonWidth, ButtonHeight);
    m_acceptBtn->setCheckable(true);
    m_acceptBtn->setAutoExclusive(true);
    m_acceptBtn->setFocusPolicy(Qt::NoFocus);

    m_cancelBtn = new QPushButton(tr("Cancel"), this);
    m_cancelBtn->setObjectName("CancelButton");
    m_cancelBtn->setIconSize(QSize(ButtonIconSize, ButtonIconSize));
    m_cancelBtn->setFixedSize(ButtonWidth, ButtonHeight);
    m_cancelBtn->setCheckable(true);
    m_cancelBtn->setAutoExclusive(true);
    m_cancelBtn->setFocusPolicy(Qt::NoFocus);

    const auto ratio = devicePixelRatioF();
    QIcon icon_pix = QIcon::fromTheme(":/img/cancel_normal.svg").pixmap(m_cancelBtn->iconSize() * ratio);
    m_cancelBtn->setIcon(icon_pix);

    m_confirmTextLabel = new QLabel;

    m_inhibitorListLayout = new QVBoxLayout;

    std::function<void (QVariant)> buttonChanged = std::bind(&InhibitWarnView::onOtherPageDataChanged, this, std::placeholders::_1);
    m_dataBindIndex = FrameDataBind::Instance()->registerFunction("InhibitWarnView", buttonChanged);

    m_confirmTextLabel->setText("The reason of inhibit.");
    m_confirmTextLabel->setAlignment(Qt::AlignCenter);
    m_confirmTextLabel->setStyleSheet("color:white;");

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addStretch();
    centralLayout->addLayout(m_inhibitorListLayout);
    centralLayout->addSpacing(20);
    centralLayout->addWidget(m_confirmTextLabel);
    centralLayout->addSpacing(20);
    centralLayout->addWidget(m_cancelBtn, 0, Qt::AlignHCenter);
    centralLayout->addSpacing(20);
    centralLayout->addWidget(m_acceptBtn, 0, Qt::AlignHCenter);
    centralLayout->addStretch();

    setLayout(centralLayout);

    m_cancelBtn->setChecked(true);
    m_currentBtn = m_cancelBtn;

    updateIcon();

    connect(m_cancelBtn, &QPushButton::clicked, this, &InhibitWarnView::cancelled);
    connect(m_acceptBtn, &QPushButton::clicked, this, &InhibitWarnView::actionInvoked, Qt::QueuedConnection);
}

InhibitWarnView::~InhibitWarnView()
{
    FrameDataBind::Instance()->unRegisterFunction("InhibitWarnView", m_dataBindIndex);
}

void InhibitWarnView::setInhibitorList(const QList<InhibitorData> &list)
{
    for (QWidget *widget : m_inhibitorPtrList) {
        m_inhibitorListLayout->removeWidget(widget);
        widget->deleteLater();
    }
    m_inhibitorPtrList.clear();

    for (const InhibitorData &inhibitor : list) {
        QIcon icon;

        if (inhibitor.icon.isEmpty() && inhibitor.pid) {
            QFileInfo executable_info(QFile::symLinkTarget(QString("/proc/%1/exe").arg(inhibitor.pid)));

            if (executable_info.exists()) {
                icon = QIcon::fromTheme(executable_info.fileName());
            }
        } else {
            icon = QIcon::fromTheme(inhibitor.icon, QIcon::fromTheme("application-x-desktop"));
        }

        if (icon.isNull()) {
            icon = QIcon::fromTheme("application-x-desktop");
        }

        QWidget *inhibitorWidget = new InhibitorRow(inhibitor.who, inhibitor.why, icon, this);

        m_inhibitorPtrList.append(inhibitorWidget);
        m_inhibitorListLayout->addWidget(inhibitorWidget, 0, Qt::AlignHCenter);
    }
}

void InhibitWarnView::setInhibitConfirmMessage(const QString &text)
{
    m_confirmTextLabel->setText(text);
}

void InhibitWarnView::setAcceptReason(const QString &reason)
{
    m_acceptBtn->setText(reason);
}

void InhibitWarnView::updateIcon()
{
    QString icon_string;
    switch (m_inhibitType) {
    case SessionBaseModel::PowerAction::RequireShutdown:
        icon_string = ":/img/poweroff_warning_normal.svg";
        break;
    case SessionBaseModel::PowerAction::RequireLogout:
        icon_string = ":/img/logout_warning_normal.svg";
        break;
    default:
        icon_string = ":/img/reboot_warning_normal.svg";
        break;
    }

    const auto ratio = devicePixelRatioF();
    QIcon icon_pix = QIcon::fromTheme(icon_string).pixmap(m_acceptBtn->iconSize() * ratio);
    m_acceptBtn->setIcon(icon_pix);
}

void InhibitWarnView::setAcceptVisible(const bool acceptable)
{
    m_acceptBtn->setVisible(acceptable);
}

void InhibitWarnView::toggleButtonState()
{
    if (m_cancelBtn->isChecked() && m_acceptBtn->isVisible())
        setCurrentButton(ButtonType::Accept);
    else
        setCurrentButton(ButtonType::Cancel);

    FrameDataBind::Instance()->updateValue("InhibitWarnView", m_currentBtn->objectName());
}

void InhibitWarnView::buttonClickHandle()
{
    emit m_currentBtn->clicked();
}

SessionBaseModel::PowerAction InhibitWarnView::inhibitType() const
{
    return m_inhibitType;
}

bool InhibitWarnView::focusNextPrevChild(bool next)
{
    if (!next) {
        qWarning() << "focus handling error, nextPrevChild is False";
        return WarningView::focusNextPrevChild(next);
    }

    if (m_acceptBtn->hasFocus() && m_acceptBtn->isVisible())
        setCurrentButton(ButtonType::Cancel);
    else
        setCurrentButton(ButtonType::Accept);

    FrameDataBind::Instance()->updateValue("InhibitWarnView", m_currentBtn->objectName());

    return WarningView::focusNextPrevChild(next);
}

void InhibitWarnView::setCurrentButton(const ButtonType btntype)
{
    switch (btntype) {
    case ButtonType::Cancel:
        m_acceptBtn->setChecked(false);
        m_cancelBtn->setChecked(true);
        m_currentBtn = m_cancelBtn;
        break;

    case ButtonType::Accept:
        m_cancelBtn->setChecked(false);
        m_acceptBtn->setChecked(true);
        m_currentBtn = m_acceptBtn;
        break;
    }
}

void InhibitWarnView::onOtherPageDataChanged(const QVariant &value)
{
    const QString objectName { value.toString() };

    if (objectName == "AcceptButton")
        setCurrentButton(ButtonType::Accept);
    else
        setCurrentButton(ButtonType::Cancel);
}

void InhibitWarnView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Tab:
        toggleButtonState();
        break;
    case Qt::Key_Return:
        m_currentBtn->clicked();
        break;
    case Qt::Key_Enter:
        m_currentBtn->clicked();
        break;
    }
    QWidget::keyPressEvent(event);
}
