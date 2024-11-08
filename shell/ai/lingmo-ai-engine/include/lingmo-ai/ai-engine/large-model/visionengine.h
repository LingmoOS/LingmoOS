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

#ifndef VISIONENGINE_H
#define VISIONENGINE_H

#include <string>
#include <functional>
#include "aiengineerror.h"

namespace ai_engine {
namespace lm {
namespace vision {

struct ImageData {
    int format;
    int height;
    int width;
    int total;  // 总共生成的图片数量
    int index; // 当前数据对应的图片的索引
    std::vector<char> imageData;
    EngineError error;
};

using ImageResultCallback = std::function<void (ImageData imageData)>;

class AbstractVisionEngine {
public:
    virtual ~AbstractVisionEngine() {}
    virtual std::string engineName() = 0;
    virtual bool isCloud() = 0;

    virtual void setPrompt2ImageCallback(ImageResultCallback callback) = 0;
    virtual void setPrompt2ImageSize(int width, int height) {}
    virtual void setPrompt2ImageNumber(int number) {}
    virtual void setPrompt2ImageStyle(int style) {}
    virtual std::string getPrompt2ImageSupportedParams() const = 0;

    virtual bool prompt2Image(const std::string &prompt, ai_engine::lm::EngineError &error) = 0;
};

}
}
}


#endif // VISIONENGINE_H
