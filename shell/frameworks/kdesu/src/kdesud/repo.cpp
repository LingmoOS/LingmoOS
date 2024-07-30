/* vi: ts=8 sts=4 sw=4

    This file is part of the KDE project, module kdesu.
    SPDX-FileCopyrightText: 1999, 2000 Geert Jansen <g.t.jansen@stud.tue.nl>
*/

#include "repo.h"

#include <ksud_debug.h>

#include <assert.h>
#include <time.h>

#include <QStack>

Repository::Repository()
{
    head_time = (unsigned)-1;
}

Repository::~Repository()
{
}

void Repository::add(const QByteArray &key, Data_entry &data)
{
    RepoIterator it = repo.find(key);
    if (it != repo.end()) {
        remove(key);
    }
    if (data.timeout == 0) {
        data.timeout = (unsigned)-1;
    } else {
        data.timeout += time(nullptr);
    }
    head_time = qMin(head_time, data.timeout);
    repo.insert(key, data);
}

int Repository::remove(const QByteArray &key)
{
    if (key.isEmpty()) {
        return -1;
    }

    RepoIterator it = repo.find(key);
    if (it == repo.end()) {
        return -1;
    }
    it.value().value.fill('x');
    it.value().group.fill('x');
    repo.erase(it);
    return 0;
}

int Repository::removeSpecialKey(const QByteArray &key)
{
    int found = -1;
    if (!key.isEmpty()) {
        QStack<QByteArray> rm_keys;
        for (RepoCIterator it = repo.constBegin(); it != repo.constEnd(); ++it) {
            if (key.indexOf(it.value().group) == 0 && it.key().indexOf(key) >= 0) {
                rm_keys.push(it.key());
                found = 0;
            }
        }
        while (!rm_keys.isEmpty()) {
            qCDebug(KSUD_LOG) << "Removed key: " << rm_keys.top();
            remove(rm_keys.pop());
        }
    }
    return found;
}

int Repository::removeGroup(const QByteArray &group)
{
    int found = -1;
    if (!group.isEmpty()) {
        QStack<QByteArray> rm_keys;
        for (RepoCIterator it = repo.constBegin(); it != repo.constEnd(); ++it) {
            if (it.value().group == group) {
                rm_keys.push(it.key());
                found = 0;
            }
        }
        while (!rm_keys.isEmpty()) {
            qCDebug(KSUD_LOG) << "Removed key: " << rm_keys.top();
            remove(rm_keys.pop());
        }
    }
    return found;
}

int Repository::hasGroup(const QByteArray &group) const
{
    if (!group.isEmpty()) {
        RepoCIterator it;
        for (it = repo.begin(); it != repo.end(); ++it) {
            if (it.value().group == group) {
                return 0;
            }
        }
    }
    return -1;
}

QByteArray Repository::findKeys(const QByteArray &group, const char *sep) const
{
    QByteArray list = "";
    if (!group.isEmpty()) {
        qCDebug(KSUD_LOG) << "Looking for matching key with group key: " << group;
        int pos;
        QByteArray key;
        RepoCIterator it;
        for (it = repo.begin(); it != repo.end(); ++it) {
            if (it.value().group == group) {
                key = it.key();
                qCDebug(KSUD_LOG) << "Matching key found: " << key;
                pos = key.lastIndexOf(sep);
                key.truncate(pos);
                key.remove(0, 2);
                if (!list.isEmpty()) {
                    // Add the same keys only once please :)
                    if (!list.contains(key)) {
                        qCDebug(KSUD_LOG) << "Key added to list: " << key;
                        list += '\007'; // I do not know
                        list.append(key);
                    }
                } else {
                    list = key;
                }
            }
        }
    }
    return list;
}

QByteArray Repository::find(const QByteArray &key) const
{
    if (key.isEmpty()) {
        return nullptr;
    }

    RepoCIterator it = repo.find(key);
    if (it == repo.end()) {
        return nullptr;
    }
    return it.value().value;
}

int Repository::expire()
{
    unsigned current = time(nullptr);
    if (current < head_time) {
        return 0;
    }

    unsigned t;
    QStack<QByteArray> keys;
    head_time = (unsigned)-1;
    RepoIterator it;
    for (it = repo.begin(); it != repo.end(); ++it) {
        t = it.value().timeout;
        if (t <= current) {
            keys.push(it.key());
        } else {
            head_time = qMin(head_time, t);
        }
    }

    int n = keys.count();
    while (!keys.isEmpty()) {
        remove(keys.pop());
    }
    return n;
}
