// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "itemwidget.h"
#include "constants.h"
#include "pixmaplabel.h"
#include "refreshtimer.h"

#include <QPainter>
#include <QPainterPath>
#include <QTextOption>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QApplication>
#include <QMouseEvent>
#include <QDir>
#include <QBitmap>
#include <QImageReader>
#include <QIcon>
#include <QScopedPointer>
#include <QFile>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QMimeDatabase>
#include <QGraphicsOpacityEffect>

#include <DFontSizeManager>

#include "dgiofile.h"
#include "dgiofileinfo.h"

#include <cmath>

#define HoverAlpha 200
#define UnHoverAlpha 80

/*!
 * \~chinese \class ItemWidget
 * \~chinese \brief 负责剪贴块数据的展示。
 * \~chinese 传入ItemData类型的数据，在initData之后，将数据分别设置相应的控件进行展示
 */
ItemWidget::ItemWidget(QPointer<ItemData> data, QWidget *parent)
    : DWidget(parent)
    , m_data(data)
    , m_nameLabel(new DLabel(this))
    , m_timeLabel(new DLabel(this))
    , m_closeButton(new IconButton(this))
    , m_contentLabel(new PixmapLabel(data,this))
    , m_statusLabel(new DLabel(this))
    , m_refreshTimer(new QTimer(this))
{
    initUI();
    initData(m_data);
    initConnect();

    qApp->installEventFilter(this);

    m_refreshTimer->start();
}

const QString &ItemWidget::text()
{
    return m_data->text();
}

void ItemWidget::setTextShown(const QString &length)
{
    QFont font = m_contentLabel->font();

    font.setItalic(true);

    m_contentLabel->setFont(font);

    m_contentLabel->setText(true);

    m_statusLabel->setText(length);
}

void ItemWidget::setThumnail(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
    m_data->setPixmap(pixmap);
    if (!m_pixmap.isNull()) {
        QPixmap pix = Globals::pixmapScaled(pixmap);//先缩放,再设置圆角,保证缩略图边框宽度在显示后不会变化
        m_contentLabel->setPixmap(Globals::GetRoundPixmap(pix, palette().color(QPalette::Base)));
        if (m_data->type() == Image) {
            m_statusLabel->setText(QString("%1X%2px").arg(m_data->pixSize().width()).arg(m_data->pixSize().height()));
        }
    }
}

void ItemWidget::setThumnail(const FileIconData &data)
{
    QPixmap pix = data.fileIcon.pixmap(QSize(FileIconWidth, FileIconWidth));
    setThumnail(pix);
}

void ItemWidget::setFileIcon(const QPixmap &pixmap)
{
    if (pixmap.size().isNull())
        return;
    QPixmap pix = Globals::pixmapScaled(pixmap);//如果需要加边框,先缩放再加边框
    m_data->saveFileIcons(QList<QPixmap>() << pix);
    m_contentLabel->setPixmap(pix);
}

void ItemWidget::setFileIcon(const FileIconData &data/*, bool setRadius*/)
{
    QPixmap pix = GetFileIcon(data);
    setFileIcon(pix);
}

void ItemWidget::setFileIcons(const QList<QPixmap> &list)
{
    m_data->saveFileIcons(list);

    m_contentLabel->setPixmapList(list);
}

void ItemWidget::setClipType(const QString &text)
{
    m_nameLabel->setText(text);
}

void ItemWidget::setCreateTime(const QDateTime &time)
{
    m_createTime = time;
    onRefreshTime();
}

void ItemWidget::setAlpha(int alpha)
{
    m_hoverAlpha = alpha;
    m_unHoverAlpha = alpha;

    update();
}

int ItemWidget::hoverAlpha() const
{
    return m_hoverAlpha;
}

void ItemWidget::setHoverAlpha(int alpha)
{
    m_hoverAlpha = alpha;

    update();
}

void ItemWidget::setOpacity(double opacity)
{
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect;
    setGraphicsEffect(effect);
    effect->setOpacity(opacity);
    effect->destroyed();
}

