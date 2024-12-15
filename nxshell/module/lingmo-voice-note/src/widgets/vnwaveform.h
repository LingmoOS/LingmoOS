// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VNWAVEFORM_H
#define VNWAVEFORM_H

#include <DFrame>

#include <QAudioBuffer>

DWIDGET_USE_NAMESPACE

class VNWaveform : public DFrame
{
    Q_OBJECT
public:
    explicit VNWaveform(QWidget *parent = nullptr);

    const int WAVE_WIDTH = 2;
    const int WAVE_SPACE = 2;

    const int WAVE_OFFSET_X = 0; //Left/Right margin
    const int WAVE_OFFSET_Y = 0; //top/Bottom margin

    const int WAVE_REFRESH_FREQ = 100; //100ms

    enum WaveStyle {
        Columnar = 1,
        Wave = 2,
    };
signals:

public slots:
    //数据更新
    void onAudioBufferProbed(const QAudioBuffer &buffer);

protected:
    //波形数据转换
    static qreal getPeakValue(const QAudioFormat &format);
    static void getBufferLevels(const QAudioBuffer &buffer, QVector<qreal> &scaleSamples, qreal &frameGain);

    template<class T>
    static void getBufferLevels(const T *buffer,
                                int frames,
                                int channels,
                                qreal peakValue,
                                QVector<qreal> &samples,
                                qreal &frameGain);
    //绘制波形
    void paintEvent(QPaintEvent *event) override;
    //窗口大小改变
    void resizeEvent(QResizeEvent *event) override;

protected:
    QVector<qreal> m_audioScaleSamples;

    qreal m_frameGain {0};
    const qreal m_defaultGain = 3;

    int m_maxShowedSamples {0};
    int m_waveStyle {WaveStyle::Columnar};
};

#endif // VNWAVEFORM_H
