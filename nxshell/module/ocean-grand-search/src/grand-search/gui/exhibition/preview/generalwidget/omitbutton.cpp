#include "omitbutton.h"

#include <QPainter>
#include <QPainterPath>

#include <DPalette>
#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

OmitButton::OmitButton(QWidget * parent)
    : DPushButton(parent)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setFlat(true);
    m_iconOrig = QIcon(":/icons/omit.svg");
    connect(this, &OmitButton::released, this, [&] {
        m_isPress = false;
        this->setIcon(m_iconOrig);
        this->update();
    });
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &OmitButton::onUpdateSystemTheme);
}

QSize OmitButton::sizeHint() const {
    return this->iconSize();
}

void OmitButton::paintEvent(QPaintEvent* e)
{
    QRectF rect = this->rect();
    QPainter pa(this);
    pa.setRenderHints(QPainter::Antialiasing);
    QColor backgroundColor = QColor(0, 0, 0, 0);
    if (m_isPress) {
        backgroundColor = QColor(0, 0, 0, int(255 * 0.15));
    } else if (m_isHover) {
        backgroundColor = QColor(0, 0, 0, int(255 * 0.1));
    }

    pa.setPen(Qt::NoPen);
    pa.setBrush(QBrush(backgroundColor));
    pa.drawRoundRect(rect, 40, 40);
    pa.drawPixmap(0, 0, this->icon().pixmap(this->iconSize()));
}

void OmitButton::mousePressEvent(QMouseEvent *e)
{
    m_isPress = true;
    DPalette parentPb = DGuiApplicationHelper::instance()->applicationPalette();
    QColor highlightColor = parentPb.color(DPalette::Normal, DPalette::Highlight);
    this->setIcon(this->setIconColor(m_iconOrig, highlightColor));
    update();
    return DPushButton::mousePressEvent(e);
}

void OmitButton::mouseReleaseEvent(QMouseEvent *e)
{
    m_isPress = false;
    this->setIcon(m_iconOrig);
    update();
    return DPushButton::mouseReleaseEvent(e);
}

void OmitButton::enterEvent(QEvent *event)
{
    m_isHover = true;
    update();
    return DPushButton::enterEvent(event);
}

void OmitButton::leaveEvent(QEvent *event)
{
    m_isHover = false;
    update();
    return DPushButton::leaveEvent(event);
}

void OmitButton::showEvent(QShowEvent *event)
{
    this->updateRectSize();
    this->onUpdateSystemTheme();
    return DPushButton::showEvent(event);
}

void OmitButton::updateRectSize()
{
    this->setFixedSize(this->sizeHint());
    this->adjustSize();
}

QIcon OmitButton::setIconColor(QIcon icon, QColor color)
{
    QPixmap pixmap = icon.pixmap(this->iconSize());
    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), color);
    QIcon colorIcon = QIcon(pixmap);
    return colorIcon;
}

void OmitButton::onUpdateSystemTheme() {
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::ColorType::LightType) {
        m_iconOrig = this->setIconColor(m_iconOrig, QColor(0, 0, 0, 255));
        this->setIcon(m_iconOrig);
    } else {
        m_iconOrig = this->setIconColor(m_iconOrig, QColor(255, 255, 255, 255));
        this->setIcon(m_iconOrig);
    }
}
