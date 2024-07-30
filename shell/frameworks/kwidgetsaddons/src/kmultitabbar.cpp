/*
    SPDX-FileCopyrightText: 2001, 2002, 2003 Joseph Wenninger <jowenn@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kmultitabbar.h"
#include "kmultitabbar_p.h"
#include "moc_kmultitabbar.cpp"
#include "moc_kmultitabbar_p.cpp"

#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionButton>

#include <math.h>

/*
 A bit of an overview about what's going on here:
 There are two sorts of things that can be in the tab bar: tabs and regular
 buttons. The former are managed by KMultiTabBarInternal, the later
 by the KMultiTabBar itself.
*/

class KMultiTabBarTabPrivate
{
};
class KMultiTabBarButtonPrivate
{
};

class KMultiTabBarPrivate
{
public:
    class KMultiTabBarInternal *m_internal;
    QBoxLayout *m_l;
    QFrame *m_btnTabSep;
    QList<KMultiTabBarButton *> m_buttons;
    KMultiTabBar::KMultiTabBarPosition m_position;
};

KMultiTabBarInternal::KMultiTabBarInternal(QWidget *parent, KMultiTabBar::KMultiTabBarPosition pos)
    : QFrame(parent)
{
    m_position = pos;
    if (pos == KMultiTabBar::Left || pos == KMultiTabBar::Right) {
        mainLayout = new QVBoxLayout(this);
    } else {
        mainLayout = new QHBoxLayout(this);
    }
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addStretch();
    setFrameStyle(NoFrame);
    setBackgroundRole(QPalette::Window);
}

KMultiTabBarInternal::~KMultiTabBarInternal()
{
    qDeleteAll(m_tabs);
    m_tabs.clear();
}

void KMultiTabBarInternal::setStyle(enum KMultiTabBar::KMultiTabBarStyle style)
{
    m_style = style;
    for (int i = 0; i < m_tabs.count(); i++) {
        m_tabs.at(i)->setStyle(m_style);
    }

    updateGeometry();
}

void KMultiTabBarInternal::contentsMousePressEvent(QMouseEvent *ev)
{
    ev->ignore();
}

void KMultiTabBarInternal::mousePressEvent(QMouseEvent *ev)
{
    ev->ignore();
}

KMultiTabBarTab *KMultiTabBarInternal::tab(int id) const
{
    QListIterator<KMultiTabBarTab *> it(m_tabs);
    while (it.hasNext()) {
        KMultiTabBarTab *tab = it.next();
        if (tab->id() == id) {
            return tab;
        }
    }
    return nullptr;
}

int KMultiTabBarInternal::appendTab(const QIcon &icon, int id, const QString &text)
{
    KMultiTabBarTab *tab;
    m_tabs.append(tab = new KMultiTabBarTab(icon, text, id, this, m_position, m_style));

    // Insert before the stretch..
    mainLayout->insertWidget(m_tabs.size() - 1, tab);
    tab->show();
    return 0;
}

int KMultiTabBarInternal::appendTab(const QPixmap &pic, int id, const QString &text)
{
    // reuse icon variant
    return appendTab(QIcon(pic), id, text);
}

void KMultiTabBarInternal::removeTab(int id)
{
    for (int pos = 0; pos < m_tabs.count(); pos++) {
        if (m_tabs.at(pos)->id() == id) {
            // remove & delete the tab
            delete m_tabs.takeAt(pos);
            break;
        }
    }
}

void KMultiTabBarInternal::setPosition(enum KMultiTabBar::KMultiTabBarPosition pos)
{
    m_position = pos;
    for (int i = 0; i < m_tabs.count(); i++) {
        m_tabs.at(i)->setPosition(m_position);
    }
    updateGeometry();
}

// KMultiTabBarButton
///////////////////////////////////////////////////////////////////////////////

KMultiTabBarButton::KMultiTabBarButton(const QIcon &icon, const QString &text, int id, QWidget *parent)
    : QPushButton(icon, text, parent)
    , m_id(id)
    , d(nullptr)
{
    connect(this, &QPushButton::clicked, this, &KMultiTabBarButton::slotClicked);

    // we can't see the focus, so don't take focus. #45557
    // If keyboard navigation is wanted, then only the bar should take focus,
    // and arrows could change the focused button; but generally, tabbars don't take focus anyway.
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_LayoutUsesWidgetRect);
    Q_UNUSED(d);
}

KMultiTabBarButton::~KMultiTabBarButton()
{
}

void KMultiTabBarButton::setText(const QString &text)
{
    QPushButton::setText(text);
}

void KMultiTabBarButton::slotClicked()
{
    updateGeometry();
    Q_EMIT clicked(m_id);
}

int KMultiTabBarButton::id() const
{
    return m_id;
}

