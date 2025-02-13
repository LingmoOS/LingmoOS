#ifndef BASEMODEL_H
#define BASEMODEL_H

#include <QPoint>
#include <QAbstractListModel>
#include <QList>

#if defined(QT_TESTLIB_LIB)
#  define BaseModel_ASSERT(x)
#else
#  define BaseModel_ASSERT(x) Q_ASSERT(x)
#endif

namespace Internal {

template <typename T>
struct QBaseModelListGuard
{
};

class BaseModelPrivate;
class BaseModel : public QAbstractListModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(BaseModel)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(bool countEnabled READ isCountEnabled NOTIFY countEnabledChanged)

public:
    explicit BaseModel(QObject *parent = nullptr);
    virtual ~BaseModel();
protected:
    QScopedPointer<BaseModelPrivate> d_ptr;

public:
    enum ModelDataRoles {
        ModelDataRole = Qt::UserRole + 1,
    };
    QHash<int, QByteArray> roleNames() const override;

signals:
    void countChanged();
protected:
    void _q_resetCount();

signals:
    void countEnabledChanged();
public:
    bool isCountEnabled() const;
public:
    void setCountEnabled(bool y);
};

} // namespace Internal

template <typename T>
class BaseModel : public Internal::BaseModel, public QList<T>
{
public:
    BaseModel(const QList<T> &l, QObject *parent = nullptr)
        : Internal::BaseModel(parent), QList<T>(l) { }
    explicit BaseModel(QObject *parent = nullptr)
        : Internal::BaseModel(parent) { }

public:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

public:
    T value(const QModelIndex &index) const;
    T value(const QModelIndex &index, const T &defaultValue) const;

    T value(int i) const;
    T value(int i, const T &defaultValue) const;

public:
    void append(const QList<T> &l);

    void prepend(const T &t);
    void append(const T &t);

    void push_front(const T &t);
    void push_back(const T &t);

    void replace(int i, const T &t);
    void insert(int i, const T &t);

    bool removeOne(const T &t);
    int removeAll(const T &t);

    void pop_front();
    void pop_back();

    void removeAt(int i);
    void removeFirst();
    void removeLast();

    T takeAt(int i);
    T takeFirst();
    T takeLast();

    void swap(int i, int j);
    void swap(QList<T> &list);
    void move(int from, int to);

    void clear();

public: // Extra methods
    void deleteAll();

public: // Disabled stl methods
    typedef typename QList<T>::iterator         Iterator;
    typedef typename QList<T>::reverse_iterator ReverseIterator;

    Iterator begin(); // Not Implemented
    Iterator end(); // Not Implemented

    ReverseIterator rbegin(); // Not Implemented
    ReverseIterator rend(); // Not Implemented

    Iterator insert(Iterator before, const T &t); // Not Implemented
    Iterator erase(Iterator pos); // No Implemented
    Iterator erase(Iterator begin, Iterator end); // Not Implemented

public:
    BaseModel<T> &operator=(const QList<T> &l);
    inline QList<T> operator+(const QList<T> &others) const
    { QList<T> l = *this; l.append(others); return l; }

    inline bool operator==(const QList<T> &other) const
    { return QList<T>::operator==(other); }
    inline bool operator!=(const QList<T> &other) const
    { return QList<T>::operator!=(other); }

    inline BaseModel<T> &operator+=(const QList<T> &l)
    { append(l); return *this; }
    inline BaseModel<T> &operator<<(const QList<T> &l)
    { append(l); return *this; }

    inline BaseModel<T> &operator+=(const T &t)
    { append(t); return *this; }
    inline BaseModel<T> &operator<<(const T &t)
    { append(t); return *this; }

public:
    inline const QList<T> &ref() const { return *this; }
};

template <typename T>
int BaseModel<T>::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : QList<T>::count();
}

template <typename T>
QVariant BaseModel<T>::data(const QModelIndex &index, int role) const
{
    if ((ModelDataRole != role)
            || !index.isValid()
            || (index.model() != this)
            || (index.row() >= QList<T>::count())) {
        return QVariant();
    }

    return QVariant::fromValue(QList<T>::at(index.row()));
}

template<typename T>
QModelIndex BaseModel<T>::index(int row, int column, const QModelIndex &parent) const
{
    if ((row < 0)
            || (column != 0)
            || (row >= QList<T>::count())
            || parent.isValid()) {
        return QModelIndex();
    }

    return createIndex(row, 0);
}

