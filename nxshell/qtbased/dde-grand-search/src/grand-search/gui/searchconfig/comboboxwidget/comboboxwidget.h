// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMBOBOXWIDGET_H
#define COMBOBOXWIDGET_H

#include <QWidget>
#include <DComboBox>
#include <QLabel>
#include <QHBoxLayout>

namespace GrandSearch {

class ComboboxWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ComboboxWidget(const QString &title, QWidget *parent = nullptr);
    explicit ComboboxWidget(QWidget *parent = nullptr, QWidget *leftWidget = nullptr);

    void setChecked(const QString &text);
    QString checked() const;

    void setTitle(const QString &title);
    QString title() const;

    void setEnableBackground(const bool enable = true);
    bool enableBackground() const;
    Dtk::Widget::DComboBox *getComboBox();

protected:
    void paintEvent(QPaintEvent *event) override;

signals:
    void checkedChanged(int index) const;

private:
    bool m_hasBack = false;
    QHBoxLayout *m_mainLayout = nullptr;
    QWidget *m_leftWidget = nullptr;
    Dtk::Widget::DComboBox *m_comboBox = nullptr;
};

}

#endif // COMBOBOXWIDGET_H
