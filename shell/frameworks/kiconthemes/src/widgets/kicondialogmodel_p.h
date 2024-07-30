/*
    SPDX-FileCopyrightText: 2021 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KICONDIALOGMODEL_P_H
#define KICONDIALOGMODEL_P_H

#include <QAbstractListModel>
#include <QList>
#include <QPixmap>
#include <QSize>
#include <QString>
#include <QStringList>

class KIconLoader;

struct KIconDialogModelData {
    QString name;
    QString path;
    QPixmap pixmap;
};
Q_DECLARE_TYPEINFO(KIconDialogModelData, Q_RELOCATABLE_TYPE);

class KIconDialogModel : public QAbstractListModel
{
    Q_OBJECT

public:
    KIconDialogModel(KIconLoader *loader, QObject *parent);
    ~KIconDialogModel() override;

    enum Roles { PathRole = Qt::UserRole };

    qreal devicePixelRatio() const;
    void setDevicePixelRatio(qreal dpr);

    QSize iconSize() const;
    void setIconSize(const QSize &iconSize);

    static QLatin1String symbolicSuffix();
    bool hasSymbolicIcon() const;

    void load(const QStringList &paths);

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

Q_SIGNALS:
    void hasSymbolicIconChanged(bool hasSymbolicIcon);

private:
    void loadPixmap(const QModelIndex &index);

    QList<KIconDialogModelData> m_data;

    KIconLoader *m_loader;
    qreal m_dpr = 1;
    QSize m_iconSize;
    bool m_hasSymbolicIcon = false;
};

#endif // KICONDIALOGMODEL_P_H
