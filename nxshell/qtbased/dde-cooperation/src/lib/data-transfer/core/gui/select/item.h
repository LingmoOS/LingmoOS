#ifndef SELECTITEM_H
#define SELECTITEM_H

#include <QCheckBox>
#include <QItemDelegate>
#include <QPainter>
#include <QStandardItemModel>
#include <QFrame>
#include <QListView>
#include <QModelIndex>
#include <QMouseEvent>
#include <QPushButton>
#include <QSortFilterProxyModel>
class QSortFilterProxyModel;

enum ListSelectionState { selectall = 0, selecthalf, unselected };
class SelectAllButton : public QFrame
{
    Q_OBJECT
public:
    SelectAllButton(QWidget *parent = nullptr);
    ~SelectAllButton();
    void changeState(ListSelectionState state);

protected:
    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
signals:
    void selectAll();

private:
    QRectF iconPosSize{ 1, 1, 18, 18 };
    qreal iconRadius{ 4 };
    ListSelectionState curState{ ListSelectionState::unselected };
};

class SortButton : public QPushButton
{
    Q_OBJECT
public:
    SortButton(QWidget *parent = nullptr);
    ~SortButton();

private:
    bool flag{ true };
signals:
    void sort();

protected:
    void mousePressEvent(QMouseEvent *event) override;
};
class ItemTitlebar : public QFrame
{
    Q_OBJECT
public:
    ItemTitlebar(const QString &label1_, const QString &label2_, const qreal &label1LeftMargin_,
                 const qreal &label2LeftMargin_, const QRectF &iconPosSize_,
                 const qreal &iconRadius_, QWidget *parent = nullptr);
    ItemTitlebar(QWidget *parent = nullptr);
    ~ItemTitlebar();

    void setLabel1(const QString &newLabel1);

    void setLabel1LeftMargin(qreal newLabel1LeftMargin);

    void setLabel2(const QString &newLabel2);

    void setLabel2LeftMargin(qreal newLabel2LeftMargin);

    void setIconPosSize(const QRectF &newIconPosSize);

    void setIconRadius(qreal newIconRadius);

    void setType(bool value);

    SortButton *getSortButton1() const;

    SortButton *getSortButton2() const;

protected:
    void paintEvent(QPaintEvent *event) override;

public slots:
    void updateSelectAllButState(ListSelectionState selectState);
signals:
    void selectAll();
    void sort();

private:
    void initUI();

private:
    QString label1{ "label1" };
    qreal label1LeftMargin{ 50 };
    QString label2{ "label2" };
    qreal label2LeftMargin{ 360 };
    QRectF iconPosSize{ 10, 10, 14, 14 };
    qreal iconRadius{ 3 };
    bool type{ true };

    SelectAllButton *selectAllButton{ nullptr };
    SortButton *sortButton1{ nullptr };
    SortButton *sortButton2{ nullptr };
    int allTableEntries{ 0 };
};

class ItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    ItemDelegate();
    ItemDelegate(const qreal &filenameTextLeftMargin_, const qreal &filenameTextMaxLen_,
                 const qreal &remarkTextLeftMargin_, const qreal &remarkTextMaxLen_,
                 const qreal &backgroundColorLeftMargin_, const QPoint &iconPos_,
                 const QPoint &checkBoxPos_);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void setRemarkTextLeftMargin(qreal newRemarkTextLeftMargin);

    void setRemarkTextMaxLen(qreal newRemarkTextMaxLen);

    void setFilenameTextLeftMargin(qreal newFilenameTextLeftMargin);

    void setFilenameTextMaxLen(qreal newFilenameTextMaxLen);

    void setBackgroundColorLeftMargin(qreal newBackgroundColorLeftMargin);

    void setIconPos(QPoint newIconPos);

    void setCheckBoxPos(QPoint newCheckBoxPos);

private:
    void paintIcon(QPainter *painter, const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;
    void paintBackground(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const;
    void paintText(QPainter *painter, const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;
    void paintCheckbox(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

private:
    qreal remarkTextLeftMargin{ 379 };
    qreal remarkTextMaxLen{ 100 };

    qreal filenameTextLeftMargin{ 99 };
    qreal filenameTextMaxLen{ 250 };

    qreal backgroundColorLeftMargin{ 50 };

    QPoint iconPos{ 65, 6 };
    QPoint checkBoxPos{ 12, 9 };
    float opacity{ 0.6 };
};

class SaveItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    SaveItemDelegate();
    ~SaveItemDelegate();
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void paintIcon(QPainter *painter, const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;
    void paintBackground(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const;
    void paintText(QPainter *painter, const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;
    void paintCheckbox(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

private:
    qreal remarkTextLeftMargin{ 200 };
    qreal remarkTextRightMargin{ 30 };
    qreal remarkTextMaxLen{ 100 };

    qreal filenameTextLeftMargin{ 52 };
    qreal filenameTextMaxLen{ 250 };

    qreal backgroundColorLeftMargin{ 0 };

    QPoint iconPos{ 14, 10 };
    QPoint checkBoxPos{ 0, 0 };

    float opacity{ 0.6 };
};

class SidebarItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    SidebarItemDelegate();
    ~SidebarItemDelegate();
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void paintText(QPainter *painter, const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;
    void paintCheckbox(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;
    void paintBackground(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;
};

class SortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    SortProxyModel(QObject *parent = nullptr);
    ~SortProxyModel();
    void setMode(bool mode);
    void setFlag(bool flag);

private:
    bool sortModel{ true };
    bool flagDisplayrole{ true };

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
};

class SelectListView : public QListView
{
    Q_OBJECT
public:
    SelectListView(QFrame *parent = nullptr);
    ~SelectListView();
    void setAllSize(int size);
    QStandardItemModel *getModel();
    void setSortRole(bool flag);

private:
    int curSelectItemNum{ 0 };
    int allSize{ -1 };
    ListSelectionState curSelectState{ ListSelectionState::unselected };
    bool ascendingOrder{ false };
    SortProxyModel *proxyModel{ nullptr };
    bool flag_DisplayRole{ true };
public slots:
    void selectorDelAllItem();
    void updateCurSelectItem(QStandardItem *item);
    void sortListview();

signals:
    void currentSelectState(ListSelectionState selectState);
};

#endif
