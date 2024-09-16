// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/details/file_helper.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/os.h>

#include <QDir>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <mutex>
#include <string>

#include <dlog_global.h>

#ifndef SPDLOG_VERSION_CHECK
#define SPDLOG_VERSION_CHECK(major, minor, patch) (major * 10000 + minor * 100 + patch)
#endif
DLOG_CORE_BEGIN_NAMESPACE

using namespace spdlog;

/*
 * Generator of Hourly log file names in format basename.ext.YYYY-MM-DD-HH-MM-SS
 */
struct rolling_filename_calculator
{
    // Create filename for the form basename.ext.YYYY-MM-DD-HH-MM-SS
    static filename_t calc_filename(const filename_t &filename, const tm &now_tm)
    {
#if SPDLOG_VERSION < SPDLOG_VERSION_CHECK(1, 10, 0)
#if SPDLOG_VERSION < SPDLOG_VERSION_CHECK(1, 4, 0)
        std::conditional<std::is_same<filename_t::value_type, char>::value, fmt::basic_memory_buffer<char, 250>, fmt::basic_string_view<wchar_t>>::type w;
#else
        std::conditional<std::is_same<filename_t::value_type, char>::value, spdlog::memory_buf_t, spdlog::wmemory_buf_t>::type w;
#endif
        fmt::format_to(w, SPDLOG_FILENAME_T("{}.{:04d}-{:02d}-{:02d}-{:02d}-{:02d}-{:02d}"), filename,
                       now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday,
                       now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec);
        return fmt::to_string(w);
#else
        return fmt_lib::format(SPDLOG_FILENAME_T("{}.{:04d}-{:02d}-{:02d}-{:02d}-{:02d}-{:02d}"), filename,
                               now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday,
                               now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec);
#endif
    }
};

enum RollingInterval
{
    RI_Minutely = 0,
    RI_Hourly,
    RI_HalfDaily,
    RI_Daily,
    RI_Weekly,
    RI_Monthly
};

/*
 * Rolling file sink based on time and file size.
 * If max_files > 0, retain only the last max_files and delete previous.
 */
template <typename Mutex, typename FileNameCalc = rolling_filename_calculator>
class rolling_file_sink final : public sinks::base_sink<Mutex>
{
public:
    // create rolling file sink which rollings on given interval
    rolling_file_sink(filename_t base_filename, std::size_t max_size, std::size_t max_files,
                      bool rolling_on_open = false, RollingInterval interval = RI_Daily)
        : base_filename_(std::move(base_filename)), rolling_on_open_(rolling_on_open), interval_(interval), filenames_q_()
    {
        set_max_size(max_size);
        set_max_files(max_files);

        auto filename = base_filename_;
        file_helper_.open(filename, false);
        current_size_ = file_helper_.size(); // expensive. called only once
        rotation_tp_ = next_rotation_tp_();
        if (rolling_on_open && current_size_ > 0)
        {
            rolling_();
            current_size_ = 0;
        }
    }

    filename_t filename()
    {
        std::lock_guard<Mutex> lock(sinks::base_sink<Mutex>::mutex_);
        return file_helper_.filename();
    }
    size_t filesize()
    {
        std::lock_guard<Mutex> lock(sinks::base_sink<Mutex>::mutex_);
        return file_helper_.size();
    }
    void set_max_files(std::size_t max_files)
    {
        std::lock_guard<Mutex> lock(sinks::base_sink<Mutex>::mutex_);
        if (max_files > 200000)
        {
            throw spdlog_ex("rolling sink constructor: max_files arg cannot exceed 200000");
        }
        max_files_ = max_files;
        if (max_files > 0)
            init_filenames_q_();
    }
    void set_max_size(std::size_t max_size)
    {
        std::lock_guard<Mutex> lock(sinks::base_sink<Mutex>::mutex_);
        if (max_size == 0)
        {
            throw spdlog_ex("rolling sink constructor: max_size arg cannot be zero");
        }
        max_size_ = max_size;
    }
    void set_interval(RollingInterval interval)
    {
        std::lock_guard<Mutex> lock(sinks::base_sink<Mutex>::mutex_);
        interval_ = interval;
        rotation_tp_ = next_rotation_tp_();
    }

protected:
    void sink_it_(const details::log_msg &msg) override
    {
#if SPDLOG_VERSION < SPDLOG_VERSION_CHECK(1, 4, 0)
        fmt::memory_buffer formatted;
#else
        memory_buf_t formatted;
#endif
        sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        auto new_size = current_size_ + formatted.size();

        auto time = msg.time;
        bool should_rolling = time >= rotation_tp_ || new_size > max_size_;
        if (should_rolling)
        {
            file_helper_.flush();
            if (file_helper_.size() > 0)
            {
                rolling_();
                new_size = formatted.size();
            }
        }

        file_helper_.write(formatted);
        current_size_ = new_size;

        // Do the cleaning only at the end because it might throw on failure.
        if (should_rolling && max_files_ > 0)
        {
            delete_old_();
        }
    }

