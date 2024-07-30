/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 1999 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "navigationextension.h"

#include "kparts_logging.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <KUriFilter>

#include <QApplication>
#include <QClipboard>
#include <QMap>
#include <QRegularExpression>
#include <QTimer>

using namespace KParts;

namespace KParts
{
// Internal class, use to store the status of the actions
class KBitArray
{
public:
    int val = 0;
    bool operator[](int index)
    {
        return (val & (1 << index)) ? true : false;
    }
    void setBit(int index, bool value)
    {
        if (value) {
            val = val | (1 << index);
        } else {
            val = val & ~(1 << index);
        }
    }
};

class NavigationExtensionPrivate
{
public:
    NavigationExtensionPrivate(KParts::ReadOnlyPart *parent)
        : m_urlDropHandlingEnabled(false)
        , m_part(parent)
    {
    }

    struct DelayedRequest {
        QUrl m_delayedURL;
        KParts::OpenUrlArguments m_delayedArgs;
    };

    QList<DelayedRequest> m_requests;
    bool m_urlDropHandlingEnabled;
    KBitArray m_actionStatus;
    QMap<int, QString> m_actionText;

    static void createActionSlotMap();

    KParts::ReadOnlyPart *m_part;
};

Q_GLOBAL_STATIC(NavigationExtension::ActionSlotMap, s_actionSlotMap)
Q_GLOBAL_STATIC(NavigationExtension::ActionNumberMap, s_actionNumberMap)

void NavigationExtensionPrivate::createActionSlotMap()
{
    s_actionSlotMap()->insert("cut", SLOT(cut()));
    s_actionSlotMap()->insert("copy", SLOT(copy()));
    s_actionSlotMap()->insert("paste", SLOT(paste()));
    s_actionSlotMap()->insert("print", SLOT(print()));
    // Tricky. Those aren't actions in fact, but simply methods that a browserextension
    // can have or not. No need to return them here.
    // s_actionSlotMap()->insert( "reparseConfiguration", SLOT(reparseConfiguration()) );
    // s_actionSlotMap()->insert( "refreshMimeTypes", SLOT(refreshMimeTypes()) );

    // Create the action-number map
    NavigationExtension::ActionSlotMap::ConstIterator it = s_actionSlotMap()->constBegin();
    NavigationExtension::ActionSlotMap::ConstIterator itEnd = s_actionSlotMap()->constEnd();
    for (int i = 0; it != itEnd; ++it, ++i) {
        // qDebug() << " action " << it.key() << " number " << i;
        s_actionNumberMap()->insert(it.key(), i);
    }
}

}

NavigationExtension::NavigationExtension(KParts::ReadOnlyPart *parent)
    : QObject(parent)
    , d(new NavigationExtensionPrivate(parent))
{
    if (s_actionSlotMap()->isEmpty())
    // Create the action-slot map
    {
        NavigationExtensionPrivate::createActionSlotMap();
    }

    // Set the initial status of the actions depending on whether
    // they're supported or not
    const QMetaObject *metaobj = metaObject();
    ActionSlotMap::ConstIterator it = s_actionSlotMap()->constBegin();
    ActionSlotMap::ConstIterator itEnd = s_actionSlotMap()->constEnd();
    for (int i = 0; it != itEnd; ++it, ++i) {
        // Does the extension have a slot with the name of this action ?
        QByteArray slotSig = it.key() + "()";
        d->m_actionStatus.setBit(i, metaobj->indexOfMethod(slotSig.constData()) != -1);
    }

    connect(this, &NavigationExtension::openUrlRequest, this, &NavigationExtension::slotOpenUrlRequest);
    connect(this, &NavigationExtension::enableAction, this, &NavigationExtension::slotEnableAction);
    connect(this, &NavigationExtension::setActionText, this, &NavigationExtension::slotSetActionText);
}

NavigationExtension::~NavigationExtension()
{
    // qDebug() << "BrowserExtension::~BrowserExtension() " << this;
}

int NavigationExtension::xOffset()
{
    return 0;
}

int NavigationExtension::yOffset()
{
    return 0;
}

void NavigationExtension::saveState(QDataStream &stream)
{
    // TODO add d->m_part->mimeType()
    stream << d->m_part->url() << static_cast<qint32>(xOffset()) << static_cast<qint32>(yOffset());
}

void NavigationExtension::restoreState(QDataStream &stream)
{
    QUrl u;
    qint32 xOfs;
    qint32 yOfs;
    stream >> u >> xOfs >> yOfs;

    OpenUrlArguments args;
    args.setXOffset(xOfs);
    args.setYOffset(yOfs);
    // TODO add args.setMimeType
    d->m_part->setArguments(args);
    d->m_part->openUrl(u);
}

bool NavigationExtension::isURLDropHandlingEnabled() const
{
    return d->m_urlDropHandlingEnabled;
}

void NavigationExtension::setURLDropHandlingEnabled(bool enable)
{
    d->m_urlDropHandlingEnabled = enable;
}

