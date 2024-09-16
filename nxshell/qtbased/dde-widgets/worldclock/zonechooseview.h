// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <DDialog>
#include <DListView>
#include <QEvent>
#include <QSortFilterProxyModel>

DWIDGET_BEGIN_NAMESPACE
class DSearchEdit;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

namespace dwclock {
class TimezoneListModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Timezone{
        CityName = Qt::UserRole + 1,
        ZoneName
    };

    explicit TimezoneListModel(QObject *parent = nullptr);
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;

private:
    mutable QStringList m_data;
};

class ZoneProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit ZoneProxyModel(QObject *parent = nullptr);

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

class ZoneSectionProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit ZoneSectionProxyModel(QObject *parent = nullptr);

    void setSectionKey(const QString &sectionKey);

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    QString m_sectionKey;
};

class ZoneSectionView : public DListView {
    Q_OBJECT
public:
    explicit ZoneSectionView(QWidget *parent = nullptr);

protected:
    virtual bool event(QEvent *event) override;
};

class ZoneSection : public QWidget {
    Q_OBJECT
public:
    explicit ZoneSection(const QString &sectionKey, QWidget *parent = nullptr);

    void setModel(QAbstractItemModel *model);
    int rowCount() const;
    void clearCurrentIndex();
    QString currentZone() const;

Q_SIGNALS:
    void currentChanged(const QModelIndex &previous);

private:
    QString m_sectionKey;
    ZoneSectionView *m_zones = nullptr;
};

class ZoneChooseView : public DDialog {
    Q_OBJECT
public:
    explicit ZoneChooseView(QWidget *parent = nullptr);

    QString currentZone() const;

private Q_SLOTS:
    void onSearchTextChanged(const QString &text);
    void onCurrentChanged(const QModelIndex &previous);

private:
    QWidget *fillZones();

    DSearchEdit *m_search = nullptr;
    QSortFilterProxyModel *m_proxyModel = nullptr;
    QList<ZoneSection *> m_sections;
    ZoneSection *m_currentSection = nullptr;
    QAbstractButton *m_okBtn = nullptr;
};
}
