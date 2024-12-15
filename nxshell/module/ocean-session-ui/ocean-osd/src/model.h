// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MODEL_H
#define MODEL_H

#include <QAbstractListModel>

class AbstractOSDProvider;
/*!
 * \~chinese \class ListView
 * \~chinese \brief osd的model类
 */
class Model : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit Model(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

    void setProvider(AbstractOSDProvider *provider);
    AbstractOSDProvider *provider();

private:
    AbstractOSDProvider *m_provider;
};

#endif // MODEL_H