    void flush_() override
    {
        file_helper_.flush();
    }

private:
    void init_filenames_q_()
    {
        filenames_q_.clear();
        QDir dir(QString::fromStdString(base_filename_));
        dir.cdUp();

        auto namefilter = QFileInfo(base_filename_.c_str()).fileName().append("*");

        auto fileInfos = dir.entryInfoList({namefilter}, QDir::NoDotAndDotDot | QDir::Files, QDir::Name);
        for (const auto &fi : fileInfos)
        {
            if (fi.filePath().compare(base_filename_.c_str()))
                filenames_q_.push_back(std::move(fi.filePath().toStdString()));
        }
    }

    tm now_tm(log_clock::time_point tp)
    {
        time_t tnow = log_clock::to_time_t(tp);
        return spdlog::details::os::localtime(tnow);
    }

    log_clock::time_point next_rotation_tp_()
    {
        auto now = log_clock::now();
        tm date = now_tm(now);
        auto rotation_time = log_clock::from_time_t(std::mktime(&date));

        switch (interval_)
        {
        case RI_Minutely:
            date.tm_min += 1;
            break;
        case RI_Hourly:
            date.tm_hour += 1;
            break;
        case RI_HalfDaily:
            date.tm_hour += 12;
            break;
        case RI_Daily:
            date.tm_mday += 1;
            break;
        case RI_Weekly:
            date.tm_mday += 7;
            break;
        case RI_Monthly:
            date.tm_mon += 1;
            break;
        }

        rotation_time = log_clock::from_time_t(std::mktime(&date));
        return rotation_time;
    }

    // Delete the file N rotations ago.
    // Throw spdlog_ex on failure to delete the old file.
    void delete_old_()
    {
        using details::os::filename_to_str;
        using details::os::remove;

        // base_filename_ not in filenames_q_
        while (filenames_q_.size() > max_files_ - 1)
        {
            auto old_filename = std::move(filenames_q_.front());
            filenames_q_.pop_front();
            bool ok = remove(old_filename) == 0;
            if (!ok)
            {
                filenames_q_.push_back(std::move(old_filename));
                throw(spdlog_ex("Failed removing file " + filename_to_str(old_filename), errno));
            }
        }
    }

    void rolling_()
    {
        using details::os::filename_to_str;

        file_helper_.close();
        // xxx.log == > xxx.log.YYYY-MM-DD-HH-MM-SS
        auto backupName = FileNameCalc::calc_filename(base_filename_, now_tm(log_clock::now()));
        if (details::os::rename(base_filename_, backupName))
        {
            file_helper_.reopen(true); // truncate the log file anyway to prevent it to grow beyond its limit!
            current_size_ = 0;
            throw spdlog_ex("rolling_file_sink: failed renaming " + filename_to_str(base_filename_) + " to " + filename_to_str(backupName), errno);
        }

        filenames_q_.push_back(std::move(backupName));
        rotation_tp_ = next_rotation_tp_();

        file_helper_.reopen(true);
    }

    filename_t base_filename_;
    log_clock::time_point rotation_tp_;
    details::file_helper file_helper_;
    bool rolling_on_open_;
    std::size_t max_size_;
    std::size_t max_files_;
    std::size_t current_size_;
    RollingInterval interval_;
    std::list<filename_t> filenames_q_;
};

using rolling_file_sink_mt = rolling_file_sink<std::mutex>;
using rolling_file_sink_st = rolling_file_sink<details::null_mutex>;

//
// factory functions
//
template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> rolling_logger_mt(const std::string &logger_name, const filename_t &filename, std::size_t max_file_size,
                                                 std::size_t max_files, bool rolling_on_open = false, RollingInterval interval = RI_Daily)
{
    return Factory::template create<rolling_file_sink_mt>(logger_name, filename, max_file_size, max_files, rolling_on_open, interval);
}

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> rolling_logger_st(const std::string &logger_name, const filename_t &filename, std::size_t max_file_size,
                                                 std::size_t max_files, bool rolling_on_open = false, RollingInterval interval = RI_Daily)
{
    return Factory::template create<rolling_file_sink_st>(logger_name, filename, max_file_size, max_files, rolling_on_open, interval);
}

template <typename Sink>
Sink *get_sink(const std::string &logger_name)
{
    Sink *sink = nullptr;

    auto fl = spdlog::get(logger_name);
    if (!fl)
        return sink;
    spdlog::sink_ptr s_ptr = fl->sinks()[0];
    sink = dynamic_cast<Sink *>(s_ptr.get());

    return sink;
}

DLOG_CORE_END_NAMESPACE
