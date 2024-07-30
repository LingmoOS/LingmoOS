/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Urs Wolfer <uwolfer@kde.org>
    SPDX-FileCopyrightText: 2007 Michaël Larouche <larouche@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ktitlewidget.h"

#include <QApplication>
#include <QFrame>
#include <QIcon>
#include <QLabel>
#include <QLayout>
#include <QMouseEvent>
#include <QStyle>
#include <QTextDocument>
#include <QTimer>

class KTitleWidgetPrivate
{
public:
    KTitleWidgetPrivate(KTitleWidget *parent)
        : q(parent)
        // use Left so updateIconAlignment(ImageRight) as called by constructor triggers the default layout
        , iconAlignment(KTitleWidget::ImageLeft)
        , autoHideTimeout(0)
        , messageType(KTitleWidget::InfoMessage)
    {
    }

    QString textStyleSheet() const
    {
        qreal factor;
        switch (level) {
        case 1:
            factor = 1.35;
            break;
        case 2:
            factor = 1.20;
            break;
        case 3:
            factor = 1.15;
            break;
        case 4:
            factor = 1.10;
            break;
        default:
            factor = 1;
        }
        const double fontSize = QApplication::font().pointSize() * factor;
        return QStringLiteral("QLabel { font-size: %1pt; color: %2 }").arg(QString::number(fontSize), q->palette().color(QPalette::WindowText).name());
    }

    QString commentStyleSheet() const
    {
        QString styleSheet;
        switch (messageType) {
        // FIXME: we need the usability color styles to implement different
        //       yet palette appropriate colours for the different use cases!
        //       also .. should we include an icon here,
        //       perhaps using the imageLabel?
        case KTitleWidget::InfoMessage:
        case KTitleWidget::WarningMessage:
        case KTitleWidget::ErrorMessage:
            styleSheet = QStringLiteral("QLabel { color: palette(%1); background: palette(%2); }")
                             .arg(q->palette().color(QPalette::HighlightedText).name(), q->palette().color(QPalette::Highlight).name());
            break;
        case KTitleWidget::PlainMessage:
        default:
            break;
        }
        return styleSheet;
    }

    void updateIconAlignment(KTitleWidget::ImageAlignment newIconAlignment)
    {
        if (iconAlignment == newIconAlignment) {
            return;
        }

        iconAlignment = newIconAlignment;

        headerLayout->removeWidget(textLabel);
        headerLayout->removeWidget(commentLabel);
        headerLayout->removeWidget(imageLabel);

        if (iconAlignment == KTitleWidget::ImageLeft) {
            // swap the text and image labels around
            headerLayout->addWidget(imageLabel, 0, 0, 2, 1);
            headerLayout->addWidget(textLabel, 0, 1);
            headerLayout->addWidget(commentLabel, 1, 1);
            headerLayout->setColumnStretch(0, 0);
            headerLayout->setColumnStretch(1, 1);
        } else {
            headerLayout->addWidget(textLabel, 0, 0);
            headerLayout->addWidget(commentLabel, 1, 0);
            headerLayout->addWidget(imageLabel, 0, 1, 2, 1);
            headerLayout->setColumnStretch(1, 0);
            headerLayout->setColumnStretch(0, 1);
        }
    }

    void updatePixmap()
    {
        const QPixmap pixmap = icon.pixmap(q->iconSize());
        imageLabel->setPixmap(pixmap);
    }

    int level = 1;
    KTitleWidget *const q;
    QGridLayout *headerLayout;
    QLabel *imageLabel;
    QLabel *textLabel;
    QLabel *commentLabel;
    QIcon icon;
    QSize iconSize;
    KTitleWidget::ImageAlignment iconAlignment;
    int autoHideTimeout;
    KTitleWidget::MessageType messageType;

    /**
     * @brief Get the icon name from the icon type
     * @param type icon type from the enum
     * @return named icon as QString
     */
    QString iconTypeToIconName(KTitleWidget::MessageType type);

    void timeoutFinished()
    {
        q->setVisible(false);
    }
};

QString KTitleWidgetPrivate::iconTypeToIconName(KTitleWidget::MessageType type)
{
    switch (type) {
    case KTitleWidget::InfoMessage:
        return QStringLiteral("dialog-information");
    case KTitleWidget::ErrorMessage:
        return QStringLiteral("dialog-error");
    case KTitleWidget::WarningMessage:
        return QStringLiteral("dialog-warning");
    case KTitleWidget::PlainMessage:
        break;
    }

    return QString();
}