const QPointer<ItemData> ItemWidget::itemData()
{
    return m_data;
}

int ItemWidget::unHoverAlpha() const
{
    return m_unHoverAlpha;
}

void ItemWidget::setUnHoverAlpha(int alpha)
{
    m_unHoverAlpha = alpha;

    update();
}

void ItemWidget::setRadius(int radius)
{
    m_radius = radius;

    update();
}

void ItemWidget::onHoverStateChanged(bool hover)
{
    m_havor = hover;

    if (hover) {
        m_timeLabel->hide();
        m_closeButton->show();
    } else {
        m_timeLabel->show();
        m_closeButton->hide();
    }

    update();
}

void ItemWidget::onRefreshTime()
{
    m_timeLabel->setText(CreateTimeString(m_createTime));
}

void ItemWidget::onClose()
{
    if (m_destroy == false) {
        QParallelAnimationGroup *group = new QParallelAnimationGroup(this);

        QPropertyAnimation *geoAni = new QPropertyAnimation(this, "geometry", group);
        geoAni->setStartValue(geometry());
        geoAni->setEndValue(QRect(geometry().center(), geometry().center()));
        geoAni->setDuration(AnimationTime);

        QPropertyAnimation *opacityAni = new QPropertyAnimation(this, "opacity", group);
        opacityAni->setStartValue(1.0);
        opacityAni->setEndValue(0.0);
        opacityAni->setDuration(AnimationTime);

        group->addAnimation(geoAni);
        group->addAnimation(opacityAni);

        m_data->remove();

        group->start(QAbstractAnimation::DeleteWhenStopped);
        m_destroy = true;
    } else {
        return;
    }
}

void ItemWidget::initUI()
{
    //标题区域
    QWidget *titleWidget = new QWidget;
    QHBoxLayout *titleLayout = new QHBoxLayout(titleWidget);
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(10, 0, 10, 0);
    titleLayout->addWidget(m_nameLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(m_timeLabel);
    titleLayout->addWidget(m_closeButton);

    titleWidget->setFixedHeight(ItemTitleHeight);

    QFont font = DFontSizeManager::instance()->t4();
    font.setWeight(75);
    m_nameLabel->setFont(font);
    m_nameLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_timeLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);

    m_closeButton->setFixedSize(QSize(ItemTitleHeight, ItemTitleHeight) * 2 / 3);
    m_closeButton->setRadius(ItemTitleHeight);
    m_closeButton->setVisible(false);

    m_refreshTimer->setInterval(60 * 1000);

    //布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    mainLayout->addWidget(titleWidget, 0, Qt::AlignTop);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(ContentMargin, 0, ContentMargin, 0);
    layout->addWidget(m_contentLabel, 0, Qt::AlignCenter);
    mainLayout->addLayout(layout, 0);
    mainLayout->addWidget(m_statusLabel, 0, Qt::AlignBottom);

    m_contentLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setFixedHeight(ItemStatusBarHeight);
    m_statusLabel->setAlignment(Qt::AlignCenter);

    setHoverAlpha(HoverAlpha);
    setUnHoverAlpha(UnHoverAlpha);
    setRadius(8);

    setFocusPolicy(Qt::StrongFocus);

    setMouseTracking(true);
    m_contentLabel->setMouseTracking(true);
    m_nameLabel->setMouseTracking(true);
    m_timeLabel->setMouseTracking(true);
    m_statusLabel->setMouseTracking(true);
}

