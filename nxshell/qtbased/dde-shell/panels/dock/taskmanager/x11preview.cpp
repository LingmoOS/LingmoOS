// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appitem.h"
#include "x11utils.h"
#include "x11preview.h"
#include "abstractwindow.h"
#include "x11windowmonitor.h"

#include <cstdint>
#include <unistd.h>

#include <QFile>
#include <QTimer>
#include <QEvent>
#include <QWindow>
#include <QPixmap>
#include <QLayout>
#include <QScreen>
#include <QPainter>
#include <QByteArray>
#include <QDBusReply>
#include <QMouseEvent>
#include <QDBusInterface>
#include <QLoggingCategory>
#include <QDBusUnixFileDescriptor>
#include <QPainterPath>
#include <DIconButton>

#include <DStyle>
#include <DPlatformHandle>

#include <DWindowManagerHelper>
#include <DGuiApplicationHelper>

Q_LOGGING_CATEGORY(x11WindowPreview, "dde.shell.dock.taskmanager.x11WindowPreview")

#define PREVIEW_TITLE_HEIGHT 24
#define PREVIEW_TITLE_BOTTOMMARGIN 8
#define PREVIEW_CONTENT_HEIGHT 118
#define PREVIEW_CONTENT_MAX_WIDTH 240
#define PREVIEW_CONTENT_MIN_WIDTH 80
#define PREVIEW_CONTENT_MARGIN 10
#define PREVIEW_CONTAINER_MARGIN 10
#define PREVIEW_HOVER_BORDER 4
#define PREVIEW_MINI_WIDTH 140
#define PREVIEW_HOVER_BORDER_COLOR QColor(0, 0, 0, 255 * 0.2)
#define PREVIEW_HOVER_BORDER_COLOR_DARK_MODE QColor(255, 255, 255, 255 * 0.3)
#define PREVIEW_BACKGROUND_COLOR QColor(0, 0, 0, 255 * 0.05)
#define PREVIEW_BACKGROUND_COLOR_DARK_MODE QColor(255, 255, 255, 255 * 0.05)
#define WM_HELPER DWindowManagerHelper::instance()

DGUI_USE_NAMESPACE

namespace dock {
enum {
    WindowIdRole = Qt::UserRole + 1,
    WindowTitleRole,
    WindowIconRole,
    WindowPreviewContentRole,
};

class PreviewsListView : public QListView
{
public:
    using QListView::QListView;

    QSize viewportSizeHint() const override
    {
        QSize size(0, 0);
        int count = model()->rowCount();
        for (int row = 0; row < count; row++) {
            QModelIndex index = model()->index(row, 0);

            QSize s = sizeHintForIndex(index);
            if (flow() == Flow::LeftToRight) {
                size.rwidth() += s.width();
                if (size.height() < s.height()) {
                    size.setHeight(s.height());
                }
            } else {
                size.rheight() += s.height();
                if (size.width() < s.width()) {
                    size.setWidth(s.width());
                }
            }
        }

        if (flow() == Flow::LeftToRight) {
            size.rwidth() += spacing() * count * 2;
            size.rheight() += spacing() * 2;
        } else {
            size.rheight() += spacing() * count * 2;
            size.rwidth() += spacing() * 2;
        }

        return size;
    }
};

class AppItemWindowModel : public QAbstractListModel
{
public:
    AppItemWindowModel(QObject* parent = nullptr) : QAbstractListModel(parent) {}

    int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        return m_item.isNull() ? 0 : m_item->getAppendWindows().size();
    }

    QVariant data(const QModelIndex& index, int role = WindowIdRole) const override
    {
        if (!index.isValid() || index.row() >= rowCount() || m_item.isNull())
            return QVariant();
        
        switch (role) {
            case WindowIdRole: {
                return m_item->getAppendWindows()[index.row()]->id();
            }
            case WindowTitleRole: {
                return m_item->getAppendWindows()[index.row()]->title();
            }
            case WindowIconRole: {
                return m_item->getAppendWindows()[index.row()]->icon();
            }
            case WindowPreviewContentRole: {
                if (index.row() >= m_previewPixmaps.size()) {
                    return fetchWindowPreview(m_item->getAppendWindows()[index.row()]->id());
                } else {
                    return m_previewPixmaps.value(m_item->getAppendWindows()[index.row()]->id());
                }
            }
        }

        return QVariant();
    }

