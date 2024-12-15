// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notifyentity.h"

#include <QDateTime>
#include <QStringList>
#include <QLoggingCategory>

namespace notification {
Q_LOGGING_CATEGORY(notifyLog, "ocean.shell.notification")

#define ACTION_SEGMENT ("|")
#define HINT_SEGMENT ("|")
#define KEY_VALUE_SEGMENT ("!!!")

static const uint NoReplaceId = 0;

class NotifyData : public QSharedData
{
public:
    explicit NotifyData()
        : QSharedData()
    {
    }

    QString appId;
    QString appName;
    QString appIcon;
    QString summary;
    QString body;
    QStringList actions;
    QVariantMap hints;
    uint bubbleId = 0;
    uint replacesId = NoReplaceId;
    int expireTimeout = 0;

    qint64 id = -1;
    qint64 cTime = 0;
    int processedType = NotifyEntity::NotProcessed;
};

NotifyEntity::NotifyEntity()
    : d(new NotifyData())
{

}

NotifyEntity::NotifyEntity(qint64 id, const QString &appName)
    : NotifyEntity()
{
    d->id = id;
    d->appName = appName;
}

NotifyEntity::NotifyEntity(const QString &appName, uint replacesId, const QString &appIcon, const QString &summary,
                           const QString &body, const QStringList &actions, const QVariantMap &hints,
                           int expireTimeout)
    : d(new NotifyData())
{
    d->appName = appName;
    d->replacesId = replacesId;
    d->appIcon = appIcon;
    d->summary = summary;
    d->body = body;
    d->actions = actions;
    d->hints = hints;
    d->expireTimeout = expireTimeout;
    d->cTime = QDateTime::currentMSecsSinceEpoch();
}

 NotifyEntity::NotifyEntity(const NotifyEntity &other)
    : d(other.d)
 {

 }

 NotifyEntity::NotifyEntity(NotifyEntity &&other) noexcept
 {
     d = other.d;
     other.d = nullptr;
 }

 NotifyEntity::~NotifyEntity()
 {

 }

 NotifyEntity &NotifyEntity::operator=(const NotifyEntity &other)
 {
     if (this != &other)
         d = other.d;

     return *this;
 }

 NotifyEntity &NotifyEntity::operator=(NotifyEntity &&other)
 {
     d.swap(other.d);
     return *this;
 }

bool NotifyEntity::operator==(const NotifyEntity &other) const
{
    if (d && other.d) {
        return id() == other.id();
    }
    return false;
}

bool NotifyEntity::operator!=(const NotifyEntity &other) const
{
    return !(operator==(other));
}

bool NotifyEntity::isValid() const
{
    return d && d->id > 0;
}

qint64 NotifyEntity::id() const
{
    return d->id;
}

void NotifyEntity::setId(qint64 id)
{
    d->id = id;
}

QString NotifyEntity::appName() const
{
    return d->appName;
}

void NotifyEntity::setAppName(const QString &appName)
{
    d->appName = appName;
}

QString NotifyEntity::appId() const
{
    return d->appId;
}

void NotifyEntity::setAppId(const QString &appId)
{
    d->appId = appId;
}

QString NotifyEntity::body() const
{
    return d->body;
}

void NotifyEntity::setBody(const QString &body)
{
    d->body = body;
}

QString NotifyEntity::summary() const
{
    return d->summary;
}

void NotifyEntity::setSummary(const QString &summary)
{
    d->summary = summary;
}

QString NotifyEntity::appIcon() const
{
    return d->appIcon;
}

void NotifyEntity::setAppIcon(const QString &appIcon)
{
    d->appIcon = appIcon;
}

QStringList NotifyEntity::actions() const
{
    return d->actions;
}

QString NotifyEntity::actionsString() const
{
    return convertActionsToString(d->actions);
}

void NotifyEntity::setActionString(const QString &actionString)
{
    d->actions = parseAction(actionString);
}

QVariantMap NotifyEntity::hints() const
{
    return d->hints;
}

QString NotifyEntity::hintsString() const
{
    return convertHintsToString(d->hints);
}

void NotifyEntity::setHintString(const QString &hintString)
{
    d->hints = parseHint(hintString);
}

uint NotifyEntity::replacesId() const
{
    return d->replacesId;
}

void NotifyEntity::setReplacesId(uint replacesId)
{
    d->replacesId = replacesId;
}

bool NotifyEntity::isReplace() const
{
    return d->replacesId != NoReplaceId;
}

qint64 NotifyEntity::cTime() const
{
    return d->cTime;
}

void NotifyEntity::setCTime(qint64 cTime)
{
    d->cTime = cTime;
}

bool NotifyEntity::processed() const
{
    return d->processedType == NotifyEntity::Processed;
}

int NotifyEntity::processedType() const
{
    return d->processedType;
}

void NotifyEntity::setProcessedType(int type)
{
    d->processedType = type;
}

QString NotifyEntity::convertActionsToString(const QStringList &actions)
{
    QString action;
    foreach (QString text, actions) {
        action += text;
        action += ACTION_SEGMENT;
    }

    if (!action.isEmpty())
        action = action.mid(0, action.length() - 1);

    return action;
}

uint NotifyEntity::bubbleId() const
{
    return d->bubbleId;
}

void NotifyEntity::setBubbleId(qint64 bubbleId)
{
    d->bubbleId = bubbleId;
}

// https://specifications.freedesktop.org/notification-spec/1.2/icons-and-images.html
QString NotifyEntity::bodyIcon() const
{
    const auto hints = this->hints();
    if (auto iter = hints.find("image-path"); iter != hints.end()) {
        const auto path = iter.value().toString();
        return path;
    }
    return QString();
}

QString NotifyEntity::convertHintsToString(const QVariantMap &map)
{
    QString text;

    QMapIterator<QString, QVariant> it(map);
    while (it.hasNext()) {
        it.next();
        QString key = it.key();
        text += key;
        text += KEY_VALUE_SEGMENT;
        QString value = it.value().toString();
        text += value;
        text += HINT_SEGMENT;
    }

    return text;
}

QStringList NotifyEntity::parseAction(const QString &action)
{
    if (action.isEmpty())
        return {};

    QStringList actions = action.split("|");
    const auto defaultIndex = actions.indexOf("default");
    Q_ASSERT((defaultIndex % 2) != 1);
    if (defaultIndex < 0)
        Q_ASSERT((actions.size() % 2) != 1);

    return actions;
}

QVariantMap NotifyEntity::parseHint(const QString &hint)
{
    if (hint.isEmpty())
        return {};

    QVariantMap map;
    QStringList keyValueList = hint.split("|");
    foreach (QString text, keyValueList) {
        QStringList list = text.split("!!!");
        if (list.size() != 2)
            continue;
        const QString &key = list[0];
        QVariant value = QVariant::fromValue(list[1]);

        map.insert(key, value);
    }

    return map;
}

}