void ItemWidget::initData(QPointer<ItemData> data)
{
    setClipType(data->title());
    setCreateTime(data->time());
    switch (data->type()) {
    case Text: {
        setTextShown(data->subTitle());
    }
    break;
    case Image: {
        m_contentLabel->setAlignment(Qt::AlignCenter);
        setThumnail(data->pixmap());
    }
    break;
    case File: {
        if (data->urls().size() == 0) {
            qDebug() << "error";
            return;
        }

        QUrl url = data->urls().first();
        if (data->urls().size() == 1) {
            if (!m_data->pixmap().isNull()) {//避免重复获取
                setThumnail(m_data->pixmap());
            } else if (m_data->FileIcons().size() == 1) { //避免重复获取
                setFileIcon(m_data->FileIcons().first());
            } else {
                if (data->IconDataList().size() == data->urls().size()) {//先查看文件管理器在复制时有没有提供缩略图
                    FileIconData iconData = data->IconDataList().first();
                    //图片不需要加角标,但需要对图片进行圆角处理(此时文件可能已经被删除，缩略图由文件管理器提供)
                    if (QImageReader::supportedImageFormats().contains(QFileInfo(url.path()).suffix().toLatin1())) {
                        //只有图片需要圆角边框
                        iconData.cornerIconList.clear();
                        setThumnail(iconData);
                    } else {
                        // 图标为空时，通过url获取图标
                        if (iconData.fileIcon.isNull()) {
                            QPixmap pix = GetFileIcon(url.path());
                            setFileIcon(pix);
                        } else {
                            setFileIcon(iconData);
                        }
                    }
                } else {
                    QMimeDatabase db;
                    QMimeType mime = db.mimeTypeForFile(url.path());
                    if (mime.name().startsWith("image/")) { //如果文件是图片,提供缩略图
                        QFile file(url.path());
                        QPixmap pix;
                        if (file.open(QFile::ReadOnly)) {
                            QByteArray array = file.readAll();
                            pix.loadFromData(array);

                            if (pix.isNull()) {
                                QIcon icon = QIcon::fromTheme(mime.genericIconName());
                                pix = icon.pixmap(PixmapWidth, PixmapHeight);
                                setFileIcon(pix);
                            } else {
                                setThumnail(pix);
                            }
                        } else {
                            QIcon icon = QIcon::fromTheme(mime.genericIconName());
                            pix = icon.pixmap(PixmapWidth, PixmapHeight);
                            setFileIcon(pix);
                        }
                        file.close();
                    } else {
                        QPixmap pix = GetFileIcon(url.path());
                        setFileIcon(pix);
                    }
                }
            }

            QFontMetrics metrix = m_statusLabel->fontMetrics();
            QString text = metrix.elidedText(url.fileName(), Qt::ElideMiddle, WindowWidth - 2 * ItemMargin - 10, 0);
            m_statusLabel->setText(text);

        } else if (data->urls().size() > 1) {
            QFontMetrics metrix = m_statusLabel->fontMetrics();
            QString text = metrix.elidedText(tr("%1 files (%2...)").arg(data->urls().size()).arg(url.fileName()),
                                             Qt::ElideMiddle, WindowWidth - 2 * ItemMargin - 10, 0);
            m_statusLabel->setText(text);

            bool getByUrl = true;
            //判断文件管理器是否提供,提供不全或图标为空时，通过url获取图标
            if (data->IconDataList().size() == data->urls().size()) {
                QList<QPixmap> pixmapList;
                foreach (auto iconData, data->IconDataList()) {
                    if (iconData.fileIcon.isNull()) {
                        break;
                    }
                    QPixmap pix = GetFileIcon(iconData);
                    pixmapList.push_back(pix);
                    if (pixmapList.size() == 3) {
                        getByUrl = false;
                        break;
                    }
                }
                if (!getByUrl) {
                    std::sort(pixmapList.begin(), pixmapList.end(), [ = ](const QPixmap & pix1, const QPixmap & pix2) {
                        return pix1.size().width() < pix2.size().width();
                    });
                    setFileIcons(pixmapList);
                }
            }
            if (getByUrl) {
                if (!m_data->FileIcons().isEmpty()) {//避免重复获取
                    setFileIcons(m_data->FileIcons());
                    break;
                }

                int iconNum = MIN(3, data->urls().size());
                QList<QPixmap> pixmapList;
                for (int i = 0; i < iconNum; ++i) {
                    QUrl fileUrl = data->urls()[i];
                    QPixmap pix = GetFileIcon(fileUrl.toLocalFile());
                    pixmapList.push_back(pix);
                }
                setFileIcons(pixmapList);
            }
        }
    }
    break;
    default:
        break;
    }

    if (!data->dataEnabled()) {
        m_contentLabel->setEnabled(false);
        QFontMetrics metrix = m_statusLabel->fontMetrics();
        QString tips = tr("(File deleted)");
        int tipsWidth = metrix.horizontalAdvance(tips);
        QString text = metrix.elidedText(m_statusLabel->text(), Qt::ElideMiddle, WindowWidth - 2 * ItemMargin - 10 - tipsWidth, 0);
        m_statusLabel->setText(text + tips);
    }
}

