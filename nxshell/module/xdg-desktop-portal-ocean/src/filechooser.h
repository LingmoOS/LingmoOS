// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>
#include <qobjectdefs.h>

class FileChooserPortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.FileChooser")

public:
    typedef struct {
        uint type;
        QString filterString;
    } Filter;
    typedef QList<Filter> Filters;

    typedef struct {
        QString userVisibleName;
        Filters filters;
    } FilterList;
    typedef QList<FilterList> FilterListList;

    typedef struct {
        QString id;
        QString value;
    } Choice;
    typedef QList<Choice> Choices;

    typedef struct {
        QString id;
        QString label;
        Choices choices;
        QString initialChoiceId;
    } Option;
    typedef QList<Option> OptionList;

    explicit FileChooserPortal(QObject *parent);
    ~FileChooserPortal() = default;

public slots:
    uint OpenFile(const QDBusObjectPath &handle,
                  const QString &app_id,
                  const QString &parent_window,
                  const QString &title,
                  const QVariantMap &options,
                  QVariantMap &results);
    uint SaveFile(const QDBusObjectPath &handle,
                  const QString &app_id,
                  const QString &parent_window,
                  const QString &title,
                  const QVariantMap &options,
                  QVariantMap &results);

    uint SaveFiles(const QDBusObjectPath &handle,
                  const QString &app_id,
                  const QString &parent_window,
                  const QString &title,
                  const QVariantMap &options,
                  QVariantMap &results);

private:
    QString parseAcceptLabel(const QVariantMap &options);
    void parseFilters(const QVariantMap &options,
                                           QStringList &nameFilters,
                                           QStringList &mimeTypeFilters,
                                           QMap<QString, FilterList> &allFilters,
                                           QString &selectedMimeTypeFilter);
};
