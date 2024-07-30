/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2005 Olivier Goffart <ogoffart at kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "knotifyeventlist.h"

#include <knotifyconfig_debug.h>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

#include <QHeaderView>
#include <QPainter>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QStyledItemDelegate>

// BEGIN KNotifyEventListDelegate

class KNotifyEventList::KNotifyEventListDelegate : public QStyledItemDelegate
{
public:
    explicit KNotifyEventListDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
};

KNotifyEventList::KNotifyEventListDelegate::KNotifyEventListDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void KNotifyEventList::KNotifyEventListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() != 0) {
        return QStyledItemDelegate::paint(painter, option, index);
    }

    QVariant displayData = index.data(Qt::UserRole);
    QString prstring = displayData.toString();

    QStyledItemDelegate::paint(painter, option, index);

    //  qDebug() << prstring;

    QRect rect = option.rect;

    QStringList optionsList = prstring.split(QLatin1Char('|'));
    QList<QIcon> iconList;
    iconList << (optionsList.contains(QStringLiteral("Sound")) ? QIcon::fromTheme(QStringLiteral("media-playback-start")) : QIcon());
    iconList << (optionsList.contains(QStringLiteral("Popup")) ? QIcon::fromTheme(QStringLiteral("dialog-information")) : QIcon());

    int mc_x = 0;

    int iconWidth = option.decorationSize.width();
    int iconHeight = option.decorationSize.height();
    for (const QIcon &icon : std::as_const(iconList)) {
        icon.paint(painter, rect.left() + mc_x + 4, rect.top() + (rect.height() - iconHeight) / 2, iconWidth, iconHeight);
        mc_x += iconWidth + 4;
    }
}

// END KNotifyEventListDelegate

KNotifyEventList::KNotifyEventList(QWidget *parent)
    : QTreeWidget(parent)
    , config(nullptr)
{
    QStringList headerLabels;
    headerLabels << i18nc("State of the notified event", "State") << i18nc("Title of the notified event", "Title")
                 << i18nc("Description of the notified event", "Description");
    setHeaderLabels(headerLabels);

    setItemDelegate(new KNotifyEventListDelegate(this));
    setRootIsDecorated(false);
    setAlternatingRowColors(true);

    // Extract icon size as the font height (as h=w on icons)
    QStyleOptionViewItem iconOption;
    iconOption.initFrom(this);
    int iconWidth = iconOption.fontMetrics.height() - 2; // 1px margin top & bottom
    setIconSize(QSize(iconWidth, iconWidth));

    header()->setSectionResizeMode(0, QHeaderView::Fixed);
    header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(slotSelectionChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
}

KNotifyEventList::~KNotifyEventList()
{
    delete config;
}

void KNotifyEventList::fill(const QString &appname, bool loadDefaults)
{
    m_elements.clear();
    clear();
    delete config;
    config = new KConfig(appname + QStringLiteral(".notifyrc"), KConfig::NoGlobals);
    config->addConfigSources(
        QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("knotifications6/") + appname + QStringLiteral(".notifyrc")));

    QStringList conflist = config->groupList();
    QRegularExpression rx(QStringLiteral("^Event/([^/]*)$"));
    conflist = conflist.filter(rx);

    for (const QString &group : std::as_const(conflist)) {
        KConfigGroup cg(config, group);
        QString id = rx.match(group).captured(1);
        QString name = cg.readEntry("Name");
        QString description = cg.readEntry("Comment");

        if (loadDefaults) {
            KConfigGroup g(config, QStringLiteral("Event/") + id);
            const auto keyList = g.keyList();
            for (const QString &entry : keyList) {
                g.revertToDefault(entry);
            }
        }

        m_elements << new KNotifyEventListItem(this, id, name, description, config);
    }

    resizeColumnToContents(2);
}

void KNotifyEventList::save()
{
    for (KNotifyEventListItem *it : std::as_const(m_elements)) {
        it->save();
    }
    config->sync();
}

bool KNotifyEventList::disableAllSounds()
{
    bool changed = false;
    for (KNotifyEventListItem *it : std::as_const(m_elements)) {
        QStringList actions = it->configElement()->readEntry(QStringLiteral("Action")).split(QLatin1Char('|'));
        if (actions.removeAll(QStringLiteral("Sound"))) {
            it->configElement()->writeEntry(QStringLiteral("Action"), actions.join(QLatin1Char('|')));
            changed = true;
        }
    }
    return changed;
}

void KNotifyEventList::slotSelectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    Q_UNUSED(current);

    KNotifyEventListItem *it = dynamic_cast<KNotifyEventListItem *>(currentItem());
    if (it) {
        Q_EMIT eventSelected(it->configElement());
    } else {
        Q_EMIT eventSelected(nullptr);
    }

    it = dynamic_cast<KNotifyEventListItem *>(previous);
    if (it) {
        it->update();
    }
}

void KNotifyEventList::updateCurrentItem()
{
    KNotifyEventListItem *it = dynamic_cast<KNotifyEventListItem *>(currentItem());
    if (it) {
        it->update();
    }
}

void KNotifyEventList::updateAllItems()
{
    for (KNotifyEventListItem *it : std::as_const(m_elements)) {
        it->update();
    }
}

void KNotifyEventList::selectEvent(const QString &eventId)
{
    auto it = std::find_if(m_elements.constBegin(), m_elements.constEnd(), [&eventId](KNotifyEventListItem *item) {
        return item->configElement()->eventId() == eventId;
    });

    if (it != m_elements.constEnd()) {
        setCurrentItem(*it);
    }
}

QSize KNotifyEventList::sizeHint() const
{
    int fontSize = fontMetrics().height();
    return QSize(48 * fontSize, 12 * fontSize);
}

KNotifyEventListItem::KNotifyEventListItem(QTreeWidget *parent, const QString &eventName, const QString &name, const QString &description, KConfig *config)
    : QTreeWidgetItem(parent)
    , m_config(eventName, config)
{
    setText(1, name);
    setToolTip(1, description);
    setText(2, description);
    setToolTip(2, description);
    update();
}

KNotifyEventListItem::~KNotifyEventListItem()
{
}

void KNotifyEventListItem::save()
{
    m_config.save();
}

void KNotifyEventListItem::update()
{
    setData(0, Qt::UserRole, m_config.readEntry(QStringLiteral("Action")));
}

#include "moc_knotifyeventlist.cpp"