void KMultiTabBarButton::hideEvent(QHideEvent *he)
{
    QPushButton::hideEvent(he);
    KMultiTabBar *tb = dynamic_cast<KMultiTabBar *>(parentWidget());
    if (tb) {
        tb->updateSeparator();
    }
}

void KMultiTabBarButton::showEvent(QShowEvent *he)
{
    QPushButton::showEvent(he);
    KMultiTabBar *tb = dynamic_cast<KMultiTabBar *>(parentWidget());
    if (tb) {
        tb->updateSeparator();
    }
}

void KMultiTabBarButton::paintEvent(QPaintEvent *)
{
    QStyleOptionButton opt;
    opt.initFrom(this);
    opt.icon = icon();
    opt.iconSize = iconSize();
    // removes the QStyleOptionButton::HasMenu ButtonFeature
    opt.features = QStyleOptionButton::Flat;
    QPainter painter(this);
    style()->drawControl(QStyle::CE_PushButton, &opt, &painter, this);
}

// KMultiTabBarTab
///////////////////////////////////////////////////////////////////////////////

KMultiTabBarTab::KMultiTabBarTab(const QIcon &icon,
                                 const QString &text,
                                 int id,
                                 QWidget *parent,
                                 KMultiTabBar::KMultiTabBarPosition pos,
                                 KMultiTabBar::KMultiTabBarStyle style)
    : KMultiTabBarButton(icon, text, id, parent)
    , m_style(style)
    , d(nullptr)
{
    m_position = pos;
    setToolTip(text);
    setCheckable(true);
    // shrink down to icon only, but prefer to show text if it's there
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

KMultiTabBarTab::~KMultiTabBarTab()
{
}

void KMultiTabBarTab::setPosition(KMultiTabBar::KMultiTabBarPosition pos)
{
    m_position = pos;
    updateGeometry();
}

void KMultiTabBarTab::setStyle(KMultiTabBar::KMultiTabBarStyle style)
{
    m_style = style;
    updateGeometry();
}

void KMultiTabBarTab::initStyleOption(QStyleOptionToolButton *opt) const
{
    opt->initFrom(this);

    // Setup icon..
    if (!icon().isNull()) {
        opt->iconSize = iconSize();
        opt->icon = icon();
    }

    // Should we draw text?
    if (shouldDrawText()) {
        opt->text = text();
    }

    opt->state |= QStyle::State_AutoRaise;
    if (!isChecked() && !isDown()) {
        opt->state |= QStyle::State_Raised;
    }
    if (isDown()) {
        opt->state |= QStyle::State_Sunken;
    }
    if (isChecked()) {
        opt->state |= QStyle::State_On;
    }

    opt->font = font();
    opt->toolButtonStyle = shouldDrawText() ? Qt::ToolButtonTextBesideIcon : Qt::ToolButtonIconOnly;
    opt->subControls = QStyle::SC_ToolButton;
}

QSize KMultiTabBarTab::sizeHint() const
{
    return computeSizeHint(shouldDrawText());
}

QSize KMultiTabBarTab::minimumSizeHint() const
{
    return computeSizeHint(false);
}

void KMultiTabBarTab::computeMargins(int *hMargin, int *vMargin) const
{
    // Unfortunately, QStyle does not give us enough information to figure out
    // where to place things, so we try to reverse-engineer it
    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    QSize trialSize = opt.iconSize;
    QSize expandSize = style()->sizeFromContents(QStyle::CT_ToolButton, &opt, trialSize, this);

    *hMargin = (expandSize.width() - trialSize.width()) / 2;
    *vMargin = (expandSize.height() - trialSize.height()) / 2;
}

QSize KMultiTabBarTab::computeSizeHint(bool withText) const
{
    // Compute as horizontal first, then flip around if need be.
    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    int hMargin;
    int vMargin;
    computeMargins(&hMargin, &vMargin);

    // Compute interior size, starting from pixmap..
    QSize size = opt.iconSize;

    // Always include text height in computation, to avoid resizing the minor direction
    // when expanding text..
    QSize textSize = fontMetrics().size(0, text());
    size.setHeight(qMax(size.height(), textSize.height()));

    // Pick margins for major/minor direction, depending on orientation
    int majorMargin = isVertical() ? vMargin : hMargin;
    int minorMargin = isVertical() ? hMargin : vMargin;

    size.setWidth(size.width() + 2 * majorMargin);
    size.setHeight(size.height() + 2 * minorMargin);

    if (withText && !text().isEmpty())
    // Add enough room for the text, and an extra major margin.
    {
        size.setWidth(size.width() + textSize.width() + majorMargin);
    }

    // flip time?
    if (isVertical()) {
        return QSize(size.height(), size.width());
    } else {
        return size;
    }
}

void KMultiTabBarTab::setState(bool newState)
{
    setChecked(newState);
    updateGeometry();
}

bool KMultiTabBarTab::shouldDrawText() const
{
    return (m_style == KMultiTabBar::KDEV3ICON) || isChecked();
}

bool KMultiTabBarTab::isVertical() const
{
    return m_position == KMultiTabBar::Right || m_position == KMultiTabBar::Left;
}

void KMultiTabBarTab::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    // Paint bevel..
    if (underMouse() || isChecked()) {
        opt.text.clear();
        opt.icon = QIcon();
        style()->drawComplexControl(QStyle::CC_ToolButton, &opt, &painter, this);
    }

    int hMargin;
    int vMargin;
    computeMargins(&hMargin, &vMargin);

    // We first figure out how much room we have for the text, based on
    // icon size and margin, try to fit in by eliding, and perhaps
    // give up on drawing the text entirely if we're too short on room
    int textRoom = 0;
    int iconRoom = 0;

    QString t;
    if (shouldDrawText()) {
        if (isVertical()) {
            iconRoom = opt.iconSize.height() + 2 * vMargin;
            textRoom = height() - iconRoom - vMargin;
        } else {
            iconRoom = opt.iconSize.width() + 2 * hMargin;
            textRoom = width() - iconRoom - hMargin;
        }

        t = painter.fontMetrics().elidedText(text(), Qt::ElideRight, textRoom);

        // See whether anything is left. Qt will return either
        // ... or the ellipsis unicode character, 0x2026
        if (t == QLatin1String("...") || t == QChar(0x2026)) {
            t.clear();
        }
    }

    const QIcon::Mode iconMode = (opt.state & QStyle::State_MouseOver) ? QIcon::Active : QIcon::Normal;
    const QPixmap iconPixmap = icon().pixmap(opt.iconSize, devicePixelRatioF(), iconMode, QIcon::On);

    // Label time.... Simple case: no text, so just plop down the icon right in the center
    // We only do this when the button never draws the text, to avoid jumps in icon position
    // when resizing
    if (!shouldDrawText()) {
        style()->drawItemPixmap(&painter, rect(), Qt::AlignCenter | Qt::AlignVCenter, iconPixmap);
        return;
    }

    // Now where the icon/text goes depends on text direction and tab position
    QRect iconArea;
    QRect labelArea;

    bool bottomIcon = false;
    bool rtl = layoutDirection() == Qt::RightToLeft;
    if (isVertical()) {
        if (m_position == KMultiTabBar::Left && !rtl) {
            bottomIcon = true;
        }
        if (m_position == KMultiTabBar::Right && rtl) {
            bottomIcon = true;
        }
    }
    // alignFlags = Qt::AlignLeading | Qt::AlignVCenter;

    const int iconXShift = (isChecked() || isDown()) ? style()->pixelMetric(QStyle::PM_ButtonShiftHorizontal, &opt, this) : 0;
    const int iconYShift = (isChecked() || isDown()) ? style()->pixelMetric(QStyle::PM_ButtonShiftVertical, &opt, this) : 0;
    if (isVertical()) {
        if (bottomIcon) {
            labelArea = QRect(0, vMargin, width(), textRoom);
            iconArea = QRect(0, vMargin + textRoom, width(), iconRoom);
            iconArea.translate(iconYShift, -iconXShift);
        } else {
            labelArea = QRect(0, iconRoom, width(), textRoom);
            iconArea = QRect(0, 0, width(), iconRoom);
            iconArea.translate(-iconYShift, iconXShift);
        }
    } else {
        // Pretty simple --- depends only on RTL/LTR
        if (rtl) {
            labelArea = QRect(hMargin, 0, textRoom, height());
            iconArea = QRect(hMargin + textRoom, 0, iconRoom, height());
        } else {
            labelArea = QRect(iconRoom, 0, textRoom, height());
            iconArea = QRect(0, 0, iconRoom, height());
        }
        iconArea.translate(iconXShift, iconYShift);
    }

    style()->drawItemPixmap(&painter, iconArea, Qt::AlignCenter | Qt::AlignVCenter, iconPixmap);

    if (t.isEmpty()) {
        return;
    }

    QRect labelPaintArea = labelArea;

    if (isVertical()) {
        // If we're vertical, we paint to a simple 0,0 origin rect,
        // and get the transformations to get us in the right place
        labelPaintArea = QRect(0, 0, labelArea.height(), labelArea.width());

        QTransform tr;

        if (bottomIcon) {
            tr.translate(labelArea.x(), labelPaintArea.width() + labelArea.y());
            tr.rotate(-90);
        } else {
            tr.translate(labelPaintArea.height() + labelArea.x(), labelArea.y());
            tr.rotate(90);
        }
        painter.setTransform(tr);
    }

    opt.text = t;
    opt.icon = QIcon();
    opt.rect = labelPaintArea;
    style()->drawControl(QStyle::CE_ToolButtonLabel, &opt, &painter, this);
}

