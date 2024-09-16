// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef NETSECRETWIDGET_H
#define NETSECRETWIDGET_H

#include <QMap>
#include <QWidget>
#include <QPointer>

namespace dde {
namespace network {

class NetSecretWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NetSecretWidget(QWidget *parent = nullptr);
    ~NetSecretWidget() override;

Q_SIGNALS:
    void submit(const QVariantMap &data);
    void requestCheckInput(const QVariantMap &data);

public Q_SLOTS:
    void initUI(const QVariantMap &param);
    void setFocusToEdit();
    void showError(const QVariantMap &param);
    void updateInputValid(const QVariantMap &param);
    void onCancel();
    void onConnect();

    void onTextChanged();
    void onReturnPressed();

private:
    QWidget *m_connectButton;
    const QMap<QString, QString> m_secretKeyStrMap;
    const QMap<QString, QString> m_propMap;
    QPointer<QWidget> m_passwordWidget;
};

} // namespace network
} // namespace dde

#endif // NETSECRETWIDGET_H
