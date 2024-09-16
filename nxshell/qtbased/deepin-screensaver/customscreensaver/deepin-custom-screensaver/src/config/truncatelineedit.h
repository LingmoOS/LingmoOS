// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRUNCATELINEEDIT_H
#define TRUNCATELINEEDIT_H

#include <DLineEdit>

class TruncateLineEdit : public Dtk::Widget::DLineEdit
{
    Q_OBJECT
public:
    explicit TruncateLineEdit(QWidget *parent = nullptr);

    void setText(const QString &text);
    QString getText() const;

    void setElideMode(Qt::TextElideMode elideMode);
    Qt::TextElideMode elideMode() const;

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateShowText();

private:
    Qt::TextElideMode m_elideMode;
    QString m_fullText;
    QString m_elideText;
};

#endif // TRUNCATELINEEDIT_H
