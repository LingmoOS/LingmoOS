#include "custom_color_panel.h"
#include <QDebug>
#include <QDesktopWidget>
#include <QApplication>
#include <QStyle>

QString CustomColorPanel::BLACK = "#000000";
QString CustomColorPanel::WHITE = "#FFFFFF";


CustomColorPanel::CustomColorPanel(QWidget *parent)
    : kdk::KTranslucentFloor(parent)
{
    setEnableBlur(false);
    setShadow(true);
    int radius = qApp->style()->property("maxRadius").toInt();
    setBorderRadius(radius);
    init();
    connect(&LingmoUISettingsMonitor::instance(), &LingmoUISettingsMonitor::styleStatusUpdate,
            this, &CustomColorPanel::handleStyleStatus);
    connect(&LingmoUISettingsMonitor::instance(), &LingmoUISettingsMonitor::tabletModeUpdate,
            this, &CustomColorPanel::handleTabletMode);
    handleStyleStatus(LingmoUISettingsMonitor::instance().styleStatus());
    handleTabletMode(LingmoUISettingsMonitor::instance().tabletMode());
}

void CustomColorPanel::addColor(const QColor& color, bool isClicked)
{
    kdk::KColorButton *btn = new kdk::KColorButton(this);
    btn->setFixedSize(16,16);
    btn->setBackgroundColor(color);
    btn->setButtonType(kdk::KColorButton::CheckedRect);
    if (isClicked == true) {
        setSelectColorButton(btn);
    }
    m_layout->addWidget(btn);
    m_group->addButton(btn);
    m_buttons.append(btn);
    if (isBlackOrWhite(color)) {
        if (m_blackOrWhiteButton != nullptr) {
            qDebug() << "Black-or-white button is set repeatedly!";
        }
        m_blackOrWhiteButton = btn;
    }
    handleTabletMode(LingmoUISettingsMonitor::instance().tabletMode());
}

void CustomColorPanel::init()
{
    m_layout = new QVBoxLayout(this);
    m_group = new QButtonGroup(this);
    connect(m_group, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),[=](QAbstractButton *button){
        kdk::KColorButton* btn =static_cast<kdk::KColorButton*>(button);
        emit currentColorChanged(btn->backgroundColor());
    });
}

void CustomColorPanel::setSelectColorButton(kdk::KColorButton *button)
{
    button->click();
    m_selectButton = button;
}

void CustomColorPanel::setSelectColor(const QColor& color)
{
    if (isBlackOrWhite(color) && m_blackOrWhiteButton) {
        setSelectColorButton(m_blackOrWhiteButton);
        return ;
    }
    for (auto button : m_buttons) {
        if (button->backgroundColor() == color) {
            setSelectColorButton(button);
            return ;
        }
    }
    qDebug() << "Cannot find the button!";
}

void CustomColorPanel::showEvent(QShowEvent *event)
{
    if (parent() != nullptr) {
        QWidget *parentWidget = static_cast<QWidget*>(parent());
        qDebug() << parentWidget->size();
        QPoint size = parentWidget->mapToGlobal(QPoint(0,0));
        QPoint temp = QPoint(
            size.x() + parentWidget->width() - this->width(),
            size.y() + parentWidget->height());

        QDesktopWidget* desktopWidget = QApplication::desktop();
        QRect clientRect = desktopWidget->availableGeometry();

        if(height() + temp.y() < clientRect.height()) {
            this->move(temp);
        }
        else {
            this->move(QPoint(size.x(), size.y() - this->height()));
        }

    }

    kdk::KTranslucentFloor::showEvent(event);
}

void CustomColorPanel::handleStyleStatus(LingmoUISettingsMonitor::StyleStatus status)
{
    switch (status) {
    case LingmoUISettingsMonitor::StyleStatus::LIGHT:
        handleLightMode();
        break;
    case LingmoUISettingsMonitor::StyleStatus::DARK:
        handleDarkMode();
        break;
    default:
        break;
    }
}

void CustomColorPanel::handleDarkMode()
{
    if (m_blackOrWhiteButton) {
        if (m_blackOrWhiteButton == m_selectButton && m_blackOrWhiteButton->backgroundColor().name().toUpper() == BLACK) {
            emit currentColorChanged(m_blackOrWhiteButton->backgroundColor());
        }
        m_blackOrWhiteButton->setBackgroundColor(QColor(WHITE));
    }
}

void CustomColorPanel::handleLightMode()
{
    if (m_blackOrWhiteButton) {
        if (m_blackOrWhiteButton == m_selectButton && m_blackOrWhiteButton->backgroundColor().name().toUpper() == WHITE) {
            emit currentColorChanged(m_blackOrWhiteButton->backgroundColor());
        }
        m_blackOrWhiteButton->setBackgroundColor(QColor(BLACK));
    }
}

bool CustomColorPanel::isBlackOrWhite(const QColor& color)
{
    if (color.name().toUpper() == WHITE || color.name().toUpper() == BLACK) {
        return true;
    }
    return false;
}

void CustomColorPanel::handleTabletMode(LingmoUISettingsMonitor::TabletStatus status)
{
    switch (status) {
    case LingmoUISettingsMonitor::TabletStatus::PCMode:
    case LingmoUISettingsMonitor::TabletStatus::PADHorizontalMode:
    case LingmoUISettingsMonitor::TabletStatus::PADVerticalMode:
        handlePADMode();
        break;
    default:
        break;
    }
}

void CustomColorPanel::handlePADMode()
{
    m_layout->setContentsMargins(16,16,16,16);
    m_layout->setSpacing(16);
    for (auto button : m_buttons) {
        button->setButtonType(kdk::KColorButton::ButtonType::Circle);
    }
}

void CustomColorPanel::handlePCMode()
{
    m_layout->setContentsMargins(8,8,8,8);
    m_layout->setSpacing(8);
    for (auto button : m_buttons) {
        button->setButtonType(kdk::KColorButton::ButtonType::CheckedRect);
    }
}