void NavigationExtension::pasteRequest()
{
    QString plain(QStringLiteral("plain"));
    QString url = QApplication::clipboard()->text(plain, QClipboard::Selection).trimmed();
    // Remove linefeeds and any whitespace surrounding it.
    url.remove(QRegularExpression(QStringLiteral("[\\ ]*\\n+[\\ ]*")));

    // Check if it's a URL
    QStringList filters = KUriFilter::self()->pluginNames();
    filters.removeAll(QStringLiteral("kuriikwsfilter"));
    filters.removeAll(QStringLiteral("localdomainurifilter"));
    KUriFilterData filterData;
    filterData.setData(url);
    filterData.setCheckForExecutables(false);
    if (KUriFilter::self()->filterUri(filterData, filters)) {
        switch (filterData.uriType()) {
        case KUriFilterData::LocalFile:
        case KUriFilterData::LocalDir:
        case KUriFilterData::NetProtocol:
            slotOpenUrlRequest(filterData.uri());
            break;
        case KUriFilterData::Error:
            KMessageBox::error(d->m_part->widget(), filterData.errorMsg());
            break;
        default:
            break;
        }
    } else if (KUriFilter::self()->filterUri(filterData, QStringList(QStringLiteral("kuriikwsfilter"))) && url.length() < 250) {
        if (KMessageBox::questionTwoActions(d->m_part->widget(),
                                            i18n("<qt>Do you want to search the Internet for <b>%1</b>?</qt>", url.toHtmlEscaped()),
                                            i18n("Internet Search"),
                                            KGuiItem(i18n("&Search"), QStringLiteral("edit-find")),
                                            KStandardGuiItem::cancel(),
                                            QStringLiteral("MiddleClickSearch"))
            == KMessageBox::PrimaryAction) {
            slotOpenUrlRequest(filterData.uri());
        }
    }
}

void NavigationExtension::slotOpenUrlRequest(const QUrl &url, const KParts::OpenUrlArguments &args)
{
    // qDebug() << this << " BrowserExtension::slotOpenURLRequest(): url=" << url.url();
    NavigationExtensionPrivate::DelayedRequest req;
    req.m_delayedURL = url;
    req.m_delayedArgs = args;
    d->m_requests.append(req);
    QTimer::singleShot(0, this, &NavigationExtension::slotEmitOpenUrlRequestDelayed);
}

void NavigationExtension::slotEmitOpenUrlRequestDelayed()
{
    if (d->m_requests.isEmpty()) {
        return;
    }
    NavigationExtensionPrivate::DelayedRequest req = d->m_requests.front();
    d->m_requests.pop_front();
    Q_EMIT openUrlRequestDelayed(req.m_delayedURL, req.m_delayedArgs);
    // tricky: do not do anything here! (no access to member variables, etc.)
}

void NavigationExtension::slotEnableAction(const char *name, bool enabled)
{
    // qDebug() << "BrowserExtension::slotEnableAction " << name << " " << enabled;
    ActionNumberMap::ConstIterator it = s_actionNumberMap()->constFind(name);
    if (it != s_actionNumberMap()->constEnd()) {
        d->m_actionStatus.setBit(it.value(), enabled);
        // qDebug() << "BrowserExtension::slotEnableAction setting bit " << it.data() << " to " << enabled;
    } else {
        qCWarning(KPARTSLOG) << "BrowserExtension::slotEnableAction unknown action " << name;
    }
}

bool NavigationExtension::isActionEnabled(const char *name) const
{
    int actionNumber = (*s_actionNumberMap())[name];
    return d->m_actionStatus[actionNumber];
}

void NavigationExtension::slotSetActionText(const char *name, const QString &text)
{
    // qDebug() << "BrowserExtension::slotSetActionText " << name << " " << text;
    ActionNumberMap::ConstIterator it = s_actionNumberMap()->constFind(name);
    if (it != s_actionNumberMap()->constEnd()) {
        d->m_actionText[it.value()] = text;
    } else {
        qCWarning(KPARTSLOG) << "BrowserExtension::slotSetActionText unknown action " << name;
    }
}

QString NavigationExtension::actionText(const char *name) const
{
    int actionNumber = (*s_actionNumberMap())[name];
    QMap<int, QString>::ConstIterator it = d->m_actionText.constFind(actionNumber);
    if (it != d->m_actionText.constEnd()) {
        return *it;
    }
    return QString();
}

NavigationExtension::ActionSlotMap *NavigationExtension::actionSlotMap()
{
    if (s_actionSlotMap()->isEmpty()) {
        NavigationExtensionPrivate::createActionSlotMap();
    }
    return s_actionSlotMap();
}

NavigationExtension *NavigationExtension::childObject(QObject *obj)
{
    return obj->findChild<KParts::NavigationExtension *>(QString(), Qt::FindDirectChildrenOnly);
}

#include "moc_navigationextension.cpp"
