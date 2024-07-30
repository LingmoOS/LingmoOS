/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include <QAbstractListModel>
#include <QQmlParserStatus>
#include <qqmlregistration.h>

#include <KNSCore/Entry>

#include <memory>

class PageDataObject;

class PagesModel : public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    QML_ELEMENT

public:
    enum Roles {
        TitleRole = Qt::UserRole + 1,
        DataRole,
        IconRole,
        FileNameRole,
        HiddenRole,
        FilesWriteableRole,
    };
    Q_ENUM(Roles)

    enum FilesWriteableStates {
        NotWriteable,
        AllWriteable,
        LocalChanges,
    };
    Q_ENUM(FilesWriteableStates)

    explicit PagesModel(QObject *parent = nullptr);

    Q_PROPERTY(QStringList pageOrder READ pageOrder WRITE setPageOrder NOTIFY pageOrderChanged)
    Q_PROPERTY(QStringList hiddenPages READ hiddenPages WRITE setHiddenPages NOTIFY hiddenPagesChanged)

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    void sort(int column = 0, Qt::SortOrder order = Qt::AscendingOrder) override;

    void classBegin() override;
    void componentComplete() override;

    Q_INVOKABLE PageDataObject *addPage(const QString &fileName, const QVariantMap &properties = QVariantMap{});
    Q_INVOKABLE PageDataObject *importPage(const QUrl &file);
    Q_INVOKABLE void removeLocalPageFiles(const QString &fileName);
    Q_INVOKABLE void ghnsEntryStatusChanged(const KNSCore::Entry &entry);

    QStringList pageOrder() const;
    void setPageOrder(const QStringList &pageOrder);

    QStringList hiddenPages() const;
    void setHiddenPages(const QStringList &hiddenPages);

Q_SIGNALS:
    void pageOrderChanged();
    void hiddenPagesChanged();

private:
    QList<PageDataObject *> m_pages;
    QStringList m_pageOrder;
    QStringList m_hiddenPages;
    QHash<QString, FilesWriteableStates> m_writeableCache;
};
