/*
 * Peony-Qt
 *
 * Copyright (C) 2023, KylinSoft Information Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef UDFFORMATDIALOG_H
#define UDFFORMATDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QProgressBar>
#include <QGridLayout>
#include <QCloseEvent>
#include <QMutex>

#include "explor-core_global.h"

class DiscControl;
class QThread;

class PEONYCORESHARED_EXPORT  UdfFormatDialog : public QDialog
{
    Q_OBJECT
public:
    UdfFormatDialog(const QString &uri, DiscControl *discControl,QWidget *parent = nullptr);
    virtual ~UdfFormatDialog();

private:
    QString m_uri;

    bool m_check;
    QMutex m_mutex;

    DiscControl *m_discControl = nullptr;
    QThread *m_thread = nullptr;

    const int m_widgetWidth = 424;
    const int m_widgetHeight = 300;

    QLabel* m_discTypeLabel = nullptr;
    QLabel* m_discNameLabel = nullptr;

    QLineEdit* m_discTypeEdit = nullptr;
    QLineEdit* m_discNameEdit = nullptr;

    QPushButton* m_okBtn = nullptr;
    QPushButton* m_cancelBtn = nullptr;

    QProgressBar* m_progress = nullptr;
    QGridLayout* m_mainLayout = nullptr;

public Q_SLOTS:
    void slot_udfFormat();
    void slot_udfCancel();
    void slot_formatFinished(bool successful, QString errorInfo);
    void slot_volumeDeviceRemove(const QString dev);
    void slot_FreeMemory();


protected:
    void closeEvent(QCloseEvent *) override;

private:
    bool udfFormatEnsureMsgBox();
    void setButtonState(bool state);
};

#endif // UDFFORMATDIALOG_H
