#ifndef DETAILLISTTABLE_H
#define DETAILLISTTABLE_H

#include <QTableView>
#include <QStandardItemModel>
#include <QScroller>
#include <QHeaderView>

#include <QStyledItemDelegate>

class BaseDetailItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit BaseDetailItemDelegate(QObject *parent = nullptr);

    virtual ~BaseDetailItemDelegate();

protected:
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class DetailListTable : public QTableView
{
    Q_OBJECT
public:
    DetailListTable(QAbstractTableModel *dataModel, QStyledItemDelegate *baseStyle = nullptr, QWidget *parent = nullptr);
    ~DetailListTable();

public slots:
    void onModelUpdate();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QFont m_font;
    QAbstractTableModel *m_model;
    QStyledItemDelegate *mBaseStyleDelegate;
};

#endif // DETAILLISTTABLE_H
