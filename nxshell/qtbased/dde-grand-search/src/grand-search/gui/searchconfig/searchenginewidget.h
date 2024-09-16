// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHENGINEWIDGET_H
#define SEARCHENGINEWIDGET_H

#include <DWidget>
#include <DLabel>
#include <DSwitchButton>
#include <DLineEdit>

#include <QVBoxLayout>

namespace GrandSearch {

class ComboboxWidget;
class SearchEngineWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit SearchEngineWidget(QWidget *parent = nullptr);
    ~SearchEngineWidget();

private slots:
    void checkedChangedIndex(int index);
    void setCustomSearchEngineAddress(const QString &text);

private:
    int getIndex(const QString text) const;

private:
    QVBoxLayout *m_mainLayout = nullptr;
    QLabel *m_groupLabel= nullptr;
    QLabel *m_contentLabel = nullptr;
    Dtk::Widget::DLineEdit *m_lineEdit = nullptr;

    QHash<QString, QString> m_groupName;        // <searchGroupName, displayGroupName>
    ComboboxWidget *m_comboboxWidget;
};

}

#endif // SEARCHENGINEWIDGET_H
