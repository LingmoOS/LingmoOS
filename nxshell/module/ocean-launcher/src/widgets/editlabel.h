// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EDITLABEL_H
#define EDITLABEL_H

#include <dtkwidget_global.h>
#include <DFontSizeManager>

#include <QWidget>
#include <QLineEdit>
#include <QLabel>

class EditLabel : public QWidget
{
    Q_OBJECT

public:
    explicit EditLabel(QWidget *parent = nullptr);
    ~EditLabel() override;

    void setText(int maxWidth, const QString &title = QString());
    QString text() const;

    void cancelEditState();

signals:
    void titleChanged();

private slots:
    void onReturnPressed();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void initUi();
    void initConnection();
    void initAccessibleName();

private:
    QLineEdit *m_lineEdit;
    QLabel *m_label;
    QString m_oldTitle;      // 修改前的标题名称
    QString m_originTitle;   // 没有添加省略号的标题名称(输入的原始标题内容)
    int m_maxWidth;          // 文本最大宽度
};

#endif // EDITLABEL_H
