// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DCC_BLUETOOTH_PINCODEDIALOG_H
#define DCC_BLUETOOTH_PINCODEDIALOG_H

#include <DDialog>

DWIDGET_USE_NAMESPACE

class LargeLabel;
class PinCodeDialog : public DDialog
{
    Q_OBJECT
public:
    explicit PinCodeDialog();
    explicit PinCodeDialog(const QString &pinCode,  const QString &devicepath, const QString &starttime,  const bool &cancelable = true);
    ~PinCodeDialog();

    QString pinCode() const;
private Q_SLOTS:
    void HandleBlutoothPower(const QString &message);
private:
    LargeLabel *m_pinCodeLabel;
    LargeLabel *m_titileLabel;
};

#endif // DCC_BLUETOOTH_PINCODEDIALOG_H
