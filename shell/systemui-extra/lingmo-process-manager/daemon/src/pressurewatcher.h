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

#ifndef PRESSUREWATCHER_H
#define PRESSUREWATCHER_H

#include "resourcewatcher.h"
#include "resource.h"

//! \keyword urgency \value PSI trigger
typedef std::map<resource::ResourceUrgency, std::string> PsiResourceUrgencyThreshold;

struct PsiTrigger
{
    PsiTrigger(const std::string& theResource = std::string(),
               resource::ResourceUrgency theUrgency = resource::ResourceUrgency::Unknown,
               std::string theTrigger = std::string(), int theFd = -1)
        : resource(theResource)
        , urgency(theUrgency)
        , trigger(theTrigger)
        , fd(theFd)
    {}

    std::string resource;
    resource::ResourceUrgency urgency;
    std::string trigger; // psi trigger see https://www.kernel.org/doc/html/latest/accounting/psi.html
    int fd;
};

class epoll_event;
class PressureWatcher : public ResourceWatcher
{

    Q_OBJECT
public:
    explicit PressureWatcher(const std::map<resource::Resource, resource::ResourceUrgencyThreshold>& resourceThresholds,
                             QObject *parent = nullptr);

public Q_SLOTS:
    void start() override;
    void stop() override { m_stop = true; }

private:
    void initPsiTriggers(const std::map<resource::Resource, resource::ResourceUrgencyThreshold>& resourceThresholds);
    void receiveEpollMessage(int epfd);
    std::vector<PsiTrigger> handleEpollMessage(const epoll_event* events, int eventNum) const;
    void handlePsiMessage(const std::vector<PsiTrigger>& triggers);

    std::string mapPressureValueToPsiTrigger(int pressure) const;
    int registerPsiTriggers();
    bool addToEpoll(int epfd, int fd) const;
    PsiTrigger findTriggerByFd(int fd) const;
    int containResource(std::vector<PsiTrigger> triggers, const std::string& resource) const;
    void closeTriggers();

    bool m_stop;
    std::vector<PsiTrigger> m_triggers;
};

#endif // PRESSUREWATCHER_H
