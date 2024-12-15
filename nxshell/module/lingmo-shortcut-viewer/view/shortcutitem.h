// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHORTCUTITEM_H
#define SHORTCUTITEM_H

#include <QWidget>
#include <QLabel>

class ShortcutItem : public QWidget
{
    Q_OBJECT
public:
    explicit ShortcutItem(bool isGroup = false, QWidget *parent = nullptr);

    void setText(const QString &name, const QString &value);
    QString name() const;
    QString value() const;

    void setEnableBackground(bool enable);
    bool enableBackground() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    bool m_hasBackground = false;
    bool m_isGroup = false;

    QLabel *m_nameLabel = nullptr;
    QLabel *m_valueLabel = nullptr;
};

#endif   // SHORTCUTITEM_H
