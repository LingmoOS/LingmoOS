// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CCUSTOMTIMEEDIT_H
#define CCUSTOMTIMEEDIT_H

#include <QDateTimeEdit>

/**
 * @brief The CCustomTimeEdit class
 */
class CCustomTimeEdit : public QTimeEdit
{
    Q_OBJECT
public:
    explicit CCustomTimeEdit(QWidget *parent = nullptr);
    //获取编辑框
    QLineEdit *getLineEdit();
protected:
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
signals:
    void signalUpdateFocus(bool showFocus);
};

#endif // CCUSTOMTIMEEDIT_H
