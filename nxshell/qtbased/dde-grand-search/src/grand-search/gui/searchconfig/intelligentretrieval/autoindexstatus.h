// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AUTOINDEXSTATUS_H
#define AUTOINDEXSTATUS_H

#include <DSpinner>

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>

namespace GrandSearch {

class AutoIndexStatus : public QWidget
{
    Q_OBJECT
public:
    enum Status {Success, Updating, Fail};
    explicit AutoIndexStatus(QWidget *parent = nullptr);
    void updateContent(Status st, const QString &text);
signals:

public slots:
private:
    QLabel *m_text = nullptr;
    QLabel *m_icon = nullptr;
    QHBoxLayout *m_layout = nullptr;
    DTK_WIDGET_NAMESPACE::DSpinner *m_spinner = nullptr;
};

}
#endif // AUTOINDEXSTATUS_H
