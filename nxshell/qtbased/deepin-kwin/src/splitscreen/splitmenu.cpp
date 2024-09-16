#include "splitmenu.h"
#include "x11window.h"
#include "waylandwindow.h"
#include "workspace.h"
#include "wayland_server.h"

#include <QLoggingCategory>
#include <QTranslator>
#include <QCoreApplication>
#include <QToolTip>
#include <QEvent>
#include <QPainter>
#include <QtMath>
#include <QScreen>
#include <QSvgRenderer>
#include <QPainterPath>

#define LONG_PRESS_TIME 500
#define HIDE_DELAY_TIME 300

#define LEFT_HOVER      "left_split_hover.svg"
#define LEFT_NORMAL     "left_split_normal.svg"
#define RIGHT_HOVER     "right_split_hover.svg"
#define RIGHT_NORMAL    "right_split_normal.svg"
#define MAX_HOVER       "max_split_hover.svg"
#define MAX_NORMAL      "max_split_normal.svg"
#define RESTORE_HOVER   "restore_split_hover.svg"
#define RESTORE_NORMAL  "restore_split_normal.svg"

namespace KWin
{

Q_LOGGING_CATEGORY(SPLIT_MENU, "kwin.splitmenu", QtCriticalMsg);

SplitMenu *SplitMenu::instance()
{
    static SplitMenu menu;
    return &menu;
}

SplitMenu::SplitMenu()
{
    if (waylandServer()) {
        setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    } else {
        setWindowFlags(Qt::X11BypassWindowManagerHint);
    }
    setAttribute(Qt::WA_TranslucentBackground);

    m_scale = QGuiApplication::primaryScreen()->logicalDotsPerInch() / 96;

    setWindowTitle("splitmenu");

    // shape parameter
    const int label_spacing = 5 * m_scale;
    const QSize label_size(QSize(37, 37) * m_scale);
    const QSize hump_size(QSize(50, 20) * m_scale);
    // shadow parameter
    const QPoint shadow_offset(QPoint(0, 6) * m_scale);
    const QColor shadow_color(0, 0, 0, 0.15 * 255);
    const qreal shadow_blur_radius = 20 * m_scale;

    layout = new QHBoxLayout();
    layout->setSpacing(label_spacing);
    llabel = new QLabel(this);
    clabel = new QLabel(this);
    rlabel = new QLabel(this);

    layout->addWidget(llabel);
    layout->addWidget(clabel);
    layout->addWidget(rlabel);
    llabel->installEventFilter(this);
    clabel->installEventFilter(this);
    rlabel->installEventFilter(this);

    llabel->setFixedSize(label_size);
    clabel->setFixedSize(label_size);
    rlabel->setFixedSize(label_size);

    shadow = new QGraphicsDropShadowEffect(this);
    shadow->setOffset(shadow_offset);
    shadow->setColor(shadow_color);
    shadow->setBlurRadius(shadow_blur_radius);
    this->setGraphicsEffect(shadow);

    // size and relative pos.x of hump and rect are fixed
    m_hump.setSize(hump_size);
    m_hump.moveLeft(shadow->blurRadius() + 4 * layout->spacing() + 1.5 * llabel->width());
    m_rect.setSize(QSize(8 * layout->spacing() + 3 * llabel->width(), 4 * layout->spacing() + llabel->height()));
    m_rect.moveLeft(shadow->blurRadius());
    // window size is fixed
    setFixedSize(m_rect.width() + 2 * shadow->blurRadius(), m_rect.height() + m_hump.height() + shadow->blurRadius());

    QString qm = QString(":/splitmenu/translations/splitmenu_%1.qm").arg(QLocale::system().name());
    auto tran = new QTranslator(this);
    if (tran->load(qm)) {
        qApp->installTranslator(tran);
    } else {
        qCDebug(SPLIT_MENU) << "load " << qm << "failed";
    }

    show_timer.setSingleShot(true);
    connect(&show_timer, &QTimer::timeout,
        [this] {
            m_isShow = true;
            show();
        }
    );
    hide_timer.setSingleShot(true);
    connect(&hide_timer, &QTimer::timeout,
        [this] {
            m_isShow = false;
            m_entered = false;
            m_client = nullptr;
            hide();
        }
    );
}

SplitMenu::~SplitMenu()
{
    if (shadow)
        delete shadow;
}

void SplitMenu::enterEvent(QEvent *event)
{
    m_entered = true;
    QWidget::enterEvent(event);
    stopTime();
}

void SplitMenu::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    Hide(true, true);
}

