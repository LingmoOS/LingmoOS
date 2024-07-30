// SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef PASSWORDFILTERMODEL_H_
#define PASSWORDFILTERMODEL_H_

#include <QFuture>
#include <QSortFilterProxyModel>
#include <QTimer>
#include <QVector>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class QStringRef;
#endif
class KDescendantsProxyModel;

namespace PlasmaPass
{
class PasswordFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QString passwordFilter READ passwordFilter WRITE setPasswordFilter NOTIFY passwordFilterChanged)
public:
    explicit PasswordFilterModel(QObject *parent = nullptr);

    void setSourceModel(QAbstractItemModel *sourceModel) override;

    QString passwordFilter() const;
    void setPasswordFilter(const QString &filter);

    QVariant data(const QModelIndex &index, int role) const override;

Q_SIGNALS:
    void passwordFilterChanged();

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    struct PathFilter {
        using result_type = std::pair<QModelIndex, int>;

        explicit PathFilter() = default;
        PathFilter(QString filter);

        PathFilter(const PathFilter &);
        PathFilter(PathFilter &&) noexcept;
        PathFilter &operator=(const PathFilter &);
        PathFilter &operator=(PathFilter &&) noexcept;

        result_type operator()(const QModelIndex &index) const;

        QString filter;

    private:
        void updateParts();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QVector<QStringRef> mParts;
#else
        QVector<QStringView> mParts;
#endif
    };

    void delayedUpdateFilter();

    KDescendantsProxyModel *mFlatModel = nullptr;
    PathFilter mFilter;
    mutable QHash<QModelIndex, int> mSortingLookup;
    QTimer mUpdateTimer;
    QFuture<QHash<QModelIndex, int>> mFuture;
};

}

#endif
