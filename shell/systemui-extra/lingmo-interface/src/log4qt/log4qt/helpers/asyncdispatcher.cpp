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

#include "log4qt/helpers/asyncdispatcher.h"
#include "log4qt/loggingevent.h"
#include "log4qt/appenderskeleton.h"

#include <QCoreApplication>

namespace Log4Qt
{

AsyncDispatcher::AsyncDispatcher(QObject *parent) : QObject(parent)
    , mAsyncAppender(nullptr)
{}

void AsyncDispatcher::customEvent(QEvent *event)
{
    if (event->type() == LoggingEvent::eventId)
    {
        auto *logEvent = static_cast<LoggingEvent *>(event);
        if (mAsyncAppender != nullptr)
            mAsyncAppender->asyncAppend(*logEvent);
    }
    QObject::customEvent(event);
}

void AsyncDispatcher::setAsyncAppender(AppenderSkeleton *asyncAppender)
{
    mAsyncAppender = asyncAppender;
}

} // namespace Log4Qt