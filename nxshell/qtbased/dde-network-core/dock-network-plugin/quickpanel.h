// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef QUICKPANEL_H
#define QUICKPANEL_H

#include <QIcon>
#include <QVariant>
#include <QWidget>

namespace Dtk {
namespace Widget {
class DLabel;
class DIconButton;
}
}

class QuickPanel : public QWidget
{
    Q_OBJECT

public:
    explicit QuickPanel(QWidget *parent = nullptr);
    const QVariant &userData() const;
    void setUserData(const QVariant &data);
    const QString text() const;
    const QString description() const;

public Q_SLOTS:
    void setIcon(const QIcon &icon);
    void setText(const QString &text);
    void setDescription(const QString &description);
    void setActive(bool active);

Q_SIGNALS:
    void panelClicked();
    void iconClicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void initUi();
    void initConnect();
    void setHover(bool hover);

private:
    QVariant m_userData;

    Dtk::Widget::DIconButton *m_iconButton;
    Dtk::Widget::DLabel *m_text;
    Dtk::Widget::DLabel *m_description;

    bool m_hover;
};

#endif // QUICKPANEL_H
