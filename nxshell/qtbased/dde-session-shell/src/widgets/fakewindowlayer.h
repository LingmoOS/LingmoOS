// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FAKEWINDOWLAYER_H
#define FAKEWINDOWLAYER_H

#include <QWidget>

class FakeWindowLayer : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QWidget* content READ content WRITE setContent)
public:
    FakeWindowLayer(QWidget *parent = nullptr);
    ~FakeWindowLayer() override = default;

    void setContent(QWidget *toSet);
    QWidget *content() const;
    void setVisible(bool visible) override;

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    QWidget *m_content;
    QWidget *m_savedParent;
    QWidget *m_savedFocus;
};

#endif  // FAKEWINDOWLAYER_H