// KMultiTabBar
///////////////////////////////////////////////////////////////////////////////

KMultiTabBar::KMultiTabBar(QWidget *parent)
    : KMultiTabBar(Left, parent)
{
}

KMultiTabBar::KMultiTabBar(KMultiTabBarPosition pos, QWidget *parent)
    : QWidget(parent)
    , d(new KMultiTabBarPrivate)
{
    if (pos == Left || pos == Right) {
        d->m_l = new QVBoxLayout(this);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding /*, true*/);
    } else {
        d->m_l = new QHBoxLayout(this);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed /*, true*/);
    }
    d->m_l->setContentsMargins(0, 0, 0, 0);
    d->m_l->setSpacing(0);

    d->m_internal = new KMultiTabBarInternal(this, pos);
    setPosition(pos);
    setStyle(VSNET);
    d->m_l->insertWidget(0, d->m_internal);
    d->m_l->insertWidget(0, d->m_btnTabSep = new QFrame(this));
    d->m_btnTabSep->setFixedHeight(4);
    d->m_btnTabSep->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    d->m_btnTabSep->setLineWidth(2);
    d->m_btnTabSep->hide();

    updateGeometry();
}

KMultiTabBar::~KMultiTabBar()
{
    qDeleteAll(d->m_buttons);
    d->m_buttons.clear();
}