void ItemWidget::initConnect()
{
    connect(m_refreshTimer, &QTimer::timeout, this, &ItemWidget::onRefreshTime);
    connect(RefreshTimer::instance(), &RefreshTimer::forceRefresh, this, &ItemWidget::onRefreshTime);
    connect(this, &ItemWidget::hoverStateChanged, this, &ItemWidget::onHoverStateChanged);
    connect(m_closeButton, &IconButton::clicked, this, &ItemWidget::onClose);
    connect(this, &ItemWidget::closeHasFocus, m_closeButton, &IconButton::setFocusState);
}

QString ItemWidget::CreateTimeString(const QDateTime &time)
{
    QString text;

    QDateTime t = QDateTime::currentDateTime();

    if (time.daysTo(t) >= 1 && time.daysTo(t) < 2) { //昨天发生的
        text = tr("Yesterday") + time.toString(" hh:mm");
    } else if (time.daysTo(t) >= 2 && time.daysTo(t) < 7) { //昨天以前，一周以内
        text = time.toString("ddd hh:mm");
    } else if (time.daysTo(t) >= 7) { //一周前以前的
        text = time.toString("yyyy/MM/dd");
    } else if (time.secsTo(t) < 60 && time.secsTo(t) >= 0) { //60秒以内
        text = tr("Just now");
    } else if (time.secsTo(t) >= 60 && time.secsTo(t) < 2 * 60) { //一分钟
        text = tr("1 minute ago");
    } else if (time.secsTo(t) >= 2 * 60 && time.secsTo(t) < 60 * 60) { //多少分钟前
        text = tr("%1 minutes ago").arg(time.secsTo(t) / 60);
    } else if (time.secsTo(t) >= 60 * 60 && time.secsTo(t) < 2 * 60 * 60) {//一小时前
        text = tr("1 hour ago");
    } else if (time.secsTo(t) >= 2 * 60 * 60 && time.daysTo(t) < 1) { //多少小时前(0点以后)
        text = tr("%1 hours ago").arg(time.secsTo(t) / 60 / 60);
    }

    return text;
}

QList<QRectF> ItemWidget::getCornerGeometryList(const QRectF &baseRect, const QSizeF &cornerSize)
{
    QList<QRectF> list;
    int offset = int(baseRect.width() / 8);
    const QSizeF &offset_size = cornerSize / 2;

    QRectF r1 = QRectF(QPointF(std::floor(baseRect.right() - offset - offset_size.width()),
                                  std::floor(baseRect.bottom() - offset - offset_size.height())), cornerSize);
    QRectF r2 = QRectF(QPointF(std::floor(baseRect.left() + offset - offset_size.width()), std::floor(r1.top())), cornerSize);
    QRectF r3 = QRectF(QPointF(std::floor(r2.left()), std::floor(baseRect.top() + offset - offset_size.height())), cornerSize);
    QRectF r4 = QRectF(QPointF(std::floor(r1.left()), std::floor(r3.top())), cornerSize);

    list << r1;
    list << r2;
    list << r3;
    list << r4;

    return list;
}

