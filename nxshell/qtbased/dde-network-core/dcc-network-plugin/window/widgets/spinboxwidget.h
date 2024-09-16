// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SPINBOXWIDGET_H
#define SPINBOXWIDGET_H

#include "widgets/settingsitem.h"

#include <DIconButton>
#include <DSpinBox>


QT_BEGIN_NAMESPACE
class QLabel;
class QSpinBox;
QT_END_NAMESPACE

namespace dcc {
namespace network {

class SpinBoxWidget : public DCC_NAMESPACE::SettingsItem
{
    Q_OBJECT

public:
    explicit SpinBoxWidget(QWidget *parent = nullptr);

    inline DTK_WIDGET_NAMESPACE::DSpinBox *spinBox() const { return m_spinBox; }

    void setTitle(const QString &title);
    void setDefaultVal(int defaultVal);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    int m_defaultVal;
    QLabel *m_title;
    DTK_WIDGET_NAMESPACE::DSpinBox *m_spinBox;
    DTK_WIDGET_NAMESPACE::DIconButton *m_resetBtn;
};

}
}

#endif // SPINBOXWIDGET_H
