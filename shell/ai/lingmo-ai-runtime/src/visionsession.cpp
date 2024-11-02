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

#include "visionsession.h"
#include "modelmanager.h"
#include "imageloader.h"
#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include <iostream>
#include <unistd.h>
#include "token.h"
#include "cpr/api.h"
#include "util.h"
#include "servererror.h"
#include "baiduresponseprocessor.h"

namespace {

std::map<VisionImageStyle, std::string> imageStyleMap = {
    { EXPLORE_INFINITY, "探索无限" },
    { ANCIENT_STYLE, "古风" },
    { TWO_DIMENSIONAL, "二次元" },
    { REALISTIC_STYLE, "写实风格" },
    { UKIYO_E, "浮世绘" },
    { LOW_POLY, "low poly" },
    { FUTURISM, "未来主义" },
    { PIXEL_STYLE, "像素风格" },
    { CONCEPTUAL_ART, "概念艺术" },
    { CYBERPUNK, "赛博朋克" },
    { LOLITA_STYLE, "洛丽塔风格" },
    { BAROQUE_STYLE, "巴洛克风格" },
    { SURREALISM, "超现实主义" },
    { WATERCOLOR_PAINTING, "水彩画" },
    { VAPORWAVE_ART, "蒸汽波艺术" },
    { OIL_PAINTING, "油画" },
    { ARTOON_DRAWING, "卡通画" },
};

}

namespace vision {

const std::string& BaiduSession::getApiKey()
{
    ModelManager& modelManager = ModelManager::getInstance();
    const auto* baiduRemoteModel =
         dynamic_cast<const BaiduRemoteModel*>(modelManager.getRemoteModelByVendor(BAIDU));
    return baiduRemoteModel->getApiKey(VISION);
}

const std::string& BaiduSession::getSecretKey()
{
    ModelManager& modelManager = ModelManager::getInstance();
    const auto* baiduRemoteModel =
         dynamic_cast<const BaiduRemoteModel*>(modelManager.getRemoteModelByVendor(BAIDU));
    return baiduRemoteModel->getSecretKey(VISION);
}

BaiduSession::BaiduSession()
{
    initToken();
}

bool BaiduSession::isValid() const
{
    return !token_.empty();
}

void BaiduSession::setGenerateCallback(ImageResultCallback callback)
{
    generateCallback_ = std::move(callback);
}

void BaiduSession::setUserData(void* userData)
{
    userData_ = std::move(userData);
}

ImageResultCallback BaiduSession::getGenerateCallback()
{
    return generateCallback_;
}

void* BaiduSession::getUserData()
{
    return userData_;
}

void BaiduSession::setImageSize(int width, int height)
{
    imageWidth_ = width;
    imageHeight_ = height;
}

const int BaiduSession::imageWidth() const
{
    return imageWidth_;
}

const int BaiduSession::imageHeight() const
{
    return imageHeight_;
}

void BaiduSession::setImageNumber(int number)
{
    imageNumber_ = number;
}

void BaiduSession::setImageStyle(VisionImageStyle style)
{
    imageStyle_ = imageStyleMap[style];
}

const char* BaiduSession::getSpecification() const
{
    ModelManager& modelManager = ModelManager::getInstance();
    const auto* baiduRemoteModel =
         dynamic_cast<const BaiduRemoteModel*>(modelManager.getRemoteModelByVendor(BAIDU));
    int version = baiduRemoteModel->getVersion(VISION);

    if(version == 2) {
        return u8R"(
        {
            "number": 8,
            "size": [
                {"width":512,"height":512},{"width":640,"height":360},{"width":360,"height":640},
                {"width":1024,"height":1024},{"width":1280,"height":720},{"width":720,"height":1280},
                {"width":2048,"height":2048},{"width":2560,"height":1440},{"width":1440,"height":2560}
            ]
        }
        )";
    }
    if(version == 1) {
        return u8R"(
        {
            "number": 6,
            "size": [
                {"width":1024,"height":1024},{"width":1024,"height":1536},{"width":1536,"height":1024}
            ]
        }
        )";
    }
    return "";
}