KTitleWidget::KTitleWidget(QWidget *parent)
    : QWidget(parent)
    , d(new KTitleWidgetPrivate(this))
{
    // default image / text part start
    d->headerLayout = new QGridLayout();
    d->headerLayout->setContentsMargins(0, 0, 0, 0);
    d->headerLayout->setSizeConstraint(QLayout::SetFixedSize);

    d->textLabel = new QLabel(this);
    d->textLabel->setVisible(false);
    d->textLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);

    d->imageLabel = new QLabel(this);
    d->imageLabel->setVisible(false);

    d->commentLabel = new QLabel(this);
    d->commentLabel->setVisible(false);
    d->commentLabel->setOpenExternalLinks(true);
    d->commentLabel->setWordWrap(true);
    d->commentLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);

    d->updateIconAlignment(ImageRight); // make sure d->iconAlignment is left, to trigger initial layout
    // default image / text part end

    // vertical centering of the complete header
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(d->headerLayout);
    // headerLayout's sizeConstraint being QLayout::SetFixedSize is ignored when added to a layout
    // with default Qt::Alignment(). instead is stretched to match the whole size.
    // Sadly QVBoxLayout::addLayout does not have a alignment argument, other than QVBoxLayout::addWidget,
    // so set it afterwards onto the layout item generated.
    mainLayout->itemAt(0)->setAlignment(Qt::AlignVCenter);
    mainLayout->setContentsMargins(0, 0, 0, 0);
}

KTitleWidget::~KTitleWidget() = default;

bool KTitleWidget::eventFilter(QObject *object, QEvent *event)
{
    // Hide message label on click
    if (d->autoHideTimeout > 0 && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent && mouseEvent->button() == Qt::LeftButton) {
            setVisible(false);
            return true;
        }
    }

    return QWidget::eventFilter(object, event);
}

void KTitleWidget::setWidget(QWidget *widget)
{
    d->headerLayout->addWidget(widget, 2, 0, 1, 2);
}

QString KTitleWidget::text() const
{
    return d->textLabel->text();
}

QString KTitleWidget::comment() const
{
    return d->commentLabel->text();
}

QIcon KTitleWidget::icon() const
{
    return d->icon;
}

QSize KTitleWidget::iconSize() const
{
    if (d->iconSize.isValid()) {
        return d->iconSize;
    }
    const int iconSizeExtent = style()->pixelMetric(QStyle::PM_MessageBoxIconSize);
    return QSize(iconSizeExtent, iconSizeExtent);
}

void KTitleWidget::setBuddy(QWidget *buddy)
{
    d->textLabel->setBuddy(buddy);
}

void KTitleWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    if (e->type() == QEvent::PaletteChange || e->type() == QEvent::FontChange || e->type() == QEvent::ApplicationFontChange) {
        d->textLabel->setStyleSheet(d->textStyleSheet());
        d->commentLabel->setStyleSheet(d->commentStyleSheet());
        d->updatePixmap();
    } else if (e->type() == QEvent::StyleChange) {
        if (!d->iconSize.isValid()) {
            // relies on style's PM_MessageBoxIconSize
            d->updatePixmap();
        }
    }
}

void KTitleWidget::setText(const QString &text, Qt::Alignment alignment)
{
    d->textLabel->setVisible(!text.isNull());

    if (!Qt::mightBeRichText(text)) {
        d->textLabel->setStyleSheet(d->textStyleSheet());
    }

    d->textLabel->setText(text);
    d->textLabel->setAlignment(alignment);
    show();
}

void KTitleWidget::setLevel(int level)
{
    if (d->level == level) {
        return;
    }

    d->level = level;

    d->textLabel->setStyleSheet(d->textStyleSheet());
}

int KTitleWidget::level()
{
    return d->level;
}

void KTitleWidget::setText(const QString &text, MessageType type)
{
    setIcon(type);
    setText(text);
}

void KTitleWidget::setComment(const QString &comment, MessageType type)
{
    d->commentLabel->setVisible(!comment.isNull());

    // TODO: should we override the current icon with the corresponding MessageType icon?
    d->messageType = type;
    d->commentLabel->setStyleSheet(d->commentStyleSheet());
    d->commentLabel->setText(comment);
    show();
}

void KTitleWidget::setIcon(const QIcon &icon, KTitleWidget::ImageAlignment alignment)
{
    d->icon = icon;

    d->imageLabel->setVisible(!icon.isNull());

    d->updateIconAlignment(alignment);

    d->updatePixmap();
}

void KTitleWidget::setIconSize(const QSize &iconSize)
{
    if (d->iconSize == iconSize) {
        return;
    }

    const QSize oldEffectiveIconSize = this->iconSize();

    d->iconSize = iconSize;

    if (oldEffectiveIconSize != this->iconSize()) {
        d->updatePixmap();
    }
}

void KTitleWidget::setIcon(MessageType type, ImageAlignment alignment)
{
    setIcon(QIcon::fromTheme(d->iconTypeToIconName(type)), alignment);
}

int KTitleWidget::autoHideTimeout() const
{
    return d->autoHideTimeout;
}

void KTitleWidget::setAutoHideTimeout(int msecs)
{
    d->autoHideTimeout = msecs;

    if (msecs > 0) {
        installEventFilter(this);
    } else {
        removeEventFilter(this);
    }
}

void KTitleWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    if (d->autoHideTimeout > 0) {
        QTimer::singleShot(d->autoHideTimeout, this, [this] {
            d->timeoutFinished();
        });
    }
}

#include "moc_ktitlewidget.cpp"
