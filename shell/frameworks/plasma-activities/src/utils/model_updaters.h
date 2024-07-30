/*
    SPDX-FileCopyrightText: 2012 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KACTIVITIES_MODEL_UPDATERS_H
#define KACTIVITIES_MODEL_UPDATERS_H

// -----------------------------------------
// RAII classes for model updates ----------
// -----------------------------------------

// clang-format off
#define DECLARE_RAII_MODEL_UPDATERS(Class)                                     \
    template <typename T> class _model_reset {                                 \
        T *model;                                                              \
                                                                               \
    public:                                                                    \
        _model_reset(T *m) : model(m)                                          \
        {                                                                      \
            model->beginResetModel();                                          \
        }                                                                      \
        ~_model_reset()                                                        \
        {                                                                      \
            model->endResetModel();                                            \
        }                                                                      \
    };                                                                         \
    template <typename T> class _model_insert {                                \
        T *model;                                                              \
                                                                               \
    public:                                                                    \
        _model_insert(T *m, const QModelIndex &parent, int first, int last)    \
            : model(m)                                                         \
        {                                                                      \
            model->beginInsertRows(parent, first, last);                       \
        }                                                                      \
        ~_model_insert()                                                       \
        {                                                                      \
            model->endInsertRows();                                            \
        }                                                                      \
    };                                                                         \
    template <typename T> class _model_remove {                                \
        T *model;                                                              \
                                                                               \
    public:                                                                    \
        _model_remove(T *m, const QModelIndex &parent, int first, int last)    \
            : model(m)                                                         \
        {                                                                      \
            model->beginRemoveRows(parent, first, last);                       \
        }                                                                      \
        ~_model_remove()                                                       \
        {                                                                      \
            model->endRemoveRows();                                            \
        }                                                                      \
    };                                                                         \
    typedef _model_reset<Class> model_reset;                                   \
    typedef _model_remove<Class> model_remove;                                 \
    typedef _model_insert<Class> model_insert;

// -----------------------------------------

#endif // KACTIVITIES_MODEL_UPDATERS_H

