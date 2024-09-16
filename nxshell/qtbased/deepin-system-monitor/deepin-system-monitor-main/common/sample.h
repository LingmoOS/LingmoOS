// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SAMPLE_H
#define SAMPLE_H

#include "time_period.h"
#include "common/common.h"

#include <memory>
#include <list>

#include <sys/time.h>

using namespace common::core;
using namespace common::time;

struct IO {
    qulonglong inBytes;
    qulonglong outBytes;

    struct IO &operator-=(const struct IO &rhs)
    {
        inBytes -= rhs.inBytes;
        outBytes -= rhs.outBytes;
        return *this;
    }
    struct IO &operator+=(const struct IO &rhs)
    {
        inBytes += rhs.inBytes;
        outBytes += rhs.outBytes;
        return *this;
    }
    const struct IO operator-(const struct IO &other) const {
        return IO(*this) -= other;
    }
    const struct IO operator+(const struct IO &other) const {
        return IO(*this) += other;
    }
};

struct DiskIO {
    qulonglong inBytes;
    qulonglong outBytes;
    qulonglong cancelBytes;

    struct DiskIO &operator-=(const struct DiskIO &rhs)
    {
        inBytes -= rhs.inBytes;
        outBytes -= rhs.outBytes;
        cancelBytes -= rhs.cancelBytes;
        return *this;
    }
    struct DiskIO &operator+=(const struct DiskIO &rhs)
    {
        inBytes += rhs.inBytes;
        outBytes += rhs.outBytes;
        cancelBytes += rhs.cancelBytes;
        return *this;
    }
    const struct DiskIO operator-(const struct DiskIO &other) const {
        return DiskIO(*this) -= other;
    }
    const struct DiskIO operator+(const struct DiskIO &other) const {
        return DiskIO(*this) += other;
    }
};

struct IOPS {
    qreal inBps;
    qreal outBps;
};

template<typename T>
class SampleFrame
{
public:
    SampleFrame()
        : data()
    {
    }
    SampleFrame(const T &d)
        : data(d)
    {
    }
    SampleFrame(const SampleFrame<T> &other)
        : data(other.data)
    {
    }

    T data;
};

template<>
class SampleFrame<IO>
{
public:
    SampleFrame()
        : ts()
        , data()
    {
    }
    SampleFrame(const struct timeval &tv, const struct IO &d)
        : ts(tv)
        , data(d)
    {
    }
    SampleFrame(const SampleFrame<IO> &other)
        : ts(other.ts)
        , data(other.data)
    {
    }

    SampleFrame<IO> &operator+=(const SampleFrame<IO> &rhs)
    {
        ts += rhs.ts;
        data += rhs.data;
        return *this;
    }
    SampleFrame<IO> &operator-=(const SampleFrame<IO> &rhs)
    {
        ts -= rhs.ts;
        data -= rhs.data;
        return *this;
    }
    const SampleFrame<IO> operator+(const SampleFrame<IO> &rhs) const
    {
        return SampleFrame<IO>(*this) += rhs;
    }
    const SampleFrame<IO> operator-(const SampleFrame<IO> &rhs) const
    {
        return SampleFrame<IO>(*this) -= rhs;
    }

    static IOPS iops(const SampleFrame<IO> *lhs, const SampleFrame<IO> *rhs)
    {
        if (!lhs || !rhs)
            return {.0, .0};

        auto ltime = lhs->ts.tv_sec + lhs->ts.tv_usec * 1. / 1000000;
        auto rtime = rhs->ts.tv_sec + rhs->ts.tv_usec * 1. / 1000000;
        auto interval = (rtime > ltime) ? (rtime - ltime) : 1;

        auto inBps = rhs->data.inBytes / interval;
        auto outBps = rhs->data.outBytes / interval;

        return {inBps, outBps};
    }

    struct timeval ts;
    struct IO data;
};

template<>
class SampleFrame<DiskIO>
{
public:
    SampleFrame()
        : ts()
        , data()
    {
    }
    SampleFrame(const struct timeval &tv, const struct DiskIO &d)
        : ts(tv)
        , data(d)
    {
    }
    SampleFrame(const SampleFrame<DiskIO> &other)
        : ts(other.ts)
        , data(other.data)
    {
    }

    SampleFrame<DiskIO> &operator+=(const SampleFrame<DiskIO> &rhs)
    {
        ts += rhs.ts;
        data += rhs.data;
        return *this;
    }
    SampleFrame<DiskIO> &operator-=(const SampleFrame<DiskIO> &rhs)
    {
        ts -= rhs.ts;
        data -= rhs.data;
        return *this;
    }
    const SampleFrame<DiskIO> operator+(const SampleFrame<DiskIO> &rhs) const
    {
        return SampleFrame<DiskIO>(*this) += rhs;
    }
    const SampleFrame<DiskIO> operator-(const SampleFrame<DiskIO> &rhs) const
    {
        return SampleFrame<DiskIO>(*this) -= rhs;
    }