template<typename T>
T BaseModel<T>::value(const QModelIndex &index) const
{
    BaseModel_ASSERT(index.model() == this);

    return QList<T>::value(index.row());
}

template<typename T>
T BaseModel<T>::value(const QModelIndex &index, const T &defaultValue) const
{
    BaseModel_ASSERT(index.model() == this);

    return QList<T>::value(index.row(), defaultValue);
}

template<typename T>
T BaseModel<T>::value(int i) const
{
    return QList<T>::value(i);
}

template<typename T>
T BaseModel<T>::value(int i, const T &defaultValue) const
{
    return QList<T>::value(i, defaultValue);
}

template <typename T>
void BaseModel<T>::append(const QList<T> &l)
{
    if (l.isEmpty())
        return;

    int f = QList<T>::count();
    int t = f + l.count() - 1;
    beginInsertRows(QModelIndex(), f, t);
    QList<T>::append(l);
    endInsertRows();
    _q_resetCount();
}

template <typename T>
void BaseModel<T>::prepend(const T &t)
{
    beginInsertRows(QModelIndex(), 0, 0);
    QList<T>::prepend(t);
    endInsertRows();
    _q_resetCount();
}

template <typename T>
void BaseModel<T>::append(const T &t)
{
    int r = QList<T>::count();
    beginInsertRows(QModelIndex(), r, r);
    QList<T>::append(t);
    endInsertRows();
    _q_resetCount();
}

template <typename T>
void BaseModel<T>::push_front(const T &t)
{
    beginInsertRows(QModelIndex(), 0, 0);
    QList<T>::push_front(t);
    endInsertRows();
    _q_resetCount();
}

template <typename T>
void BaseModel<T>::push_back(const T &t)
{
    int r = QList<T>::count();
    beginInsertRows(QModelIndex(), r, r);
    QList<T>::push_back(t);
    endInsertRows();
    _q_resetCount();
}

template <typename T>
void BaseModel<T>::replace(int i, const T &t)
{
    BaseModel_ASSERT(i >= 0);
    BaseModel_ASSERT(i < QList<T>::count());

    QModelIndex x = index(i, 0);
    if (!x.isValid()) {
        return;
    }

    QList<T>::replace(i, t); emit dataChanged(x, x);
}

template <typename T>
void BaseModel<T>::insert(int i, const T &t)
{
    BaseModel_ASSERT(i >= 0);
    BaseModel_ASSERT(i <= QList<T>::count());

    beginInsertRows(QModelIndex(), i, i);
    QList<T>::insert(i, t);
    endInsertRows();
    _q_resetCount();
}

template <typename T>
bool BaseModel<T>::removeOne(const T &t)
{
    typename QList<T>::iterator p = QList<T>::begin();
    typename QList<T>::iterator end = QList<T>::end();

    for (int i = 0; p != end; ++p, ++i) {
        if (*p == t) {
            beginRemoveRows(QModelIndex(), i, i);
            QList<T>::erase(p);
            endRemoveRows();
            _q_resetCount();

            return true;
        }
    }

    return false;
}

template <typename T>
int BaseModel<T>::removeAll(const T &t)
{
    typename QList<T>::iterator null;
    typename QList<T>::iterator f = null;
    typename QList<T>::iterator l = null;
    typename QList<T>::iterator p = QList<T>::begin();
    typename QList<T>::iterator end = QList<T>::end();

    int c = 0;
    int fpos = 0;
    int lpos = 0;

    for (int i = 0; p != end; ++p, ++i) {
        if (t == *p) {
            if (f == null) {
                fpos = i;
                f = p;
            }
            lpos = i;
            l = p;

        } else if (f != null) {
            beginRemoveRows(QModelIndex(), fpos - c, lpos - c);
            QList<T>::erase(f, ++l);
            endRemoveRows();

            c += lpos - fpos + 1;

            f = null;
            l = null;
        }
    }

    if (f != null) {
        beginRemoveRows(QModelIndex(), fpos - c, lpos - c);
        QList<T>::erase(f, ++l);
        endRemoveRows();

        c += lpos - fpos + 1;
    }

    _q_resetCount();

    return c;
}

template <typename T>
void BaseModel<T>::pop_front()
{
    BaseModel_ASSERT(QList<T>::count() > 0);

    beginRemoveRows(QModelIndex(), 0, 0);
    QList<T>::pop_front();
    endRemoveRows();
    _q_resetCount();
}