bool SplitMenu::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == llabel) {
        if (event->type() == QEvent::MouseButtonRelease) {
            if (m_client) {
                m_client->setQuickTileFromMenu(QuickTileFlag::Left);
            }
            llabel->setPixmap(getLabelPixmap(LEFT_NORMAL, llabel->size()));
            Hide(false, true);
        } else if (event->type() == QEvent::Enter) {
            llabel->setPixmap(getLabelPixmap(LEFT_HOVER, llabel->size()));
            QPoint p(m_rect.left() + 2 * layout->spacing(), m_rect.top() + llabel->height());
            QToolTip::showText(p + pos(), tr("Tile window to left of screen"), this);
        } else if (event->type() == QEvent::Leave) {
            llabel->setPixmap(getLabelPixmap(LEFT_NORMAL, llabel->size()));
            QToolTip::hideText();
        }
        return false;
    } else if (obj == clabel) {
        if (event->type() == QEvent::MouseButtonRelease) {
            if (m_client) {
                m_client->setQuickTileFromMenu(QuickTileFlag::Right);
            }
            clabel->setPixmap(getLabelPixmap(RIGHT_NORMAL, clabel->size()));
            Hide(false, true);
        } else if (event->type() == QEvent::Enter) {
            clabel->setPixmap(getLabelPixmap(RIGHT_HOVER, clabel->size()));
            QPoint p(m_rect.left() + 4 * layout->spacing() + llabel->width(), m_rect.top() + clabel->height());
            QToolTip::showText(p + pos(), tr("Tile window to right of screen"), this);
        } else if (event->type() == QEvent::Leave) {
            clabel->setPixmap(getLabelPixmap(RIGHT_NORMAL, clabel->size()));
            QToolTip::hideText();
        }
        return false;
    } else if (obj == rlabel) {
        bool maximized = m_client && m_client->maximizeMode() == MaximizeMode::MaximizeFull;
        if (event->type() == QEvent::MouseButtonRelease) {
            if (m_client) {
                m_client->setQuickTileFromMenu(QuickTileFlag::Maximize, false);
            }
            if (maximized) {
                rlabel->setPixmap(getLabelPixmap(RESTORE_NORMAL, rlabel->size()));
            } else {
                rlabel->setPixmap(getLabelPixmap(MAX_NORMAL, rlabel->size()));
            }
            Hide(false, true);
        } else if (event->type() == QEvent::Enter) {
            QPoint p(m_rect.left() + 6 * layout->spacing() + 2 * llabel->width(), m_rect.top() + clabel->height());
            if (maximized) {
                rlabel->setPixmap(getLabelPixmap(RESTORE_HOVER, rlabel->size()));
                QToolTip::showText(p + pos(), tr("Unmaximize"), this);
            } else {
                rlabel->setPixmap(getLabelPixmap(MAX_HOVER, rlabel->size()));
                QToolTip::showText(p + pos(), tr("Maximize"), this);
            }
        } else if (event->type() == QEvent::Leave) {
            if (maximized) {
                rlabel->setPixmap(getLabelPixmap(RESTORE_NORMAL, rlabel->size()));
            } else {
                rlabel->setPixmap(getLabelPixmap(MAX_NORMAL, rlabel->size()));
            }
            QToolTip::hideText();
        }
        return false;
    }

    return QWidget::eventFilter(obj, event);
}