    static IOPS diskiops(const SampleFrame<DiskIO> *lhs, const SampleFrame<DiskIO> *rhs)
    {
        if (!lhs || !rhs)
            return {.0, .0};

        auto ltime = lhs->ts.tv_sec + lhs->ts.tv_usec * 1. / 1000000;
        auto rtime = rhs->ts.tv_sec + rhs->ts.tv_usec * 1. / 1000000;
        auto interval = (rtime > ltime) ? (rtime - ltime) : 1;

        // read speed, write speed
        qreal rdio {0}, wrio {0};
        // current read/write/cancelled, previous read/write/cancelled
        qulonglong crb{0}, cwb{0}, ccwb{0}, prb{0}, pwb{0}, pcwb{0};

        // current read bytes, write bytes, cancelled write bytes
        crb = rhs->data.inBytes;
        cwb = rhs->data.outBytes;
        ccwb = rhs->data.cancelBytes;

        prb = lhs->data.inBytes;
        pwb = lhs->data.outBytes;
        pcwb = lhs->data.cancelBytes;

        // read/write/cancelled write increments between stats
        auto rdiff = (crb < prb) ? 0 : (crb - prb);
        auto wdiff = (cwb < pwb) ? 0 : (cwb - pwb);
        auto cwdiff = (ccwb < pcwb) ? 0 : (ccwb - pcwb);
        // calculate read/write speed
        rdio = qreal(rdiff) / interval;
        wrio = (wdiff < cwdiff) ? 0 : qreal(wdiff - cwdiff) / interval;

        return {rdio, wrio};
    }

    struct timeval ts;
    struct DiskIO data;
};

template<>
class SampleFrame<qulonglong>
{
public:
    SampleFrame()
        : data()
    {
    }
    SampleFrame(const struct timeval &tv, qulonglong &d)
        : ts(tv)
        , data(d)
    {
    }
    SampleFrame(const SampleFrame<qulonglong> &other)
        : ts(other.ts)
        , data(other.data)
    {
    }

    SampleFrame<qulonglong> &operator+=(const SampleFrame<qulonglong> &rhs)
    {
        ts += rhs.ts;
        data += rhs.data;
        return *this;
    }
    SampleFrame<qulonglong> &operator-=(const SampleFrame<qulonglong> &rhs)
    {
        ts -= rhs.ts;
        data -= rhs.data;
        return *this;
    }
    const SampleFrame<qulonglong> operator+(const SampleFrame<qulonglong> &rhs) const
    {
        return SampleFrame(*this) += rhs;
    }
    const SampleFrame<qulonglong> operator-(const SampleFrame<qulonglong> &rhs) const
    {
        return SampleFrame(*this) -= rhs;
    }

    struct timeval ts;
    qulonglong data;
};

template<typename T>
class Sample
{
public:
    explicit Sample()
        : m_samples {}
        , m_period {}
        , m_maxSamples {}
    {
        m_maxSamples = m_period.ticks();
    }
    explicit Sample(const TimePeriod &period)
        : m_samples {}
        , m_period {period}
        , m_maxSamples {}
    {
        m_maxSamples = m_period.ticks();
    }
    Sample(const Sample &other)
        : m_period(other.m_period)
        , m_maxSamples(other.m_maxSamples)
    {
        for (auto &item : other.m_samples) {
            m_samples.push_back(std::make_shared<SampleFrame<T>>(*item));
        }
        m_maxSamples = m_period.ticks();
    }

    inline void addSample(SampleFrame<T> *frame)
    {
        if (frame) {
            if (m_samples.size() >= m_maxSamples)
                m_samples.pop_front();

            std::shared_ptr<SampleFrame<T>> framePtr;
            framePtr.reset(frame);
            m_samples.push_back(framePtr);
        }
    }

    inline const TimePeriod &timePeriod() const
    {
        return m_period;
    }

    inline const SampleFrame<T> *recentSample() const
    {
        if (m_samples.size() > 0)
            return m_samples.last().get();
        else
            return nullptr;
    }

    inline void updateTimePeriod(const TimePeriod &newPeriod)
    {
        auto ticks = newPeriod.ticks();
        auto old = m_period.ticks();

        if (timercmp(&newPeriod.interval(), &m_period.interval(), !=)) {
            m_samples.clear();
        } else if (ticks < old) {
            while (m_samples.size() > ticks) {
                m_samples.pop_front();
            }
        }
        m_maxSamples = ticks;
        m_period = newPeriod;
    }

    inline const QPair<const SampleFrame<T> *, const SampleFrame<T> *> recentSamplePair() const
    {
        QPair<const SampleFrame<T> *, const SampleFrame<T> *> pair {};
        if (m_samples.size() > 0)
            pair.first = m_samples[0].get();
        if (m_samples.size() > 1)
            pair.second = m_samples[1].get();
        return pair;
    }

    inline int count() const
    {
        return m_samples.size();
    }

    inline const SampleFrame<T> *sample(int index) const
    {
        if (index >= 0 && index < m_samples.size()) {
            return m_samples[index].get();
        }

        return nullptr;
    }

private:
    // TODO: replace with circular buffer impl. (ref: boost/circular_buffer)
    QList<std::shared_ptr<SampleFrame<T>>> m_samples;
    TimePeriod m_period;
    size_t m_maxSamples;
};

using IOSampleFrame = SampleFrame<IO>;
using DISKIOSampleFrame = SampleFrame<DiskIO>;
using IOPSSampleFrame = SampleFrame<IOPS>;
using IOSample = Sample<IO>;
using DISKIOSample = Sample<DiskIO>;
using IOPSSample = Sample<IOPS>;

#endif // SAMPLE_H