QPixmap ItemWidget::getIconPixmap(const QIcon &icon, const QSize &size, qreal pixelRatio, QIcon::Mode mode, QIcon::State state)
{
    // ###(zccrs): 开启Qt::AA_UseHighDpiPixmaps后，QIcon::pixmap会自动执行 pixmapSize *= qApp->devicePixelRatio()
    //             而且，在有些QIconEngine的实现中，会去调用另一个QIcon::pixmap，导致 pixmapSize 在这种嵌套调用中越来越大
    //             最终会获取到一个是期望大小几倍的图片，由于图片太大，会很快将 QPixmapCache 塞满，导致后面再调用QIcon::pixmap
    //             读取新的图片时无法缓存，非常影响图片绘制性能。此处在获取图片前禁用 Qt::AA_UseHighDpiPixmaps，自行处理图片大小问题
    bool useHighDpiPixmaps = qApp->testAttribute(Qt::AA_UseHighDpiPixmaps);
    qApp->setAttribute(Qt::AA_UseHighDpiPixmaps, false);

    QSize icon_size = icon.actualSize(size, mode, state);

    if (icon_size.width() > size.width() || icon_size.height() > size.height())
        icon_size.scale(size, Qt::KeepAspectRatio);

    QSize pixmapSize = icon_size * pixelRatio;
    QPixmap px = icon.pixmap(pixmapSize, mode, state);

    // restore the value
    qApp->setAttribute(Qt::AA_UseHighDpiPixmaps, useHighDpiPixmaps);

    if (px.width() > icon_size.width() * pixelRatio) {
        px.setDevicePixelRatio(px.width() * 1.0 / qreal(icon_size.width()));
    } else if (px.height() > icon_size.height() * pixelRatio) {
        px.setDevicePixelRatio(px.height() * 1.0 / qreal(icon_size.height()));
    } else {
        px.setDevicePixelRatio(pixelRatio);
    }

    return px;
}

QPixmap ItemWidget::GetFileIcon(QString path)
{
    QFileInfo info(path);
    QIcon icon;

    if (!QFileInfo::exists(path)) {
        QMimeDatabase db;
        QMimeType mime = db.mimeTypeForFile(path);
        return QIcon::fromTheme(mime.genericIconName()).pixmap(FileIconWidth, FileIconWidth);
    }
    QScopedPointer<DGioFile> file(DGioFile::createFromPath(info.absoluteFilePath()));
    QExplicitlySharedDataPointer<DGioFileInfo> fileinfo = file->createFileInfo();
    if (!fileinfo) {
        return QPixmap();
    }

    QStringList icons = fileinfo->themedIconNames();
    if (!icons.isEmpty()) {
        icon =  QIcon::fromTheme(icons.first()).pixmap(FileIconWidth, FileIconWidth);
    } else {
        QString iconStr(fileinfo->iconString());
        if (iconStr.startsWith('/')) {
            icon = QIcon(iconStr);
        } else {
            icon = QIcon::fromTheme(iconStr);
        }
    }

    //get additional icons
    QPixmap pix = icon.pixmap(FileIconWidth, FileIconWidth);
    QList<QRectF> cornerGeometryList = getCornerGeometryList(pix.rect(), pix.size() / 4);
    QList<QIcon> iconList;
    if (info.isSymLink()) {
        iconList << QIcon::fromTheme("emblem-symbolic-link");
    }

    if (!info.isWritable()) {
        iconList << QIcon::fromTheme("emblem-readonly");
    }

    if (!info.isReadable()) {
        iconList << QIcon::fromTheme("emblem-unreadable");
    }

    //if info is shared()  add icon "emblem-shared"(code from 'dde-file-manager')

    QPainter painter(&pix);
    painter.setRenderHints(painter.renderHints() | QPainter::SmoothPixmapTransform);
    for (int i = 0; i < iconList.size(); ++i) {
        painter.drawPixmap(cornerGeometryList.at(i).toRect(),
                           getIconPixmap(iconList.at(i), QSize(24, 24), painter.device()->devicePixelRatioF(), QIcon::Normal, QIcon::On));
    }

    return pix;
}