void BaiduSession::setResultCode(VisionResult result)
{
    resultCode_ = result;
}

VisionResult BaiduSession::getResultCode()
{
    return resultCode_;
}

const std::string BaiduSession::lastErrorMessage() const
{
    return lastErrorMessage_;
}

VisionResult vision::BaiduSession::generateImage(const char *prompt)
{
    ModelManager& modelManager = ModelManager::getInstance();
    const auto* baiduRemoteModel =
         dynamic_cast<const BaiduRemoteModel*>(modelManager.getRemoteModelByVendor(BAIDU));
    int version = baiduRemoteModel->getVersion(VISION);
    setResultCode(VISION_SUCCESS);
    
    std::string taskId;
    if (version == 2) {
        taskId = requestGenerateImageTaskV2(prompt);
    }
    else {
        taskId = requestGenerateImageTaskV1(prompt);
    }
    if (taskId.empty()) {
        return resultCode_;
    }

    if (version == 2) {
        return processGenerateImageUrl(taskId);
    }
    else {
        auto imageUrls = requestGenerateImageUrl(taskId);
        if (imageUrls.empty()) {
            return resultCode_;
        }
        downloadImage(imageUrls);
    }

    return VISION_SUCCESS;
}

void BaiduSession::initToken()
{
    const std::string apiKey = vision::BaiduSession::getApiKey();
    const std::string secretKey = vision::BaiduSession::getSecretKey();
    setResultCode(VISION_SUCCESS);
    bool hasNetError = false;
    token_ = token::getBaiduToken(apiKey, secretKey, hasNetError, lastErrorMessage_);
    if (token_.empty()) {
        resultCode_ = hasNetError ? VISION_NET_ERROR : VISION_UNAUTHORIZED;
        fprintf(stderr, "Get baidu token failed.\n");
    }
}

std::string BaiduSession::requestGenerateImageTaskV1(const char *prompt)
{
    const std::string postUrl {
        "https://aip.baidubce.com/rpc/2.0/ernievilg/v1/txt2img?access_token=" + token_
    };

    Json::FastWriter writer;
    Json::Value obj;
    obj["text"] = prompt;
    if (imageStyle_.empty()) {
        obj["style"] = "写实风格";
    } else {
        obj["style"] = imageStyle_;
    }
    obj["resolution"] = std::to_string(imageWidth_) + "*" + std::to_string(imageHeight_);
    obj["num"] = imageNumber_;
    std::string body = writer.write(obj);

    cpr::Url url(postUrl);
    cpr::Response r = cpr::Post(
        url, cpr::Header {{ "Content-Type", "application/json"}, { "Accept", "application/json" }},
        cpr::Body { body });

    if (r.error.code != cpr::ErrorCode::OK) {
        lastErrorMessage_ = r.error.message;
        resultCode_ = VISION_NET_ERROR;
        fprintf(stderr, "Net error: %d.\n", (int)r.error.code);
        return std::string();
    }
    Json::Value result = util::formatJsonFromString(r.text);
    if (result.isMember("data") && result["data"].isMember("taskId")) {
        return result["data"]["taskId"].asString();
    }

    if (token::isBaiduTokenExpiredByResult(result)) {
        initToken();
        return requestGenerateImageTaskV1(prompt);
    }

    int serverErrorCode = server_error::parseErrorCode(r.text);
    if (serverErrorCode > 0) {
        lastErrorMessage_ = r.text;
        resultCode_ = server_error::errorCode2visionResult(serverErrorCode);
        fprintf(stderr, "Get baidu json result failed %s.\n", r.text.c_str());
    }

    return std::string();
}

