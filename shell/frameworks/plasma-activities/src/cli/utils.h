/*
    SPDX-FileCopyrightText: 2016 Ivan Čukić <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KACTIVITIES_UTILS_H
#define KACTIVITIES_UTILS_H

QTextStream out(stdout);

class StringListView
{
public:
    StringListView(const QStringList &list, int start, int end = -1)
        : m_list(list)
        , m_start(start)
        , m_size((end == -1 ? list.count() : end) - start)
    {
    }

    const QString &operator()(int index) const
    {
        return m_list[m_start + index];
    }

    int count() const
    {
        return m_size;
    }

private:
    const QStringList &m_list;
    int m_start;
    int m_size;
};

KActivities::Controller *controller = nullptr;

class Flags
{
public:
    Flags()
        : bare(false)
        , color(true)
    {
    }

    bool bare;
    bool color;

} flags;

QString toDashes(const QString &command)
{
    QString result(command);

    for (int i = 0; i < result.size() - 1; ++i) {
        if (result[i].isLower() && result[i + 1].isUpper()) {
            result[i + 1] = result[i + 1].toLower();
            result.insert(i + 1, QStringLiteral("-"));
        }
    }

    return result;
}

void printActivity(const QString &id)
{
    // clang-format off
    if (flags.bare) {
        out << id << "\n";

    } else {
        using namespace KActivities;
        Info info(id);

        out
            << (
                info.id() == controller->currentActivity() ? "[CURRENT] " :
                info.state() == Info::Running    ? "[RUNNING] " :
                info.state() == Info::Stopped    ? "[STOPPED] " :
                info.state() == Info::Starting   ? "[STARTING]" :
                info.state() == Info::Stopping   ? "[STOPPING]" :
                                                   "unknown   "
            )
            << info.id()
            << " "
            << info.name()
            << " ("
            << info.icon()
            << ")\n"
             ;

        if (info.id() == controller->currentActivity()
            && info.state() != Info::Running) {
            qWarning()
                 << "Activity is the current one, but its state is"
                 << (
                    info.state() == Info::Running  ? "running"  :
                    info.state() == Info::Stopped  ? "stopped"  :
                    info.state() == Info::Starting ? "starting" :
                    info.state() == Info::Stopping ? "stopping" :
                                                     "unknown   "
                 );
        }
    }
    // clang-format on
}

template<typename T>
T awaitFuture(const QFuture<T> &future)
{
    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }

    return future.result();
}

void awaitFuture(const QFuture<void> &future)
{
    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }
}

void switchToActivity(const QString &id)
{
    auto result = awaitFuture(controller->setCurrentActivity(id));

    if (!flags.bare) {
        if (result) {
            qDebug() << "Current activity is" << id;
        } else {
            qDebug() << "Failed to change the activity";
        }
    }
}

// clang-format off
#define DEFINE_COMMAND(Command, MinArgCount)                                   \
    struct Command##_command {                                                 \
        const StringListView &args;                                            \
        Command##_command(const StringListView &args)                          \
            : args(args)                                                       \
        {                                                                      \
            if (args.count() < MinArgCount + 1) {                              \
                qFatal("not enough arguments for " #Command);                  \
            }                                                                  \
        }                                                                      \
                                                                               \
        int operator()();                                                      \
    };                                                                         \
                                                                               \
    int Command##_command::operator()()

#endif
// clang-format on