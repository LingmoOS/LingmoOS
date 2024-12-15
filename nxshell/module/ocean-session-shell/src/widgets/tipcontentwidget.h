// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIPCONTENTWIDGET_H
#define TIPCONTENTWIDGET_H

#include <QFrame>

class TipContentWidget : public QFrame
{
    Q_OBJECT

    enum ShowType {
        SingleLine,
        MultiLine
    };

public:
    explicit TipContentWidget(QWidget *parent = nullptr);

    const QString& text(){ return m_text; }
    const QStringList &textList() { return  m_textList; }

    void setText(const QString &text, bool multiLine = true);
    void setTextList(const QStringList &textList);

protected:
    void paintEvent(QPaintEvent *event) override;
    bool event(QEvent *event) override;

private:
    void showSingleLineText(const QString &text);

private:
    QString m_text;
    QStringList m_textList;
    ShowType m_type;
};

#endif // TIPCONTENTWIDGET_H
