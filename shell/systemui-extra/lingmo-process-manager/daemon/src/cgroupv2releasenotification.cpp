/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "cgroupv2releasenotification.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define MAX_EVENTS 5
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

CGroupV2ReleaseNotification::CGroupV2ReleaseNotification()
    : m_started(false)
{
}

void CGroupV2ReleaseNotification::addWatchGroup(
    const std::string &groupPath, ReleaseHandler releaseHandler)
{
    if (groupPath.empty()) {
        return;
    }

    if (!m_started) {
        startWatch();
    }

    int wd = inotify_add_watch(m_inotifyFd, groupPath.c_str(), IN_MODIFY);
    if (wd < 0) {
        perror("inotify_add_watch");
        return;
    }

    {
        std::lock_guard<std::mutex> locker(m_mutex);
        m_watchedPathInfo[wd] = { groupPath, std::move(releaseHandler) };   
    }
}

void CGroupV2ReleaseNotification::startWatch()
{
    m_inotifyFd = inotify_init();
    if (m_inotifyFd < 0) {
        perror("inotify_init");
        return;
    }

    m_epollFd = initEpoll();
    if (m_epollFd < 0) {
        perror("initEpoll");
        return;
    }

    m_started = true;
    doStartWatch();
}

int CGroupV2ReleaseNotification::initEpoll()
{
    int epollFd = epoll_create1(0);
    if (epollFd == -1) {
        perror("epoll_create1");
        return -1;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = m_inotifyFd;

    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, m_inotifyFd, &ev) == -1) {
        perror("epoll_ctl");
        return -1;
    }

    return epollFd;
}

void CGroupV2ReleaseNotification::doStartWatch()
{
    m_watcherFuture = std::async(std::launch::async, [this]() {
        struct epoll_event events[MAX_EVENTS];
        for (;;) {
            int eventNumber = epoll_wait(m_epollFd, (epoll_event *)events, MAX_EVENTS, -1);
            if (eventNumber < 0) {
                perror("epoll_wait");
                break;
            }
            handleEpollEvents(events, eventNumber);        
        }
        close(m_epollFd);
        close(m_inotifyFd);
    });
}

void CGroupV2ReleaseNotification::handleEpollEvents(epoll_event *events, int eventNumber)
{
    for (int i = 0; i < eventNumber; i++) {
        if (events[i].data.fd != m_inotifyFd) {
            continue;
        }
        handleInotifyEvent();
    }
}

void CGroupV2ReleaseNotification::handleInotifyEvent()
{
    char buffer[BUF_LEN];
    ssize_t len = read(m_inotifyFd, buffer, BUF_LEN);
    if (len < 0) {
        perror("read");
        return;
    }

    struct inotify_event *event = nullptr;
    for (char *ptr = buffer; ptr < buffer + len; 
        ptr += sizeof(inotify_event) + event->len) {
        event = (inotify_event *)ptr;

        if (event->mask & IN_MODIFY && strcmp(event->name, "cgroup.events") == 0) {
            handleCGroupReleaseEvent(event);
        }
    }
}

void CGroupV2ReleaseNotification::handleCGroupReleaseEvent(inotify_event *event)
{
    std::string filePath = m_watchedPathInfo[event->wd].first;
    FILE *file = fopen(std::string(filePath + "/cgroup.events").c_str(), "r");
    if (!file) {
        return;
    }

    char content[1024];
    size_t bytesRead = fread(content, 1, sizeof(content), file);
    fclose(file);
    if (bytesRead <= 0) {
        return;
    }

    int populated = 1;
    sscanf(content, "populated %d", &populated);
    if (populated == 0) {
        std::lock_guard<std::mutex> locker(m_mutex);
        m_watchedPathInfo[event->wd].second(m_watchedPathInfo[event->wd].first);
        inotify_rm_watch(m_inotifyFd, event->wd);
        m_watchedPathInfo.erase(event->wd);
    }
}
