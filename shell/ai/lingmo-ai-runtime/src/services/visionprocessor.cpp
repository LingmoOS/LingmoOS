#include "services/visionprocessor.h"

#include <iostream>
#include <lingmo-ai/ai-engine/large-model/cloudvisionengine.h>
#include "services/threadpool/async.h"
#include "defaultkeymanager/defaultkeymanager.h"

const std::string VisionProcessor::objectPath_ =
    "/org/lingmo/aisdk/visionprocessor";
std::set<int> VisionProcessor::sessionIdSet;

namespace  {

std::map<ai_engine::lm::VisionEngineErrorCode, int> visionEngineErrorMap {
    { ai_engine::lm::VisionEngineErrorCode::NomoreMemory, 3 },
    { ai_engine::lm::VisionEngineErrorCode::FileNotFound, 3 },
    { ai_engine::lm::VisionEngineErrorCode::VersionMismatched, 3 },
    { ai_engine::lm::VisionEngineErrorCode::FilePermissionDenied, 4 },
    { ai_engine::lm::VisionEngineErrorCode::NetworkDisconnected, 2 },
    { ai_engine::lm::VisionEngineErrorCode::NetworkLatencyTooLong, 2 },
    { ai_engine::lm::VisionEngineErrorCode::FailedToConnectServer, 3 },
    { ai_engine::lm::VisionEngineErrorCode::ServerNoResponse, 3 },
    { ai_engine::lm::VisionEngineErrorCode::NoServerService, 3 },
    { ai_engine::lm::VisionEngineErrorCode::Unauthorized, 4 },
    { ai_engine::lm::VisionEngineErrorCode::TooLittleParams, 8 },
    { ai_engine::lm::VisionEngineErrorCode::TooManyParams, 8 },
    { ai_engine::lm::VisionEngineErrorCode::InvalidParamType, 11 },
    { ai_engine::lm::VisionEngineErrorCode::TooLittleData, 8 },
    { ai_engine::lm::VisionEngineErrorCode::TooManyData, 8 },
    { ai_engine::lm::VisionEngineErrorCode::TooFrequentRequest, 5 },
    { ai_engine::lm::VisionEngineErrorCode::TooManyRequest, 5 },
    { ai_engine::lm::VisionEngineErrorCode::Invalidwords, 11 },
    { ai_engine::lm::VisionEngineErrorCode::ImageGenerationFailed, 9 },
    { ai_engine::lm::VisionEngineErrorCode::ImageCensorshipFailed, 10 }
};

}

VisionProcessor::VisionProcessor(
    GDBusConnection &connection,
    ai_engine::AiEnginePluginManager &aiEnginePluginManager)
    : connection_(connection), aiEnginePluginManager_(aiEnginePluginManager) {
    exportSkeleton();
}

VisionProcessor::~VisionProcessor() { unexportSkeleton(); }

void VisionProcessor::exportSkeleton() {
    delegate_ = aisdk_vision_processor_skeleton_new();
    connectSignal();

    GError *error = nullptr;

    isExported_ = g_dbus_interface_skeleton_export(
        G_DBUS_INTERFACE_SKELETON(delegate_), &connection_, objectPath_.c_str(),
        &error);
    if (!isExported_) {
        g_printerr("Error creating server at address %s: %s\n",
                   objectPath_.c_str(), error->message);
        g_error_free(error);
    }
}

bool VisionProcessor::initEngine(const std::string &engineName,
    const std::string &config, gint sessionId) {
    auto visionEngine = aiEnginePluginManager_.createVisionEngine(engineName);
    if (!visionEngine) {
        fprintf(stderr, "init engine error: visionEngine is nullptr !");
        return false;
    }

    visionEngineMap_[sessionId] = std::move(visionEngine);
    if (visionEngineMap_.at(sessionId)->isCloud()) {
        static_cast<ai_engine::lm::vision::AbstractCloudVisionEngine *>(
                    visionEngineMap_.at(sessionId).get())->setConfig(config);
    }
    return true;
}

void VisionProcessor::setPromptImageCallback(gint sessionId) {
    visionEngineMap_.at(sessionId)->setPrompt2ImageCallback(
                [this, sessionId](ai_engine::lm::vision::ImageData imageData){
        std::cout << "service: onHandlePromptImage callback"
                  << " data " << imageData.imageData.size()
                  << " width " << imageData.width
                  << " sessionId " << sessionId << std::endl;
        sendText2ImageResult(sessionId, imageData);
    });
}