std::string BaiduSession::requestGenerateImageTaskV2(const char *prompt)
{
    const std::string postUrl {
        "https://aip.baidubce.com/rpc/2.0/ernievilg/v1/txt2imgv2?access_token=" + token_
    };

    Json::FastWriter writer;
    Json::Value obj;
    if (imageStyle_.empty()) {
        obj["prompt"] = std::string(prompt);
    }
    else if (imageStyle_ == "low poly") {
        obj["prompt"] = std::string(prompt) + "，" + "低多边形";
    }
    else {
        obj["prompt"] = std::string(prompt) + "，" + imageStyle_;
    }
    obj["width"] = imageWidth_;
    obj["height"] = imageHeight_;
    obj["image_num"] = imageNumber_;
    std::string body = writer.write(obj);

    cpr::Url url(postUrl);
    cpr::Response r = cpr::Post(
        url, cpr::Header {{ "Content-Type", "application/json"}, { "Accept", "application/json" }},
        cpr::Body { body });

    if (r.error.code != cpr::ErrorCode::OK) {
        lastErrorMessage_ = r.error.message;
        resultCode_ = VISION_NET_ERROR;
        fprintf(stderr, "Net error: %d.\n", (int)r.error.code);
        return std::string();
    }
    Json::Value result = util::formatJsonFromString(r.text);
    if (result.isMember("data") && result["data"].isMember("task_id")) {
        return result["data"]["task_id"].asString();
    }

    if (token::isBaiduTokenExpiredByResult(result)) {
        initToken();
        return requestGenerateImageTaskV2(prompt);
    }

    int serverErrorCode = server_error::parseErrorCode(r.text);
    if (serverErrorCode > 0) {
        lastErrorMessage_ = r.text;
        resultCode_ = server_error::errorCode2visionResult(serverErrorCode);
        fprintf(stderr, "Get baidu json result failed %s.\n", r.text.c_str());
    }

    return std::string();
}

std::vector<std::string> BaiduSession::requestGenerateImageUrl(const std::string &taskId)
{
    while (1) {
        const std::string taskResult = doRequestGenerateImageUrlV1(taskId);
        if (taskResult.empty()) {
            return {};
        }
        Json::Value result = util::formatJsonFromString(taskResult);
        if (!result.isMember("data")) {
            lastErrorMessage_ = "Can not find the 'data' key from result: " + taskResult + ".";
            resultCode_ = VISION_REQUEST_FAILED;
            return {};
        }

        if (!result["data"].isMember("status")) {
            lastErrorMessage_ = "Can not find the 'status' key from result: " + taskResult + ".";
            resultCode_ = VISION_REQUEST_FAILED;
            return {};
        }

        if (result["data"]["status"].asInt() == 1) {
            if (!result["data"].isMember("imgUrls")) {
                lastErrorMessage_ = "Can not find the 'imgUrls' key from result: " + taskResult + ".";
                resultCode_ = VISION_REQUEST_FAILED;
                return {};
            }

            if (!result["data"]["imgUrls"].isArray()) {
                lastErrorMessage_ = "The 'imgUrls' object type is not Array: " + taskResult + ".";
                resultCode_ = VISION_REQUEST_FAILED;
                return {};
            }

            std::vector<std::string> imageUrls;
            Json::Value imageUrlList = result["data"]["imgUrls"];
            for (int i=0; i<imageUrlList.size(); ++i) {
                const std::string imageUrl = imageUrlList[i].isMember("image") ?
                    imageUrlList[i]["image"].asString() : "";
                imageUrls.emplace_back(imageUrl);
            }
            return imageUrls;
        }

        sleep(1);
    }
}

VisionResult BaiduSession::processGenerateImageUrl(const std::string &taskId)
{
    while (1) {
        const std::string taskResult = doRequestGenerateImageUrlV2(taskId);
        ImageDataList list;
        bool isTaskBlocked = false;
        ProcessResult resultCode = BaiduResponseProcessor::process(taskResult, list, isTaskBlocked, lastErrorMessage_);
        if (resultCode == ProcessResult::SUCCEEDED) {
            for(auto& imageData: list) {
                auto imageInfo =
                    vision::image_loader::getImageInfoFromData(imageData.data_.data(), imageData.data_.size());
                _ImageData data{(VisionImageFormat)imageInfo.format, imageInfo.height, imageInfo.width,
                                (int)list.size(), (int)imageData.index_, imageData.data_.data(), (int)imageData.data_.size(),
                    imageData.conclusion_==ApproveConclusion::PASS?VISION_SUCCESS:VISION_GENERATE_IMAGE_BLOCKED};
                generateCallback_(data, userData_);
            }
            return VISION_SUCCESS;
        }
        if (resultCode == ProcessResult::FAILED) {
            return isTaskBlocked ? VISION_GENERATE_IMAGE_BLOCKED : VISION_REQUEST_FAILED;
        }
        sleep(1);
    }
}

