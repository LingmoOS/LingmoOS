#include "basemodel.h"
#include "basemodel_p.h"

namespace Internal {

// class BaseModel

BaseModel::BaseModel(QObject *parent)
    : QAbstractListModel(parent)
    , d_ptr(new BaseModelPrivate())
{
    d_ptr->q_ptr = this;
}

BaseModel::~BaseModel()
{
}

QHash<int, QByteArray> BaseModel::roleNames() const
{
#ifdef Q_COMPILER_INITIALIZER_LISTS
    QHash<int, QByteArray> roleNames { { BaseModel::ModelDataRole, "modelData" } };
#else
    QHash<int, QByteArray> roleNames;
    roleNames[BaseModel::ModelDataRole] = "modelData";
#endif // Q_COMPILER_INITIALIZER_LISTS

    return roleNames;
}

void BaseModel::_q_resetCount()
{
    Q_D(BaseModel);

    if (d->countEnabled) {
        int count = rowCount();
        if (count != d->count) {
            d->count = count;
            emit countChanged();
        }
    }
}

bool BaseModel::isCountEnabled() const
{
    Q_D(const BaseModel);

    return d->countEnabled;
}

void BaseModel::setCountEnabled(bool y)
{
    Q_D(BaseModel);

    if (y != d->countEnabled) {
        d->countEnabled = y;
        if (!d->countEnabled) {
            d->count = -1;
        }
        emit countEnabledChanged();
    }
}

// class BaseModelPrivate

BaseModelPrivate::BaseModelPrivate()
    : q_ptr(nullptr)
    , countEnabled(false)
    , count(-1)
{
}

BaseModelPrivate::~BaseModelPrivate()
{
}

} // namespace Internal

