#ifndef SERVICES_VISIONPROCESSOR_H
#define SERVICES_VISONPROCESSOR_H

#include <set>
#include <string>
#include <lingmo-ai/ai-engine/large-model/visionengine.h>
#include <memory>
#include <mutex>
#include "services/visionprocessorglue.h"
#include "engine/aienginepluginmanager.h"
#include "config/prompttemplate.h"

class VisionProcessor : public std::enable_shared_from_this<VisionProcessor> {
public:
    explicit VisionProcessor(
            GDBusConnection &connection,
            ai_engine::AiEnginePluginManager &aiEnginePluginManager);

    ~VisionProcessor();

private:
    void exportSkeleton();
    void connectSignal();
    void unexportSkeleton();
    int generateRandomSessionId();
    int getSdkErrorCode(int engineErrorCode);
    bool engineErrorCodeExists(int errorCode);

    bool initEngine(const std::string &engineName, const std::string &config,
        gint sessionId);
    void setPromptImageCallback(gint sessionId);
    void sendText2ImageResult(gint sessionId,
        const ai_engine::lm::vision::ImageData &imageData, bool limited = false);

    friend bool onHandleInitEngine(AisdkVisionProcessor *delegate,
       GDBusMethodInvocation *invocation, gchar *engineName, gchar *config,
       gpointer userData);
    friend bool onHandleSetPromptImageSize(AisdkVisionProcessor *delegate,
                                           GDBusMethodInvocation *invocation,
                                           gint imageWidth, gint imageHeight,
                                           gint sessionId, gpointer userData);
    friend bool onHandleSetPromptImageNumber(AisdkVisionProcessor *delegate,
                                             GDBusMethodInvocation *invocation,
                                             gint imageNumber, gint sessionId,
                                             gpointer userData);
    friend bool onHandleSetPromptImageStyle(AisdkVisionProcessor *delegate,
                                            GDBusMethodInvocation *invocation,
                                            gint imageStyle, gint sessionId,
                                            gpointer userData);
    friend bool onHandleGetPrompt2ImageSupportedParams(AisdkVisionProcessor *delegate,
                                            GDBusMethodInvocation *invocation,
                                            gint sessionId, gpointer userData);
    friend bool onHandlePromptImage(AisdkVisionProcessor *delegate,
                                    GDBusMethodInvocation *invocation,
                                    const gchar *prompt, gint sessionId,
                                    gpointer userData);

private:
    std::shared_ptr<VisionProcessor> getSharedObject() {
        return shared_from_this();
    }

private:
    bool isExported_ = false;
    AisdkVisionProcessor *delegate_ = nullptr;

    GDBusConnection &connection_;

    static const std::string objectPath_;

    ai_engine::AiEnginePluginManager &aiEnginePluginManager_;
    std::mutex mutex_;
    bool stopped { false };
    std::string currentPrompt_;
    int actorId_ { -1 };
    bool actorChanged { false };

    static std::set<int> sessionIdSet;
    typedef std::map<gint, std::unique_ptr<ai_engine::lm::vision::AbstractVisionEngine>>
    gintAndVisionEngineMap;

    gintAndVisionEngineMap visionEngineMap_;

    bool useDefaultKey_{false};
};

#endif