    void setData(const QPointer<AppItem>& item)
    {
        if (!m_item.isNull()) {
            m_item->disconnect(this);
        }

        beginResetModel();
        m_item = item;
        resetPreviewPixmap();
        endResetModel();

        if (!item.isNull()) {
            connect(item, &AppItem::dataChanged, this, [this](){
                beginResetModel();
                resetPreviewPixmap();
                endResetModel();
            });
        }
    }

private:
    void resetPreviewPixmap()
    {
        m_previewPixmaps.clear();
        if (!m_item.isNull()) {
            for (const auto &window : m_item->getAppendWindows()) {
                auto previewPixmap = fetchWindowPreview(window->id());
                m_previewPixmaps.insert(window->id(), previewPixmap);
            }
        }
    }
    QPixmap fetchWindowPreview(const uint32_t& winId) const
    {
        // TODO: check kwin is load screenshot plugin
        if (!WM_HELPER->hasComposite()) return QPixmap();

        // pipe read write fd
        int fd[2];

        if (pipe(fd) < 0) {
            qDebug() << "failed to create pipe";
            return QPixmap();
        }

        QDBusInterface interface(QStringLiteral("org.kde.KWin"), QStringLiteral("/org/kde/KWin/ScreenShot2"), QStringLiteral("org.kde.KWin.ScreenShot2"));
        // 第一个参数，winID或者UUID
        QList<QVariant> args;
        args << QVariant::fromValue(QString::number(winId));
        // 第二个参数，需要截图的选项
        QVariantMap option;
        option["include-decoration"] = true;
        option["include-cursor"] = false;
        option["native-resolution"] = true;
        args << QVariant::fromValue(option);
        // 第三个参数，文件描述符
        args << QVariant::fromValue(QDBusUnixFileDescriptor(fd[1]));

        QDBusReply<QVariantMap> reply = interface.callWithArgumentList(QDBus::Block, QStringLiteral("CaptureWindow"), args);

        // close write
        ::close(fd[1]);

        if (!reply.isValid()) {
            ::close(fd[0]);
            qDebug() << "get current workspace background error: "<< reply.error().message();
            return QPixmap();
        }

        QVariantMap imageInfo = reply.value();
        int imageWidth = imageInfo.value("width").toUInt();
        int imageHeight = imageInfo.value("height").toUInt();
        int imageStride = imageInfo.value("stride").toUInt();
        int imageFormat = imageInfo.value("format").toUInt();

        if (imageWidth <= 1 || imageHeight <= 1) {
            ::close(fd[0]);
            return QPixmap();
        }

        QFile file;
        if (!file.open(fd[0], QIODevice::ReadOnly)) {
            file.close();
            ::close(fd[0]);
            return QPixmap();
        }

        QImage::Format qimageFormat = static_cast<QImage::Format>(imageFormat);
        int bitsCountPerPixel = QImage::toPixelFormat(qimageFormat).bitsPerPixel();

        QByteArray fileContent = file.read(imageHeight * imageWidth * bitsCountPerPixel / 8);
        QImage image(reinterpret_cast<uchar *>(fileContent.data()), imageWidth, imageHeight, imageStride, qimageFormat);
        // close read
        ::close(fd[0]);
        auto pixmap = QPixmap::fromImage(image);

        return pixmap;
    }

private:
    QPointer<AppItem> m_item;
    QHash<uint32_t, QPixmap> m_previewPixmaps;
};