std::string BaiduSession::doRequestGenerateImageUrlV1(const std::string &taskId)
{
    const std::string postUrl {
        "https://aip.baidubce.com/rpc/2.0/ernievilg/v1/getImg?access_token=" + token_
    };

    char data[1024];
    snprintf(data, sizeof(data), "{\"taskId\":\"%s\"}", taskId.c_str());

    cpr::Url url(postUrl);
    cpr::Response r = cpr::Post(
        url, cpr::Header {{ "Content-Type", "application/json"}, { "Accept", "application/json" }},
        cpr::Body { data });

    if (r.error.code != cpr::ErrorCode::OK) {
        lastErrorMessage_ = r.error.message;
        resultCode_ = VISION_NET_ERROR;
        return std::string();
    }

    if (token::isBaiduTokenExpiredByResult(util::formatJsonFromString(r.text))) {
        initToken();
        return doRequestGenerateImageUrlV1(taskId);
    }

    int serverErrorCode = server_error::parseErrorCode(r.text);
    if (serverErrorCode > 0) {
        lastErrorMessage_ = r.text;
        resultCode_ = server_error::errorCode2visionResult(serverErrorCode);
        fprintf(stderr, "Get baidu json result failed %s.\n", r.text.c_str());
    }

    return r.text;
}

std::string BaiduSession::doRequestGenerateImageUrlV2(const std::string &taskId)
{
    const std::string postUrl {
        "https://aip.baidubce.com/rpc/2.0/ernievilg/v1/getImgv2?access_token=" + token_
    };

    char data[1024];
    snprintf(data, sizeof(data), "{\"task_id\":\"%s\"}", taskId.c_str());

    cpr::Url url(postUrl);
    cpr::Response r = cpr::Post(
        url, cpr::Header {{ "Content-Type", "application/json"}, { "Accept", "application/json" }},
        cpr::Body { data });

    if (r.error.code != cpr::ErrorCode::OK) {
        lastErrorMessage_ = r.error.message;
        resultCode_ = VISION_NET_ERROR;
        return std::string();
    }

    if (token::isBaiduTokenExpiredByResult(util::formatJsonFromString(r.text))) {
        initToken();
        return doRequestGenerateImageUrlV2(taskId);
    }

    int serverErrorCode = server_error::parseErrorCode(r.text);
    if (serverErrorCode > 0) {
        lastErrorMessage_ = r.text;
        resultCode_ = server_error::errorCode2visionResult(serverErrorCode);
        fprintf(stderr, "Get baidu json result failed %s.\n", r.text.c_str());
    }

    return r.text;
}

void BaiduSession::downloadImage(const std::vector<std::string> imageUrls)
{
    for (int imageIndex=0; imageIndex<imageNumber_; ++imageIndex) {
        std::vector<unsigned char> imageData;
        if (imageIndex < imageUrls.size()) {
            imageData = vision::image_loader::downloadImage(imageUrls.at(imageIndex), lastErrorMessage_);
        }
        image_loader::ImageBaseInfo imageInfo =
            image_loader::getImageInfoFromData(imageData.data(), imageData.size());

        int errorCode = imageData.empty() ? VISION_GENERATE_IMAGE_BLOCKED : VISION_SUCCESS;
        VisionImageData data { (VisionImageFormat)imageInfo.format, imageInfo.width, imageInfo.height, imageNumber_,
                             imageIndex, imageData.data(), (int)imageData.size(), errorCode };
        generateCallback_(data, userData_);
    }
}

}