QPixmap ItemWidget::GetFileIcon(const FileIconData &data)
{
    QIcon icon;
    QStringList icons = data.cornerIconList;
    QPixmap pix = data.fileIcon.pixmap(QSize(FileIconWidth, FileIconWidth));
    if (icons.isEmpty()) {
        return pix;
    }

    QPainter painter(&pix);
    painter.setRenderHints(painter.renderHints() | QPainter::SmoothPixmapTransform);
    QList<QRectF> cornerGeometryList = getCornerGeometryList(pix.rect(), pix.size() / 4);
    for (int i = 0; i < icons.size(); ++i) {
        painter.drawPixmap(cornerGeometryList.at(i).toRect(),
                           getIconPixmap(QIcon::fromTheme(icons.at(i)), QSize(24, 24), painter.device()->devicePixelRatioF(), QIcon::Normal, QIcon::On));
    }

    return pix;
}

void ItemWidget::onSelect()
{
    if (!m_data->dataEnabled()) {
        return;
    }

    if (m_data->type() == File) {
        QList<QUrl> urls = m_data->urls();
        bool has = false;
        foreach (auto url, urls) {
            if (QDir().exists(url.toLocalFile())) {
                has = true;
            }
        }
        if (!has) {
            m_data->setDataEnabled(false);
            //源文件被删除需要提示
            m_contentLabel->setEnabled(false);
            QFontMetrics metrix = m_statusLabel->fontMetrics();
            QString tips = tr("(File deleted)");
            int tipsWidth = metrix.horizontalAdvance(tips);
            QString text = metrix.elidedText(m_statusLabel->text(), Qt::ElideMiddle, WindowWidth - 2 * ItemMargin - 10 - tipsWidth, 0);
            m_statusLabel->setText(text + tips);

            return;
        }
    }

    m_data->popTop();
}

void ItemWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_0:
        //表示切换‘焦点’，tab按键事件在delegate中已被拦截
        if (event->text() == "change focus") {
            Q_EMIT closeHasFocus(m_closeFocus = !m_closeFocus);
            return ;
        }
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return: {
        if (m_closeFocus) {
            onClose();
        } else {
            onSelect();
        }
    }
    break;
    }

    return DWidget::keyPressEvent(event);
}

void ItemWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPalette pe = this->palette();
    QColor c = pe.color(QPalette::Base);

    QPen borderPen;
    borderPen.setColor(Qt::transparent);
    painter.setPen(borderPen);

    //裁剪绘制区域
    QPainterPath path;
    path.addRoundedRect(rect(), m_radius, m_radius);
    painter.setClipPath(path);

    //绘制标题区域
    QColor brushColor(c);
    brushColor.setAlpha(80);
    painter.setBrush(brushColor);
    painter.drawRect(QRect(0, 0, width(), ItemTitleHeight));

    //绘制背景
    brushColor.setAlpha(m_havor ? m_hoverAlpha : m_unHoverAlpha);
    painter.setBrush(brushColor);
    painter.drawRect(rect());

    return DWidget::paintEvent(event);
}

void ItemWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    onSelect();
    return DWidget::mousePressEvent(event);
}

void ItemWidget::focusInEvent(QFocusEvent *event)
{
    if (isEnabled()) {
        Q_EMIT hoverStateChanged(true);
    }

    return DWidget::focusInEvent(event);
}

void ItemWidget::focusOutEvent(QFocusEvent *event)
{
    if (isEnabled()) {
        m_closeFocus = false;

        Q_EMIT hoverStateChanged(false);
        Q_EMIT closeHasFocus(false);
    }

    return DWidget::focusOutEvent(event);
}

bool ItemWidget::eventFilter(QObject *watcher, QEvent *event)
{
    if (watcher == qApp && event->type() == QEvent::ThemeChange) {
        setThumnail(m_pixmap);
    }

    return false;
}