class AppItemWindowDeletegate : public QAbstractItemDelegate
{
private:
    QListView *m_listView;
    X11WindowPreviewContainer* m_parent;

public:
    AppItemWindowDeletegate(QListView *listview, X11WindowPreviewContainer *parent = nullptr) : QAbstractItemDelegate(parent)
    {
        m_listView = listview;
        m_parent = parent;
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        auto themeType = DGuiApplicationHelper::instance()->themeType();

        QRect hoverRect = option.rect;

        QPen pen;
        if (WM_HELPER->hasComposite()) {
            auto pixmap = index.data(WindowPreviewContentRole).value<QPixmap>();
            auto size = calSize(pixmap.size()); 
            auto scaledPixmap = pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            scaledPixmap.setDevicePixelRatio(qApp->devicePixelRatio());

            DStyleHelper dstyle(m_listView->style());
            const int radius = dstyle.pixelMetric(DStyle::PM_FrameRadius);

            painter->save();
            painter->setPen(Qt::NoPen);
            if (themeType == DGuiApplicationHelper::DarkType) {
                painter->setBrush(QColor(255, 255, 255, 0.05 * 255));
            } else {
                painter->setBrush(QColor(0, 0, 0, 0.05 * 255));
            }
            painter->drawRoundedRect(option.rect.marginsRemoved(QMargins(PREVIEW_HOVER_BORDER, PREVIEW_HOVER_BORDER, PREVIEW_HOVER_BORDER, PREVIEW_HOVER_BORDER)), radius, radius);
            painter->restore();


            painter->save();
            painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            pen.setWidth(1);
            pen.setColor(themeType == DGuiApplicationHelper::DarkType ? QColor(255, 255, 255, 255 * 0.1) : QColor(0, 0, 0, 255 * 0.1));
            painter->setPen(pen);
            QRect imageRect(
                (option.rect.left() + ((option.rect.width() - scaledPixmap.width()) / 2)),
                (option.rect.top() + ((option.rect.height() - scaledPixmap.height()) / 2)),
                scaledPixmap.width(),
                scaledPixmap.height());
            QPainterPath clipPath;
            clipPath.addRoundedRect(imageRect, radius, radius);
            painter->setClipPath(clipPath);
            painter->drawPixmap(imageRect, scaledPixmap);
            painter->setClipping(false);
            painter->drawRoundedRect(imageRect, radius, radius);
            if (option.state.testFlag(QStyle::State_MouseOver)) {
                QPainterPath path;
                path.addRoundedRect(option.rect.marginsRemoved(QMargins(2, 2, 2, 2)), radius + 2, radius + 2);
                pen.setWidth(PREVIEW_HOVER_BORDER);
                pen.setColor(themeType == DGuiApplicationHelper::DarkType ? PREVIEW_HOVER_BORDER_COLOR_DARK_MODE : PREVIEW_HOVER_BORDER_COLOR);
                painter->setPen(pen);
                painter->drawPath(path);
            }
            painter->restore();
        } else {
            auto rect = QRect((option.rect.left()),
                                    (option.rect.top()),
                                    PREVIEW_CONTENT_MAX_WIDTH + PREVIEW_HOVER_BORDER * 2,
                                    PREVIEW_TITLE_HEIGHT + PREVIEW_HOVER_BORDER * 2)
                                .marginsAdded(QMargins(-PREVIEW_HOVER_BORDER, -PREVIEW_HOVER_BORDER, -PREVIEW_HOVER_BORDER, -PREVIEW_HOVER_BORDER));
            auto text = QFontMetrics(m_parent->font()).elidedText(index.data(WindowTitleRole).toString(), Qt::TextElideMode::ElideRight, rect.width() - PREVIEW_TITLE_HEIGHT);
            painter->drawText(rect, text);

            if (option.state.testFlag(QStyle::State_MouseOver)) {
                hoverRect.setSize(QSize(PREVIEW_CONTENT_MAX_WIDTH + PREVIEW_HOVER_BORDER * 2, PREVIEW_TITLE_HEIGHT + PREVIEW_HOVER_BORDER * 2));
                hoverRect = hoverRect.marginsAdded(QMargins(-2, -2, -2, -2));

                painter->save();
                painter->setRenderHint(QPainter::Antialiasing);
                painter->setPen(pen);
                painter->drawRect(hoverRect);
                painter->restore();
            }
        }

        if (!option.state.testFlag(QStyle::State_MouseOver)) {
            m_listView->closePersistentEditor(index);
            return;
        }
        m_listView->openPersistentEditor(index);
    }

    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        if (!WM_HELPER->hasComposite()) {
            return QSize(PREVIEW_CONTENT_MAX_WIDTH + PREVIEW_HOVER_BORDER * 2, PREVIEW_TITLE_HEIGHT + PREVIEW_HOVER_BORDER * 2);
        }