int KMultiTabBar::appendButton(const QIcon &icon, int id, QMenu *popup, const QString &)
{
    KMultiTabBarButton *btn = new KMultiTabBarButton(icon, QString(), id, this);
    // a button with a QMenu can have another size. Make sure the button has always the same size.
    btn->setFixedWidth(btn->height());
    btn->setMenu(popup);
    d->m_buttons.append(btn);
    d->m_l->insertWidget(0, btn);
    btn->show();
    d->m_btnTabSep->show();
    return 0;
}

void KMultiTabBar::updateSeparator()
{
    bool hideSep = true;
    QListIterator<KMultiTabBarButton *> it(d->m_buttons);
    while (it.hasNext()) {
        if (it.next()->isVisibleTo(this)) {
            hideSep = false;
            break;
        }
    }
    if (hideSep) {
        d->m_btnTabSep->hide();
    } else {
        d->m_btnTabSep->show();
    }
}

int KMultiTabBar::appendTab(const QIcon &icon, int id, const QString &text)
{
    d->m_internal->appendTab(icon, id, text);
    return 0;
}

KMultiTabBarButton *KMultiTabBar::button(int id) const
{
    QListIterator<KMultiTabBarButton *> it(d->m_buttons);
    while (it.hasNext()) {
        KMultiTabBarButton *button = it.next();
        if (button->id() == id) {
            return button;
        }
    }

    return nullptr;
}

KMultiTabBarTab *KMultiTabBar::tab(int id) const
{
    return d->m_internal->tab(id);
}

void KMultiTabBar::removeButton(int id)
{
    for (int pos = 0; pos < d->m_buttons.count(); pos++) {
        if (d->m_buttons.at(pos)->id() == id) {
            d->m_buttons.takeAt(pos)->deleteLater();
            break;
        }
    }

    if (d->m_buttons.isEmpty()) {
        d->m_btnTabSep->hide();
    }
}

void KMultiTabBar::removeTab(int id)
{
    d->m_internal->removeTab(id);
}

void KMultiTabBar::setTab(int id, bool state)
{
    KMultiTabBarTab *ttab = tab(id);
    if (ttab) {
        ttab->setState(state);
    }
}

bool KMultiTabBar::isTabRaised(int id) const
{
    KMultiTabBarTab *ttab = tab(id);
    if (ttab) {
        return ttab->isChecked();
    }

    return false;
}

void KMultiTabBar::setStyle(KMultiTabBarStyle style)
{
    d->m_internal->setStyle(style);
}

KMultiTabBar::KMultiTabBarStyle KMultiTabBar::tabStyle() const
{
    return d->m_internal->m_style;
}

void KMultiTabBar::setPosition(KMultiTabBarPosition pos)
{
    d->m_position = pos;
    d->m_internal->setPosition(pos);
}

KMultiTabBar::KMultiTabBarPosition KMultiTabBar::position() const
{
    return d->m_position;
}

void KMultiTabBar::fontChange(const QFont & /* oldFont */)
{
    updateGeometry();
}

void KMultiTabBar::paintEvent(class QPaintEvent *)
{
    // By default plain QWidget don't call the QStyle and are transparent
    // using drawPrimitive(QStyle::PE_Widget) allow QStyle implementation
    // to theme this class.
    QPainter painter(this);
    QStyleOption opt;
    opt.initFrom(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}
