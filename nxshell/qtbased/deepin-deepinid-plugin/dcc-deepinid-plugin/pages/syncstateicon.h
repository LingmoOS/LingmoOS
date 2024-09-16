// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SYNCSTATEICON_H
#define SYNCSTATEICON_H

#include <QLabel>
#include <QVariantAnimation>

class SyncStateIcon : public QLabel
{
    Q_OBJECT
public:
    explicit SyncStateIcon(QWidget* parent = nullptr);
    ~SyncStateIcon();

    void setRotatePixmap(const QPixmap &pixmap);

    void play();
    void stop();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QPixmap m_rotatePixmap;
    QVariantAnimation m_rotateAni;
    qreal m_rotateRatio;
};
#endif  // !SYNCSTATEICON_H
