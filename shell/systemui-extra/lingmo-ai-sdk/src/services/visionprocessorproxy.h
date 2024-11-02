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

#ifndef SERVICES_VISIONPROCESSORPROXY_H
#define SERVICES_VISIONPROCESSORPROXY_H

#include <functional>
#include <string>

#include "services/visionprocessorglue.h"
#include <lingmo-ai/ai-base/vision.h>
#include <jsoncpp/json/json.h>

class VisionProcessorProxy {
public:
    using PromptImageResultCallback = std::function<void(VisionImageData result)>;

    VisionProcessorProxy();
    ~VisionProcessorProxy();

    void setPromptImageCallback(PromptImageResultCallback callback) {
        promptImageCallback_ = callback;
    }

    VisionResult initEngine(const std::string &engineName, const Json::Value &config);
    void setPromptImageSize(int width, int height);
    void setPromptImageNumber(int number);
    void setPromptImageStyle(VisionImageStyle style);
    std::string getPrompt2imageSupportedParams() const;
    VisionResult promptImage(const char* prompt);

private:
    void init();
    void destroy();
    void connectSignal();

    friend void onPromptImageResult(GDBusConnection *connection,
                                    const gchar *sender_name,
                                    const gchar *object_path,
                                    const gchar *interface_name,
                                    const gchar *signal_name,
                                    GVariant *parameters,
                                    gpointer user_data);

private:
    AisdkVisionProcessor *delegate_ = nullptr;

    PromptImageResultCallback promptImageCallback_ = nullptr;

    const char *objectPath_ = "/org/lingmo/aisdk/visionprocessor";
    const char *interfaceName_ = "org.lingmo.aisdk.visionprocessor";

    int sessionId_;
    int initErrorCode_; //0表示初始化成功，非0表示初始化失败
};
#endif
