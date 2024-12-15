// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vnwaveform.h"
#include "globaldef.h"

#include <DLog>

#include <QMouseEvent>
#include <QPen>
#include <QPainter>

/**
 * @brief VNWaveform::VNWaveform
 * @param parent
 */
VNWaveform::VNWaveform(QWidget *parent)
    : DFrame(parent)
{
    setFrameShape(QFrame::NoFrame);
}

/**
 * @brief VNWaveform::onAudioBufferProbed
 * @param buffer
 */
void VNWaveform::onAudioBufferProbed(const QAudioBuffer &buffer)
{
    static struct timeval curret = {0, 0};
    static struct timeval lastUpdate = {0, 0};

    gettimeofday(&curret, nullptr);

    if (TM(lastUpdate, curret) > WAVE_REFRESH_FREQ) {
        getBufferLevels(buffer, m_audioScaleSamples, m_frameGain);

        //Max sampe value is sqrt(Max)
        m_frameGain = qSqrt(m_frameGain);

        //If the volume too low, use the
        //defaultGain * 2 as max sample value
        if (m_frameGain < m_defaultGain) {
            m_frameGain = m_defaultGain * 2;
        }

        update();

        //last update time.
        lastUpdate = curret;
    }
}

/**
 * @brief VNWaveform::paintEvent
 * @param event
 */
void VNWaveform::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    //Support highquality
    qreal waveWidth = WAVE_WIDTH;
    qreal waveSpace = WAVE_SPACE;

    qreal waveOffsetX = WAVE_OFFSET_X;
    qreal waveOffsetY = WAVE_OFFSET_Y;

    const qreal availableHeight = height() - (waveOffsetY * 2);

    const QColor startColor(3, 208, 214);
    const QColor endColor(1, 121, 255);

    for (int i = 0; (i < m_audioScaleSamples.size() && i < m_maxShowedSamples); i++) {
        qreal scaler = qSqrt(m_audioScaleSamples[i]) / m_frameGain;

        qreal waveHeight = scaler * height();

        //Use default height when waveHeight less or equal to 0
        waveHeight = (waveHeight > waveWidth) ? waveHeight : waveWidth;

        qreal startX = waveOffsetX + i * (waveWidth + waveSpace);
        qreal startY = (availableHeight - waveHeight) / m_waveStyle;

        QRectF waveRectF(startX, startY, waveWidth, waveHeight);

        QLinearGradient waveGradient(waveRectF.topLeft(), waveRectF.bottomLeft());

        waveGradient.setColorAt(0, startColor);
        waveGradient.setColorAt(1, endColor);

        painter.fillRect(waveRectF, waveGradient);
    }

    painter.restore();
}

/**
 * @brief VNWaveform::resizeEvent
 * @param event
 */
void VNWaveform::resizeEvent(QResizeEvent *event)
{
    m_maxShowedSamples = static_cast<int>(
        (event->size().width() - WAVE_OFFSET_X * 2)
        / ((WAVE_WIDTH + WAVE_SPACE)));

    qDebug() << __FUNCTION__ << "m_maxShowedSamples:" << m_maxShowedSamples
             << "width:" << event->size().width();
    DFrame::resizeEvent(event);
}

/**
 * @brief VNWaveform::getBufferLevels
 * @param buffer
 * @param scaleSamples
 * @param frameGain
 */
