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

#ifndef VISIONSESSION_H
#define VISIONSESSION_H

#include <string>
#include <memory>
#include "baiduremotemodel.h"
#include "vision.h"

namespace vision {
    
class BaiduSession
{
public:
    BaiduSession();
    bool isValid() const;

    void setGenerateCallback(ImageResultCallback callback);
    void setUserData(void* userData);

    ImageResultCallback getGenerateCallback();
    void* getUserData();

    VisionResult generateImage(const char *prompt);

    static const std::string& getApiKey();
    static const std::string& getSecretKey();

    void setImageSize(int width, int height);
    void setImageNumber(int number);

    void setImageStyle(VisionImageStyle style);

    const char* getSpecification() const;

    const int imageWidth() const;
    const int imageHeight() const;

    void setResultCode(VisionResult result);

    VisionResult getResultCode();
    const std::string lastErrorMessage() const;

private:
    void initToken();

    std::string requestGenerateImageTaskV1(const char *prompt);
    std::string requestGenerateImageTaskV2(const char *prompt);
    std::vector<std::string> requestGenerateImageUrl(const std::string &taskId);
    VisionResult processGenerateImageUrl(const std::string &taskId);
    std::string doRequestGenerateImageUrlV1(const std::string &taskId);
    std::string doRequestGenerateImageUrlV2(const std::string &taskId);
    void downloadImage(const std::vector<std::string> imageUrls);

private:
    int imageWidth_ { 1024 };
    int imageHeight_ { 1024 };
    int imageNumber_ { 1 };
    std::string imageStyle_;
    const std::string url_ { "https://vop.baidu.com/pro_api" };
    const int devPid_ { 80001 };
    const std::string scope_ { "brain_enhanced_asr" };
    ImageResultCallback generateCallback_;
    void* userData_;
    VisionResult resultCode_ {VISION_SUCCESS};
    std::string token_;
    std::string lastErrorMessage_;
};

}


#endif // SPEECHSESSION_H
