// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MFAWIDGET_H
#define MFAWIDGET_H

#include "auth_widget.h"

#include <QWidget>

class MFAWidget : public AuthWidget
{
    Q_OBJECT

public:
    explicit MFAWidget(QWidget *parent = nullptr);

    void setModel(const SessionBaseModel *model) override;
    void setAuthType(const int type) override;
    void setAuthState(const int type, const int state, const QString &message) override;
    void autoUnlock();
    int getTopSpacing() const override;

private:
    void initUI();
    void initConnections();

    void initPasswdAuth();
    void initFingerprintAuth();
    void initUKeyAuth();
    void initFaceAuth();
    void initIrisAuth();

    void checkAuthResult(const int type, const int state) override;

    void updateFocusPosition();

private:
    int m_index;
    QVBoxLayout *m_mainLayout;
    QList<QWidget *> m_widgetList;
};

#endif // MFAWIDGET_H
