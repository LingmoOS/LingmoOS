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

#ifndef DATACOLLECTOR_H
#define DATACOLLECTOR_H

#include <future>

class DataCollector
{
public:
    DataCollector() = default;
    void collectResouceLimitedEnebaledChanged(bool enabled);
    void collectSoftFreezeModeEnabledChanged(bool enabled);

private:
    struct CollectData {
        std::string key;
        std::string value;
    };

private:
    void collectData(CollectData data);

private:
    std::future<void> m_collectFuture;
};

#endif // DATACOLLECTOR_H