template <typename T>
void BaseModel<T>::pop_back()
{
    BaseModel_ASSERT(QList<T>::count() > 0);

    int r = QList<T>::count() - 1;
    beginRemoveRows(QModelIndex(), r, r);
    QList<T>::pop_back();
    endRemoveRows();
    _q_resetCount();
}

template <typename T>
void BaseModel<T>::removeAt(int i)
{
    BaseModel_ASSERT(i >= 0);
    BaseModel_ASSERT(i < QList<T>::count());

    beginRemoveRows(QModelIndex(), i, i);
    QList<T>::removeAt(i);
    endRemoveRows();
    _q_resetCount();
}

template <typename T>
void BaseModel<T>::removeFirst()
{
    BaseModel_ASSERT(QList<T>::count() > 0);

    beginRemoveRows(QModelIndex(), 0, 0);
    QList<T>::removeFirst();
    endRemoveRows();
    _q_resetCount();
}

template <typename T>
void BaseModel<T>::removeLast()
{
    BaseModel_ASSERT(QList<T>::count() > 0);

    int r = QList<T>::count() - 1;
    beginRemoveRows(QModelIndex(), r, r);
    QList<T>::removeLast();
    endRemoveRows();
    _q_resetCount();
}

template <typename T>
T BaseModel<T>::takeAt(int i)
{
    BaseModel_ASSERT(i >= 0);
    BaseModel_ASSERT(i < QList<T>::count());

    beginRemoveRows(QModelIndex(), i, i);
    T t = QList<T>::takeAt(i);
    endRemoveRows();
    _q_resetCount();

    return t;
}

template <typename T>
T BaseModel<T>::takeFirst()
{
    BaseModel_ASSERT(QList<T>::count() > 0);

    beginRemoveRows(QModelIndex(), 0, 0);
    T t = QList<T>::takeFirst();
    endRemoveRows();
    _q_resetCount();

    return t;
}

template <typename T>
T BaseModel<T>::takeLast()
{
    BaseModel_ASSERT(QList<T>::count() > 0);

    int r = QList<T>::count() - 1;
    beginRemoveRows(QModelIndex(), r, r);
    T t = QList<T>::takeLast();
    endRemoveRows();
    _q_resetCount();

    return t;
}

template <typename T>
void BaseModel<T>::swap(int i, int j)
{
    if (i == j) {
        return;
    }

    BaseModel_ASSERT(i >= 0);
    BaseModel_ASSERT(i < QList<T>::count());

    BaseModel_ASSERT(j >= 0);
    BaseModel_ASSERT(j < QList<T>::count());

    QList<T>::swap(i, j);

    QModelIndex ii = index(i, 0); emit dataChanged(ii, ii);
    QModelIndex ji = index(j, 0); emit dataChanged(ji, ji);
}

template <typename T>
void BaseModel<T>::swap(QList<T> &l)
{
    /*
    if (l.d == this->d) {
        return *this;
    }
    */

    beginResetModel();
    QList<T>::swap(l);
    endResetModel();
    _q_resetCount();
}

template <typename T>
void BaseModel<T>::move(int from, int to)
{
    if (from == to) {
        return;
    }

    BaseModel_ASSERT(from >= 0);
    BaseModel_ASSERT(from < QList<T>::count());

    BaseModel_ASSERT(to >= 0);
    BaseModel_ASSERT(to < QList<T>::count());

    QModelIndex p;
    int t = (from < to) ? (to + 1) : to;
    if (beginMoveRows(p, from, from, p, t)) {
        QList<T>::move(from, to);
        endMoveRows();
    }
}

template<typename T>
void BaseModel<T>::clear()
{
    if (QList<T>::isEmpty()) {
        return;
    }

    beginResetModel();
    QList<T>::clear();
    endResetModel();
    _q_resetCount();
}

template<typename T>
void BaseModel<T>::deleteAll()
{
    if (QList<T>::isEmpty()) {
        return;
    }

    beginResetModel();
    qDeleteAll(ref());
    QList<T>::clear();
    endResetModel();
    _q_resetCount();
}

template<typename T>
BaseModel<T> &BaseModel<T>::operator=(const QList<T> &l)
{
    /*
    if (l.d == this->d) {
        return *this;
    }
    */

    beginResetModel();
    QList<T> tmp(l);
    tmp.swap(*this);
    endResetModel();
    _q_resetCount();

    return *this;
}

#endif // BASEMODEL_H
