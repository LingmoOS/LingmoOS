// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AUDIOPROVIDER_H
#define AUDIOPROVIDER_H

#include "abstractosdprovider.h"

#include "org_deepin_dde_audio1.h"
#include "org_deepin_dde_audio1_sink.h"

using org::deepin::dde::Audio1;

/*!
 * \~chinese \class AudioProvider
 * \~chinese \brief 调整音频时的通知类
 */
class AudioProvider : public AbstractOSDProvider
{
    Q_OBJECT
public:
    explicit AudioProvider(QObject *parent = nullptr);

    // model
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    // delegate
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    Audio1 *m_audioInter;
    org::deepin::dde::audio1::Sink *m_sinkInter;

    QString pixmapName() const;
    void drawScale(QPainter *painter, const QStyleOptionViewItem &option, const QColor &color) const;

private slots:
    void defaultSinkChanged(const QDBusObjectPath &path);
};

#endif // AUDIOPROVIDER_H