void SplitMenu::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(m_color));

    // direction of hump depends on m_upside
    double base, sign;
    if (m_upside) {
        base = m_hump.top();
        sign = -1;
    } else {
        base = m_hump.bottom();
        sign = 1;
    }

    // draw hump
    QPainterPath hump_path;
    hump_path.moveTo(m_hump.left(), base);
    const double step = m_hump.width() / 50.0, f = 2 * M_PI / m_hump.width(), half = m_hump.height() / 2.0;
    for (double x = step; x < (double)m_hump.width(); x += step) {
        double y = half * sin(f * x - M_PI / 2) + half;
        hump_path.lineTo(m_hump.left() + x, base - sign * y);
    }
    hump_path.lineTo(m_hump.right(), base);
    painter.drawPath(hump_path);

    // draw rectangle
    QPainterPath rect_path;
    rect_path.addRoundedRect(m_rect, 14 * m_scale, 14 * m_scale);
    painter.drawPath(rect_path);
}

void SplitMenu::Show(const QRect &button_rect, uint32_t client_id)
{
    if (m_isShow)
        return;
    findClient(client_id);
    if (!m_client)
        return;
    stopTime();
    checkTheme();
    checkArea(button_rect);
    show_timer.start(LONG_PRESS_TIME);
}

void SplitMenu::Hide(bool delay, bool internal)
{
    if ((m_entered && !internal) || m_keepShowing)
        return;
    stopTime();
    hide_timer.start((!delay || !m_isShow) ? 0 : HIDE_DELAY_TIME);
}

void SplitMenu::findClient(uint32_t client_id)
{
    if (waylandServer()) {
        m_client = workspace()->findWaylandWindow(client_id);
        if (m_client)
            return;
    }
    m_client = workspace()->findClient(Predicate::WindowMatch, client_id);
}

void SplitMenu::checkTheme()
{
    m_isDark = workspace()->isDarkTheme();
    if (m_isDark) {
        m_color = QColor(0, 0, 0);
    } else {
        m_color = QColor(255, 255, 255);
    }

    llabel->setPixmap(getLabelPixmap(LEFT_NORMAL, llabel->size()));
    clabel->setPixmap(getLabelPixmap(RIGHT_NORMAL, clabel->size()));
    if (m_client && m_client->maximizeMode() == MaximizeMode::MaximizeFull) {
        rlabel->setPixmap(getLabelPixmap(RESTORE_NORMAL, rlabel->size()));
    } else {
        rlabel->setPixmap(getLabelPixmap(MAX_NORMAL, rlabel->size()));
    }
}

void SplitMenu::checkArea(const QRect &button_rect)
{
    QRect area = workspace()->clientArea(MaximizeArea, m_client, button_rect.center()).toRect();

    // align hump with button
    QPoint pos(button_rect.center().x() - m_hump.center().x(), button_rect.bottom() + 1);

    m_upside = false;
    QMargins margins(shadow->blurRadius(), m_hump.height(), shadow->blurRadius(), shadow->blurRadius());
    m_hump.moveTop(0);
    m_rect.moveTop(m_hump.bottom());
    if (pos.x() < area.left()) {
        pos.setX(area.left());
    } else if (pos.x() + width() > area.right()) {
        pos.setX(area.right() - width());
    }
    if (pos.y() < area.top()) {
        pos.setY(area.top());
    } else if (pos.y() + height() > area.bottom()) {
        // inverse menu
        m_upside = true;
        margins = QMargins(shadow->blurRadius(), shadow->blurRadius(), shadow->blurRadius(), m_hump.height());
        m_rect.moveTop(shadow->blurRadius());
        m_hump.moveTop(m_rect.bottom());
        pos.setY(button_rect.top() - height() + 1);
    }

    layout->setContentsMargins(margins);
    setLayout(layout);
    move(pos);
}

QPixmap SplitMenu::getLabelPixmap(const QString &file, const QSize &size)
{
    QString theme = "light";
    if (m_isDark) {
        theme = "dark";
    }

    static QMap<QString, QPixmap> cache;
    const QString key = theme + file + QString("%1%2").arg(size.width()).arg(size.height());
    if (cache.contains(key))
        return cache[key];

    QSvgRenderer svg(QString(":/splitmenu/themes/%1/icons/%2").arg(theme).arg(file));
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHints(QPainter::Antialiasing);
    svg.render(&painter);

    cache.insert(key, pixmap);
    return pixmap;
}

void SplitMenu::stopTime()
{
    show_timer.stop();
    hide_timer.stop();
}

}