void VisionProcessor::sendText2ImageResult(
    gint sessionId, const ai_engine::lm::vision::ImageData &imageData, bool limited) {

    int errorCode = limited ? 4 : getSdkErrorCode(imageData.error.getCode());

    if (useDefaultKey_ && !limited) {
        DefaultKeyManager::instance().updateText2ImageCount(imageData.total);
    }

    {
        std::lock_guard<std::mutex> locker(mutex_);
        if (stopped) {
            return;
        }
    }

    // 创建包含字节数组的 GVariant
    GVariant *byte_array_variant = g_variant_new_from_data(
        G_VARIANT_TYPE_BYTESTRING,
        imageData.imageData.data(),
        imageData.imageData.size(),
        TRUE, NULL, NULL);

    GVariantBuilder builder;
    g_variant_builder_init (&builder, G_VARIANT_TYPE_TUPLE);
    g_variant_builder_open (&builder, G_VARIANT_TYPE_ARRAY);
    g_variant_builder_add_value(&builder, byte_array_variant);
    g_variant_builder_close (&builder);

    g_variant_builder_add(&builder, "i", imageData.format);
    g_variant_builder_add(&builder, "i", imageData.height);
    g_variant_builder_add(&builder, "i", imageData.width);
    g_variant_builder_add(&builder, "i", imageData.total);
    g_variant_builder_add(&builder, "i", imageData.index);
    g_variant_builder_add(&builder, "i", errorCode);

    std::shared_ptr<VisionProcessor> visionProcessor =
            static_cast<VisionProcessor *>(this)->getSharedObject();
    std::string interfaceName = "org.lingmo.aisdk.visionprocessor" +
            std::to_string((int)sessionId);

    g_dbus_connection_emit_signal(&(visionProcessor->connection_),
                                    "org.lingmo.aisdk.visionprocessor",
                                    "/org/lingmo/aisdk/visionprocessor",
                                    interfaceName.c_str(),
                                    "PromptImageResult",
                                    g_variant_builder_end(&builder),
                                    nullptr);
}

bool onHandleInitEngine(AisdkVisionProcessor *delegate,
    GDBusMethodInvocation *invocation, gchar *engineName, gchar *config,
    gpointer userData) {
    auto *visionProcessor = static_cast<VisionProcessor *>(userData);
    if (!visionProcessor) {
        fprintf(stderr, "handle init engine: visionProcessor is nullptr!\n");
        aisdk_vision_processor_complete_init(delegate, invocation, 0,
            visionEngineErrorMap.at(ai_engine::lm::VisionEngineErrorCode::NomoreMemory));
        return true;
    }

    std::string realEngineName{engineName};
    std::string realConfig{config};
    if (realEngineName == "default") {
        visionProcessor->useDefaultKey_ = true;
        realEngineName = "xunfei";
        realConfig = DefaultKeyManager::instance().defaultVisionKey();
    } else {
        visionProcessor->useDefaultKey_ = false;
    }

    int sessionId = visionProcessor->generateRandomSessionId();
    bool initResult = visionProcessor->initEngine(
                realEngineName, realConfig, sessionId);

    if (!initResult) {
        aisdk_vision_processor_complete_init(delegate, invocation, sessionId,
            visionEngineErrorMap.at(ai_engine::lm::VisionEngineErrorCode::NomoreMemory));
        return true;
    }
    fprintf(stdout, "service: The sessionId of the engine is %d\n", sessionId);
    visionProcessor->setPromptImageCallback(sessionId);
    aisdk_vision_processor_complete_init(delegate, invocation, sessionId, 0);
    return true;
}

bool onHandleSetPromptImageSize(AisdkVisionProcessor *delegate,
                                GDBusMethodInvocation *invocation,
                                gint imageWidth, gint imageHeight,
                                gint sessionId, gpointer userData) {
    auto *visionProcessor = static_cast<VisionProcessor *>(userData);
    if (!visionProcessor ||
            visionProcessor->visionEngineMap_.find(sessionId) ==
            visionProcessor->visionEngineMap_.end()) {
        fprintf(stderr, "set prompt image size error: visionProcessor or visionEngine is nullptr!\n");
        return true;
    }

    visionProcessor->visionEngineMap_.at(sessionId)
            ->setPrompt2ImageSize(imageWidth, imageHeight);
    return true;
}

bool onHandleSetPromptImageNumber(AisdkVisionProcessor *delegate,
                                  GDBusMethodInvocation *invocation,
                                  gint imageNumber, gint sessionId,
                                  gpointer userData) {
    auto *visionProcessor = static_cast<VisionProcessor *>(userData);
    if (!visionProcessor ||
            visionProcessor->visionEngineMap_.find(sessionId) ==
            visionProcessor->visionEngineMap_.end()) {
        fprintf(stderr, "set prompt image number error: visionProcessor or visionEngine is nullptr!\n");
        return true;
    }

    visionProcessor->visionEngineMap_.at(sessionId)
            ->setPrompt2ImageNumber(imageNumber);
    return true;
}