void VNWaveform::getBufferLevels(
    const QAudioBuffer &buffer,
    QVector<qreal> &scaleSamples,
    qreal &frameGain)
{
    QVector<qreal> values;

    if (!buffer.format().isValid() || buffer.format().byteOrder() != QAudioFormat::LittleEndian)
        return;

    if (buffer.format().codec() != "audio/pcm")
        return;

    int channelCount = buffer.format().channelCount();

    scaleSamples.reserve(buffer.frameCount());
    scaleSamples.fill(0, buffer.frameCount());

    //TODO:
    //   Use the max sample value instead of this value.
    //So it's not necessory.
    //qreal peak_value = VNWaveform::getPeakValue(buffer.format());
    qreal peak_value = 0;

    switch (buffer.format().sampleType()) {
    case QAudioFormat::Unknown:
    case QAudioFormat::UnSignedInt:
        if (buffer.format().sampleSize() == 32)
            VNWaveform::getBufferLevels(
                buffer.constData<quint32>(),
                buffer.frameCount(),
                channelCount, peak_value, scaleSamples, frameGain);
        if (buffer.format().sampleSize() == 16)
            VNWaveform::getBufferLevels(
                buffer.constData<quint16>(),
                buffer.frameCount(),
                channelCount, peak_value, scaleSamples, frameGain);
        if (buffer.format().sampleSize() == 8)
            VNWaveform::getBufferLevels(
                buffer.constData<quint8>(),
                buffer.frameCount(),
                channelCount, peak_value, scaleSamples, frameGain);
        break;
    case QAudioFormat::Float:
        if (buffer.format().sampleSize() == 32) {
            VNWaveform::getBufferLevels(
                buffer.constData<float>(),
                buffer.frameCount(),
                channelCount, peak_value, scaleSamples, frameGain);
        }
        break;
    case QAudioFormat::SignedInt:
        if (buffer.format().sampleSize() == 32)
            VNWaveform::getBufferLevels(
                buffer.constData<qint32>(),
                buffer.frameCount(),
                channelCount, peak_value, scaleSamples, frameGain);
        if (buffer.format().sampleSize() == 16)
            VNWaveform::getBufferLevels(
                buffer.constData<qint16>(),
                buffer.frameCount(),
                channelCount, peak_value, scaleSamples, frameGain);
        if (buffer.format().sampleSize() == 8)
            VNWaveform::getBufferLevels(
                buffer.constData<qint8>(),
                buffer.frameCount(),
                channelCount, peak_value, scaleSamples, frameGain);
        break;
    }
}

template<class T>
/**
 * @brief VNWaveform::getBufferLevels
 * @param buffer
 * @param frames
 * @param channels
 * @param peakValue
 * @param samples
 * @param frameGain
 */
void VNWaveform::getBufferLevels(
    const T *buffer, int frames, int channels,
    qreal peakValue, QVector<qreal> &samples, qreal &frameGain)
{
    Q_UNUSED(peakValue);

    for (int i = 0; i < frames; ++i) {
        qreal averageValue = 0;

        for (int j = 0; j < channels; ++j) {
            averageValue += qAbs(qreal(buffer[i * channels + j]));
        }

        averageValue = (averageValue) / channels;

        //Get the max sample value
        if (frameGain < averageValue) {
            frameGain = averageValue;
        }

        samples[i] = averageValue;
    }
}

/**
 * @brief VNWaveform::getPeakValue
 * @param format
 * @return
 */
qreal VNWaveform::getPeakValue(const QAudioFormat &format)
{
    // Note: Only the most common sample formats are supported
    if (!format.isValid())
        return qreal(0);

    if (format.codec() != "audio/pcm")
        return qreal(0);

    switch (format.sampleType()) {
    case QAudioFormat::Unknown:
        break;
    case QAudioFormat::Float:
        if (format.sampleSize() != 32) // other sample formats are not supported
            return qreal(0);
        return qreal(1.00003);
    case QAudioFormat::SignedInt:
        if (format.sampleSize() == 32)
            return qreal(INT_MAX);
        if (format.sampleSize() == 16)
            return qreal(SHRT_MAX);
        if (format.sampleSize() == 8)
            return qreal(CHAR_MAX);
        break;
    case QAudioFormat::UnSignedInt:
        if (format.sampleSize() == 32)
            return qreal(UINT_MAX);
        if (format.sampleSize() == 16)
            return qreal(USHRT_MAX);
        if (format.sampleSize() == 8)
            return qreal(UCHAR_MAX);
        break;
    }

    return qreal(0);
}
