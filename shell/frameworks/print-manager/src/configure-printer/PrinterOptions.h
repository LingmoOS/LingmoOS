/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PRINTER_OPTIONS_H
#define PRINTER_OPTIONS_H

#include "PrinterPage.h"
#include <cups/ppd.h>

namespace Ui
{
class PrinterOptions;
}

class QAbstractButton;

class PrinterOptions : public PrinterPage
{
    Q_OBJECT
public:
    explicit PrinterOptions(const QString &destName, bool isClass, bool isRemote, QWidget *parent = nullptr);
    ~PrinterOptions() override;

    bool hasChanges() override;

    QString currentMake() const;
    QString currentMakeAndModel() const;
    void reloadPPD();

    void save() override;

private slots:
    void autoConfigureClicked();
    void currentIndexChangedCB(int index);
    void radioBtClicked(QAbstractButton *button);

private:
    QWidget *pickBoolean(ppd_option_t *option, const QString &keyword, QWidget *parent) const;
    QWidget *pickMany(ppd_option_t *option, const QString &keyword, QWidget *parent) const;
    QWidget *pickOne(ppd_option_t *option, const QString &keyword, QWidget *parent) const;
    const char *getVariable(const char *name) const;
    char *get_option_value(ppd_file_t *ppd, const char *name, char *buffer, size_t bufsize) const;
    static double get_points(double number, const char *uval);

    void createGroups();

    Ui::PrinterOptions *const ui;
    QString m_destName;
    bool m_isClass;
    bool m_isRemote;
    QString m_filename;
    ppd_file_t *m_ppd = nullptr;
    int m_changes = 0;
    const char *m_codec = nullptr;
    QHash<QString, QObject *> m_customValues;
    QString m_make, m_makeAndModel;
};

#endif