        auto pixmap = index.data(WindowPreviewContentRole).value<QPixmap>();
        int width = qBound(PREVIEW_CONTENT_MIN_WIDTH, calSize(pixmap.size()).width(), PREVIEW_CONTENT_MAX_WIDTH);
        return QSize(width, PREVIEW_CONTENT_HEIGHT) + QSize(PREVIEW_HOVER_BORDER * 2, PREVIEW_HOVER_BORDER * 2); 
    }

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        auto closeButton = new DIconButton(parent);
        closeButton->setIcon(DDciIcon::fromTheme("close"));
        closeButton->setEnabledCircle(true);
        QPalette palette = closeButton->palette();

        QColor lightColor = palette.color(QPalette::Normal, QPalette::Light);
        QColor darkColor = palette.color(QPalette::Normal, QPalette::Dark);

        lightColor.setAlpha(255 * 0.6);
        darkColor.setAlpha(255 * 0.6);

        palette.setColor(QPalette::Light,  lightColor);
        palette.setColor(QPalette::Dark,  darkColor);
        closeButton->setPalette(palette);

        closeButton->setIconSize(QSize(16, 16));
        closeButton->setFixedSize(PREVIEW_TITLE_HEIGHT, PREVIEW_TITLE_HEIGHT);
        closeButton->move(option.rect.topRight() - QPoint(PREVIEW_TITLE_HEIGHT + 4, -5));
        closeButton->setVisible(true);

        connect(closeButton, &DToolButton::clicked, this, [this, index](){
            X11Utils::instance()->closeWindow(index.data(WindowIdRole).toInt());
            if (m_listView->model()->rowCount() == 1) {
                m_parent->hide();
            }
        });

        return closeButton;
    }

private:
    QSize calSize(const QSize &imageSize) const
    {
        qreal factor = 1.0f;
        if (imageSize.height() > PREVIEW_CONTENT_HEIGHT) {
            factor = qreal(PREVIEW_CONTENT_HEIGHT) / imageSize.height();
        }
        if (imageSize.width() * factor > PREVIEW_CONTENT_MAX_WIDTH) {
            factor = qreal(PREVIEW_CONTENT_MAX_WIDTH) / imageSize.width();
        }

        return imageSize.scaled(imageSize * factor, Qt::KeepAspectRatio);
    }

};

X11WindowPreviewContainer::X11WindowPreviewContainer(X11WindowMonitor* monitor, QWidget *parent)
    : DBlurEffectWidget(parent)
    , m_direction(0)
    , m_isPreviewEntered(false)
    , m_isDockPreviewCount(0)
    , m_model(new AppItemWindowModel(this))
    , m_titleWidget(new QWidget())
{
    m_hideTimer = new QTimer(this);
    m_hideTimer->setSingleShot(true);
    m_hideTimer->setInterval(500);

    setWindowFlags(Qt::ToolTip | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus | Qt::FramelessWindowHint);
    setMouseTracking(true);
    initUI();

    connect(m_hideTimer, &QTimer::timeout, this, &X11WindowPreviewContainer::callHide);

    connect(m_closeAllButton, &DIconButton::clicked, this, [this](){
        if (m_previewItem.isNull()) return;
        for (auto window : m_previewItem->getAppendWindows()) {
            window->close();
        }
    });

    connect(m_view, &QListView::entered, this, [this](const QModelIndex &enter){
        m_closeAllButton->setVisible(false);
        if (WM_HELPER->hasComposite()) {
            m_monitor->previewWindow(enter.data(WindowIdRole).toInt());
        }

        updatePreviewIconFromBase64(enter.data(WindowIconRole).toString());
        updatePreviewTitle(enter.data(WindowTitleRole).toString());
    });

}

