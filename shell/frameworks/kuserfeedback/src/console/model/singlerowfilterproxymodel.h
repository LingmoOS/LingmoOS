/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_SINGLEROWFILTERPROXYMODEL_H
#define KUSERFEEDBACK_CONSOLE_SINGLEROWFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

namespace KUserFeedback {
namespace Console {

/** Keep a single row from the source model. */
class SingleRowFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit SingleRowFilterProxyModel(QObject *parent = nullptr);
    ~SingleRowFilterProxyModel() override;

    void setRow(int row);
    bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const override;

private:
    int m_row = 0;
};

}}

#endif // KUSERFEEDBACK_CONSOLE_SINGLEROWFILTERPROXYMODEL_H