bool onHandleSetPromptImageStyle(AisdkVisionProcessor *delegate,
                                 GDBusMethodInvocation *invocation,
                                 gint imageStyle, gint sessionId,
                                 gpointer userData) {
    auto *visionProcessor = static_cast<VisionProcessor *>(userData);
    if (!visionProcessor ||
            visionProcessor->visionEngineMap_.find(sessionId) ==
            visionProcessor->visionEngineMap_.end()) {
        fprintf(stderr, "set prompt image style error: visionProcessor or visionEngine is nullptr!\n");
        return true;
    }

    visionProcessor->visionEngineMap_.at(sessionId)
            ->setPrompt2ImageStyle(imageStyle);
    return true;
}

bool onHandleGetPrompt2ImageSupportedParams(AisdkVisionProcessor *delegate,
                                            GDBusMethodInvocation *invocation,
                                            gint sessionId, gpointer userData) {
    auto *visionProcessor = static_cast<VisionProcessor *>(userData);
    if (!visionProcessor ||
        visionProcessor->visionEngineMap_.find(sessionId) ==
            visionProcessor->visionEngineMap_.end()) {
        fprintf(stderr, "set prompt to image supported params error: visionProcessor or visionEngine is nullptr!\n");
        return true;
    }

    std::string params = visionProcessor->visionEngineMap_.at(sessionId)
        ->getPrompt2ImageSupportedParams();
    aisdk_vision_processor_complete_get_prompt2_image_supported_params(delegate, invocation, params.c_str());
    return true;
}

bool onHandlePromptImage(AisdkVisionProcessor *delegate,
                         GDBusMethodInvocation *invocation,
                         const gchar *prompt, gint sessionId,
                         gpointer userData) {
    std::shared_ptr<VisionProcessor> visionProcessor =
            static_cast<VisionProcessor *>(userData)->getSharedObject();
    if (!visionProcessor ||
            visionProcessor->visionEngineMap_.find(sessionId) ==
                                visionProcessor->visionEngineMap_.end()) {
        fprintf(stderr, "prompt image error: visionProcessor or visionEngine is nullptr!\n");
        return true;
    }

    if (visionProcessor->useDefaultKey_ && DefaultKeyManager::instance().isText2ImageLimit()) {
        visionProcessor->sendText2ImageResult(sessionId, ai_engine::lm::vision::ImageData{}, true);
        return true;
    }

    std::string promptString{prompt};
    cpr::async([visionProcessor, promptString, sessionId]() {
        ai_engine::lm::EngineError engineError;
        bool success = visionProcessor->visionEngineMap_.at(sessionId)
                ->prompt2Image(promptString, engineError);
        if (!success) {
            fprintf(stderr, "service error messages: %s\n", engineError.getMessage().c_str());
        }
    });
    return true;
}

void VisionProcessor::connectSignal() {
    g_signal_connect(delegate_, "handle-init",
                     G_CALLBACK(onHandleInitEngine),
                     this);
    g_signal_connect(delegate_, "handle-set-prompt-image-size",
                     G_CALLBACK(onHandleSetPromptImageSize),
                     this);
    g_signal_connect(delegate_, "handle-set-prompt-image-number",
                     G_CALLBACK(onHandleSetPromptImageNumber),
                     this);
    g_signal_connect(delegate_, "handle-set-prompt-image-style",
                     G_CALLBACK(onHandleSetPromptImageStyle),
                     this);
    g_signal_connect(delegate_, "handle-get-prompt2-image-supported-params",
                     G_CALLBACK(onHandleGetPrompt2ImageSupportedParams),
                     this);
    g_signal_connect(delegate_, "handle-prompt-image",
                     G_CALLBACK(onHandlePromptImage),
                     this);
}

void VisionProcessor::unexportSkeleton() {
    if (delegate_ == nullptr) {
        return;
    }

    if (isExported_) {
        g_dbus_interface_skeleton_unexport_from_connection(
            G_DBUS_INTERFACE_SKELETON(delegate_), &connection_);
    }

    g_object_unref(delegate_);
}

int VisionProcessor::generateRandomSessionId() {
    int sessionId = std::rand();
    while (sessionIdSet.find(sessionId) != sessionIdSet.end()) {
        sessionId = std::rand();
    }
    sessionIdSet.insert(sessionId);
    return sessionId;
}

int VisionProcessor::getSdkErrorCode(int engineErrorCode) {
    if (engineErrorCode == -1) {
        return 0;
    } else if (engineErrorCodeExists(engineErrorCode)) {
        return visionEngineErrorMap.at((ai_engine::lm::VisionEngineErrorCode)engineErrorCode);
    } else {
        fprintf(stderr, "service error messages: speech engine error code is not exists!\n");
        return 0;
    }
}

bool VisionProcessor::engineErrorCodeExists(int errorCode) {
    ai_engine::lm::VisionEngineErrorCode code =
            (ai_engine::lm::VisionEngineErrorCode)errorCode;
    return visionEngineErrorMap.find(code) != visionEngineErrorMap.end();
}