void X11WindowPreviewContainer::showPreview(const QPointer<AppItem> &item, const QPointer<QWindow> &window, int32_t previewXoffset, int32_t previewYoffset, uint32_t direction)
{
    if (!m_previewItem.isNull()) {
        m_previewItem->disconnect(this);
    }

    m_previewItem = item;
    m_baseWindow = window;
    m_previewXoffset = previewXoffset;
    m_previewYoffset = previewYoffset;
    m_direction = direction;

    m_isDockPreviewCount += 1;
    updatePreviewIconFromBase64(item->getCurrentActiveWindowIcon());
    updatePreviewTitle(item->getCurrentActiveWindowName());
    m_model->setData(item);

    updateSize();
    connect(m_previewItem, &AppItem::dataChanged, this, [this](){
        updateSize();
    });

    if (isHidden()) {
        show();
    }
}

void X11WindowPreviewContainer::updateOrientation()
{

    if (m_direction % 2 == 0 && WM_HELPER->hasComposite()) {
        m_view->setFlow(QListView::LeftToRight);
    } else {
        m_view->setFlow(QListView::TopToBottom);
    }

    updateSize();
}

void X11WindowPreviewContainer::callHide()
{
    if (m_isPreviewEntered) return;
    if (m_isDockPreviewCount > 0) return;

    hide();
}

void X11WindowPreviewContainer::hidePreView()
{
    m_isDockPreviewCount -= 1;
    m_isDockPreviewCount = std::max(0, m_isDockPreviewCount);
    m_hideTimer->start();
}

void X11WindowPreviewContainer::enterEvent(QEnterEvent* event)
{
    m_isPreviewEntered = true;
    return DBlurEffectWidget::enterEvent(event);
}

void X11WindowPreviewContainer::leaveEvent(QEvent* event)
{
    m_isPreviewEntered = false;
    m_hideTimer->start();
    m_closeAllButton->setVisible(false);
    return DBlurEffectWidget::leaveEvent(event);
}

void X11WindowPreviewContainer::showEvent(QShowEvent *event)
{
    updateOrientation();
    m_closeAllButton->setVisible(false);
    return DBlurEffectWidget::showEvent(event);
}

void X11WindowPreviewContainer::hideEvent(QHideEvent*)
{
    m_previewItem->disconnect(this);
    m_previewItem = QPointer<AppItem>(nullptr);
    m_model->setData(nullptr);
}

void X11WindowPreviewContainer::resizeEvent(QResizeEvent *event)
{
    updatePosition();
}

void X11WindowPreviewContainer::updatePosition()
{
    auto screenRect = m_baseWindow->screen()->geometry();
    auto dockWindowPosition = m_baseWindow->position();
    int xPosition = dockWindowPosition.x() + m_previewXoffset;
    int yPosition = dockWindowPosition.y() + m_previewYoffset;
    switch(m_direction) {
        case 0: {
            xPosition -= width() / 2;
            break;
        }
        case 1: {
            xPosition -= width();
            yPosition -= height() / 2;
            break;
        }
        case 2: {
            xPosition -= width() / 2;
            yPosition -= height();
            break;
        }
        case 3: {
            yPosition -= height() / 2;
            break;
        }
        default: {
            qCWarning(x11WindowPreview) << QStringLiteral("unknown dock direction");
            break;
        }
    }

    xPosition = std::max(xPosition, screenRect.x() + 10);
    xPosition = std::min(xPosition, screenRect.x() + screenRect.width() - width() - 10);

    yPosition = std::max(yPosition, screenRect.y() + 10);
    yPosition = std::min(yPosition, screenRect.y() + screenRect.height() - height() - 10);

    move(xPosition, yPosition);
}

void X11WindowPreviewContainer::updatePreviewTitle(const QString& title)
{
    m_previewTitleStr = title;
    m_previewTitle->setText(m_previewTitleStr);
}

