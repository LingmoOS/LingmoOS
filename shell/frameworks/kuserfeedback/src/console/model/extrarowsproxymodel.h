/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_EXTRAROWSPROXYMODEL_H
#define KUSERFEEDBACK_CONSOLE_EXTRAROWSPROXYMODEL_H

#include <QIdentityProxyModel>

namespace KUserFeedback {
namespace Console {

class ExtraRowsProxyModel : public QIdentityProxyModel
{
    Q_OBJECT
public:
    explicit ExtraRowsProxyModel(QObject *parent = nullptr);
    ~ExtraRowsProxyModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex & child) const override;

protected:
    virtual QVariant extraData(int row, int column, int role) const;

private:
    int m_extraRowCount = 1;
};

}}

#endif // KUSERFEEDBACK_CONSOLE_EXTRAROWSPROXYMODEL_H
