/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PRINTER_BEHAVIOR_H
#define PRINTER_BEHAVIOR_H

#include "PrinterPage.h"
#include <QWidget>

namespace Ui
{
class PrinterBehavior;
}

class QWidget;

class PrinterBehavior : public PrinterPage
{
    Q_OBJECT
public:
    explicit PrinterBehavior(const QString &destName, bool isClass, QWidget *parent = nullptr);
    ~PrinterBehavior() override;

    void setValues(const KCupsPrinter &printer);
    void setRemote(bool remote) override;
    bool hasChanges() override;

    QStringList neededValues() const override;
    void save() override;

private slots:
    void currentIndexChangedCB(int index);
    void userListChanged();

private:
    QString errorPolicyString(const QString &policy) const;
    QString operationPolicyString(const QString &policy) const;
    QString jobSheetsString(const QString &policy) const;

    Ui::PrinterBehavior *const ui;
    QString m_destName;
    bool m_isClass;
    QVariantMap m_changedValues;
    int m_changes = 0;
};

#endif
