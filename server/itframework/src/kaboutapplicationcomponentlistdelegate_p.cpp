/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2021 Julius Künzel <jk.kdedev@smartlab.uber.space>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kaboutapplicationcomponentlistdelegate_p.h"

#include "kaboutapplicationcomponentmodel_p.h"
#include "kaboutapplicationlistview_p.h"
#include "klicensedialog_p.h"
#include "ktoolbar.h"

#include <KLocalizedString>

#include <QAction>
#include <QApplication>
#include <QDesktopServices>
#include <QLabel>
#include <QPainter>
#include <QStandardPaths>
#include <QUrl>

namespace KDEPrivate
{

KAboutApplicationComponentListDelegate::KAboutApplicationComponentListDelegate(QAbstractItemView *itemView, QObject *parent)
    : KWidgetItemDelegate(itemView, parent)
{
}

QList<QWidget *> KAboutApplicationComponentListDelegate::createItemWidgets(const QModelIndex &index) const
{
    Q_UNUSED(index);
    QList<QWidget *> list;

    QLabel *textLabel = new QLabel(itemView());
    list.append(textLabel);

    KAboutApplicationComponentProfile profile = index.data().value<KAboutApplicationComponentProfile>();
    textLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    QObject::connect(textLabel, &QLabel::linkActivated, itemView(), [this, profile]() {
        auto *dialog = new KLicenseDialog(profile.license(), itemView());
        dialog->show();
    });

    KToolBar *mainLinks = new KToolBar(itemView(), false, false);

    QAction *homepageAction = new QAction(QIcon::fromTheme(QStringLiteral("internet-services")), //
                                          i18n("Visit component's homepage"),
                                          mainLinks);
    homepageAction->setVisible(false);
    mainLinks->addAction(homepageAction);
    QAction *visitProfileAction = new QAction(QIcon::fromTheme(QStringLiteral("get-hot-new-stuff")), QString(), mainLinks);
    visitProfileAction->setVisible(false);
    mainLinks->addAction(visitProfileAction);

    list.append(mainLinks);

    connect(mainLinks, &QToolBar::actionTriggered, this, &KAboutApplicationComponentListDelegate::launchUrl);

    return list;
}

void KAboutApplicationComponentListDelegate::updateItemWidgets(const QList<QWidget *> widgets,
                                                               const QStyleOptionViewItem &option,
                                                               const QPersistentModelIndex &index) const
{
    const int margin = option.fontMetrics.height() / 2;

    KAboutApplicationComponentProfile profile = index.data().value<KAboutApplicationComponentProfile>();

    QRect wRect = widgetsRect(option, index);

    // Let's fill in the text first...
    QLabel *label = qobject_cast<QLabel *>(widgets.at(TextLabel));
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QString text = buildTextForProfile(profile);

    label->move(wRect.left(), wRect.top());
    label->resize(wRect.width(), heightForString(text, wRect.width() - margin, option) + margin);
    label->setWordWrap(true);
    label->setContentsMargins(0, 0, 0, 0);
    label->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
    label->setForegroundRole(QPalette::WindowText);

    label->setText(text);

    // And now we fill in the main links (email + homepage + OCS profile)...
    KToolBar *mainLinks = qobject_cast<KToolBar *>(widgets.at(MainLinks));
    mainLinks->setIconSize(QSize(22, 22));
    mainLinks->setContentsMargins(0, 0, 0, 0);
    mainLinks->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QAction *action;
    if (!profile.webAdress().isEmpty()) {
        action = mainLinks->actions().at(HomepageAction);
        action->setToolTip(i18nc("@info:tooltip", "Visit components's homepage\n%1", profile.webAdress()));
        action->setData(profile.webAdress());
        action->setVisible(true);
    }
    mainLinks->resize(QSize(mainLinks->sizeHint().width(), LINK_HEIGHT));
    mainLinks->move(wRect.left(), wRect.top() + label->height());
    itemView()->reset();
}

QSize KAboutApplicationComponentListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int height = widgetsRect(option, index).height();

    QSize metrics(option.fontMetrics.height() * 7, height);
    return metrics;
}

void KAboutApplicationComponentListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    QStyle *style = QApplication::style();
    style->drawPrimitive(QStyle::PE_Widget, &option, painter, nullptr);
}

void KAboutApplicationComponentListDelegate::launchUrl(QAction *action) const
{
    QString url = action->data().toString();
    if (!url.isEmpty()) {
        QDesktopServices::openUrl(QUrl(url));
    }
}

int KAboutApplicationComponentListDelegate::heightForString(const QString &string, int lineWidth, const QStyleOptionViewItem &option) const
{
    QFontMetrics fm = option.fontMetrics;
    constexpr auto opts = Qt::AlignLeft | Qt::AlignBottom | Qt::TextWordWrap;
    QRect boundingRect = fm.boundingRect(0, 0, lineWidth, 9999, opts, string);
    return boundingRect.height();
}

QString KAboutApplicationComponentListDelegate::buildTextForProfile(const KAboutApplicationComponentProfile &profile) const
{
    QString text = QLatin1String("<b>") + i18nc("@item Component name in about dialog.", "%1", profile.name()) + QLatin1String("</b>");

    if (!profile.version().isEmpty()) {
        text += QStringLiteral("\n<br><i>%1</i>").arg(i18n("Version %1", profile.version()));
    }

    if (!profile.description().isEmpty()) {
        text += QLatin1String("\n<br>") + profile.description();
    }

    if (profile.license().key() != KAboutLicense::Unknown) {
        text += QLatin1String("\n<br>");
        text += QStringLiteral("<a href=\"#\">%2</a>").arg(i18n("License: %1", profile.license().name(KAboutLicense::FullName)));
    }

    return text;
}

QRect KAboutApplicationComponentListDelegate::widgetsRect(const QStyleOptionViewItem &option, const QPersistentModelIndex &index) const
{
    KAboutApplicationComponentProfile profile = index.data().value<KAboutApplicationComponentProfile>();
    int margin = option.fontMetrics.height() / 2;

    QRect widgetsRect = QRect(option.rect.left() + margin, //
                              margin / 2, //
                              option.rect.width() - 2 * margin, //
                              0);

    int textHeight = heightForString(buildTextForProfile(profile), widgetsRect.width() - margin, option);
    widgetsRect.setHeight(textHeight + LINK_HEIGHT + 1.5 * margin);

    return widgetsRect;
}

} // namespace KDEPrivate