void X11WindowPreviewContainer::initUI()
{
    m_view = new PreviewsListView(this);
    QVBoxLayout* mainLayout = new QVBoxLayout;
    QHBoxLayout* titleLayout = new QHBoxLayout;
    titleLayout->setContentsMargins(5, 0, 5, 0);
    titleLayout->setSpacing(0);

    m_previewIcon = new QLabel(this);
    m_previewTitle = new DLabel(this);
    m_previewTitle->setFixedHeight(PREVIEW_TITLE_HEIGHT);
    m_previewIcon->setFixedSize(PREVIEW_TITLE_HEIGHT, PREVIEW_TITLE_HEIGHT);
    m_previewIcon->setScaledContents(true);

    m_closeAllButton = new DIconButton(this);

    m_closeAllButton->setIconSize(QSize(16, 16));
    m_closeAllButton->setIcon(DDciIcon::fromTheme("close"));
    m_closeAllButton->setFixedSize(PREVIEW_TITLE_HEIGHT, PREVIEW_TITLE_HEIGHT);
    m_closeAllButton->setEnabledCircle(true);

    DIconButtonHoverFilter *hoverHandler = new DIconButtonHoverFilter(this);
    m_closeAllButton->installEventFilter(hoverHandler);

    m_previewIcon->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_previewTitle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_previewTitle->setElideMode(Qt::ElideRight);

    auto updateWindowTitleColorType = [this](){
        QPalette pa = palette();
        auto type = DGuiApplicationHelper::instance()->themeType();
        pa.setColor(QPalette::WindowText, type == DGuiApplicationHelper::ColorType::LightType ? Qt::black : Qt::white);
        m_previewTitle->setPalette(pa);
    };

    updateWindowTitleColorType();

    connect(DGuiApplicationHelper::instance(), & DGuiApplicationHelper::themeTypeChanged, this , updateWindowTitleColorType);

    titleLayout->addWidget(m_previewIcon);
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(QMargins(PREVIEW_HOVER_BORDER, 0, PREVIEW_HOVER_BORDER, 0));
    titleLayout->addSpacing(6);  
    titleLayout->addWidget(m_previewTitle);
    titleLayout->addStretch();
    titleLayout->addWidget(m_closeAllButton, 0, Qt::AlignRight);

    m_view->setModel(m_model);
    m_view->setItemDelegate(new AppItemWindowDeletegate(m_view, this));
    m_view->setMouseTracking(true);
    m_view->viewport()->installEventFilter(this);
    m_view->setAutoFillBackground(false);
    m_view->setFrameStyle(QFrame::NoFrame);
    m_view->setSpacing(2);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    m_view->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    QPalette pal = m_view->palette();
    pal.setColor(QPalette::Base, Qt::transparent);
    m_view->setPalette(pal);

    m_titleWidget->setLayout(titleLayout);

    mainLayout->addWidget(m_titleWidget, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(PREVIEW_TITLE_BOTTOMMARGIN - PREVIEW_HOVER_BORDER - m_view->spacing());
    mainLayout->addWidget(m_view);
    mainLayout->setAlignment(m_view, Qt::AlignCenter);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(PREVIEW_CONTENT_MARGIN - PREVIEW_HOVER_BORDER, PREVIEW_CONTENT_MARGIN - PREVIEW_HOVER_BORDER / 2,
                                    PREVIEW_CONTENT_MARGIN - PREVIEW_HOVER_BORDER, PREVIEW_CONTENT_MARGIN - PREVIEW_HOVER_BORDER);

    setLayout(mainLayout);

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    winId();
    DPlatformHandle handler(this->windowHandle());
    handler.setShadowRadius(12 * qApp->devicePixelRatio());
    handler.setShadowColor(QColor(0, 0, 0, 0.6 * 255));
    handler.setShadowOffset(QPoint(0, 4 * qApp->devicePixelRatio()));
}

void X11WindowPreviewContainer::updateSize()
{
    if (m_previewItem->getAppendWindows().size() == 0) {
        DBlurEffectWidget::hide();
        return;
    }

    m_view->adjustSize();
    m_view->updateGeometry();

    auto screenSize = screen()->size();

    auto calFixHeight = [=]()-> int {
        int resHeight = screenSize.height();

        bool beyondEdge = m_view->viewportSizeHint().height() + 2 * PREVIEW_CONTENT_MARGIN + PREVIEW_TITLE_HEIGHT > screenSize.height();
        // ListView的高度 + Title下边距 + Container上下边距
        int listviewContainerHeight = m_view->viewportSizeHint().height() + PREVIEW_TITLE_BOTTOMMARGIN - PREVIEW_HOVER_BORDER + 2 * (PREVIEW_CONTENT_MARGIN - PREVIEW_HOVER_BORDER) + PREVIEW_TITLE_HEIGHT;

        if (m_direction % 2 == 0) {
            // 2D模式下Listview纵向排列, 需要去掉任务栏高度, 所以减去 m_baseWindow->height()
            resHeight = beyondEdge ? screenSize.height() - 2 * PREVIEW_CONTENT_MARGIN - m_baseWindow->height() : listviewContainerHeight;
        } else {
            resHeight = beyondEdge ? screenSize.height() - 2 * PREVIEW_CONTENT_MARGIN : listviewContainerHeight;
        }

        return resHeight;
    };

    auto calFixWidth = [=]()-> int {
        int resWidth = screenSize.width();

        bool beyondEdge = m_view->viewportSizeHint().width() + 2 * (PREVIEW_CONTENT_MARGIN + PREVIEW_CONTAINER_MARGIN) > screenSize.width();
        int listviewContainerWidth = m_view->viewportSizeHint().width() + 2 * (PREVIEW_CONTENT_MARGIN - PREVIEW_HOVER_BORDER);

        if (m_direction % 2 == 0) {
            resWidth = beyondEdge ? screenSize.width() - 2 * PREVIEW_CONTENT_MARGIN : listviewContainerWidth;
        } else {
            resWidth = listviewContainerWidth;
        }

        return resWidth;
    };

    setFixedSize(calFixWidth(), calFixHeight());

    if (m_view->width() + this->contentsMargins().left() * 2 <= PREVIEW_MINI_WIDTH) {
        setMaximumWidth(PREVIEW_MINI_WIDTH);
    }

    m_titleWidget->setFixedWidth(m_view->width());
    QTimer::singleShot(0, this, &X11WindowPreviewContainer::adjustSize);
}

bool X11WindowPreviewContainer::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != m_view->viewport()) return false;

    switch (event->type()) {
    case QEvent::HoverLeave: {
        if (WM_HELPER->hasComposite()) {
            m_monitor->cancelPreviewWindow();
        }

        m_closeAllButton->setVisible(true);
        if (m_previewItem.isNull()) return false;
        updatePreviewIconFromBase64(m_previewItem->getCurrentActiveWindowIcon());
        updatePreviewTitle(m_previewItem->getCurrentActiveWindowName());
        break;
    }
    case QEvent::QEvent::MouseButtonRelease: {
        auto mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() != Qt::LeftButton) return false;

        // cancel preview b4 active window
        if (WM_HELPER->hasComposite())
            m_monitor->cancelPreviewWindow();

        auto index = m_view->indexAt(mouseEvent->pos());
        if (index.isValid()) {
            m_previewItem->getAppendWindows()[index.row()]->activate();
        }
        DBlurEffectWidget::hide();
        break;
    }
    default: {}
    }

    return false;
}

void X11WindowPreviewContainer::updatePreviewIconFromBase64(const QString &base64Data)
{
    const QStringList strs = base64Data.split("base64,");
    QPixmap pix;
    if (strs.size() == 2) {
        pix.loadFromData(QByteArray::fromBase64(strs.at(1).toLatin1()));
    }

    if (!pix.isNull()) {
        m_previewIcon->setPixmap(pix);
        return;
    }

    if (m_previewItem) {
        m_previewIcon->setPixmap(QIcon::fromTheme(m_previewItem->icon()).pixmap(PREVIEW_TITLE_HEIGHT, PREVIEW_TITLE_HEIGHT));
    }
}
}
