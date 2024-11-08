/*
 * Copyright 2024 KylinSoft Co., Ltd.
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

#ifndef CLOUD_SPEECH_ENGINE_H
#define CLOUD_SPEECH_ENGINE_H

#include "speechengine.h"

namespace ai_engine {
namespace lm {
namespace speech {

class AbstractCloudSpeechEngine : public AbstractSpeechEngine {
public:
    virtual ~AbstractCloudSpeechEngine() {}
    bool isCloud() override final { return true; }
    virtual void setConfig(const std::string &config) = 0;
};
}

}
}

#endif
