/*
 * Copyright (C) 2021 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#ifndef LOG4QT_ASYNCDISPATCHER_H
#define LOG4QT_ASYNCDISPATCHER_H

#include <QObject>
#include "lingmo-logmacros.h"

namespace Log4Qt
{

class AppenderSkeleton;

/*!
 * \brief The class AsyncDispatcher does the actual logging to the attached appanders.
 *
 * The Dispatcher is the worker object which class the attached apperders in the
 * the context of the AsyncDispatcherThread.
 *
 * \note All the functions declared in this class are thread-safe.
 */
class LIBLINGMOLOG4QT_EXPORT AsyncDispatcher : public QObject
{
    Q_OBJECT
public:
    explicit AsyncDispatcher(QObject *parent = nullptr);

    void setAsyncAppender(AppenderSkeleton *asyncAppender);

protected:
    void customEvent(QEvent *event) override;

private:
    AppenderSkeleton *mAsyncAppender;
};

} // namespace Log4Qt

#endif // LOG4QT_ASYNCDISPATCHER_H
