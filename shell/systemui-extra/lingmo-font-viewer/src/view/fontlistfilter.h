#ifndef FONTLISTFILTER_H
#define FONTLISTFILTER_H

#include <QSortFilterProxyModel>

class FontListFilter: public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit FontListFilter(QSortFilterProxyModel *parent = nullptr);
    ~FontListFilter();

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;
    bool filterAcceptsRow(int source_row , const QModelIndex &source_parent) const;
    bool compareString(QString rightStr, QString leftStr) const;
};

#endif // FONTLISTFILTER_H
