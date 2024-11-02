#ifndef SERVICES_VISIONPROCESSORPROXY_H
#define SERVICES_VISIONPROCESSORPROXY_H

#include <functional>
#include <string>

#include "services/visionprocessorglue.h"
#include <lingmo-ai/ai-base/vision.h>

class VisionProcessorProxy {
public:
    VisionProcessorProxy();
    ~VisionProcessorProxy();

    void setPromptImageCallback(ImageResultCallback callback) {
        promptImageCallback_ = callback;
    }

    void initEngine(gchar *engineName, gchar *appId,
                    gchar *apiKey, gchar *secretKey, gpointer userData);
    void setPromptImageSize(int width, int height);
    void setPromptImageNumber(int number);
    void setPromptImageStyle(VisionImageStyle style);
    void promptImage(const char* prompt);

private:
    void init();
    void destroy();

    friend void onPromptImageResult(GDBusConnection *connection,
                                    const gchar *sender_name,
                                    const gchar *object_path,
                                    const gchar *interface_name,
                                    const gchar *signal_name,
                                    GVariant *parameters,
                                    gpointer user_data);

    static void printErrorMessage(GError *error, std::string callMethodName);

    static void onInitEngineCallback(AisdkVisionProcessor *delegate,
                                     GAsyncResult *result, gpointer userData);
    static void onPromptImageSizeCallback(AisdkVisionProcessor *delegate,
                                          GAsyncResult *result, gpointer userData);
    static void onPromptImageNumberCallback(AisdkVisionProcessor *delegate,
                                            GAsyncResult *result, gpointer userData);
    static void onPromptImageStyleCallback(AisdkVisionProcessor *delegate,
                                           GAsyncResult *result, gpointer userData);
    static void onPromptImageCallback(AisdkVisionProcessor *delegate,
                                      GAsyncResult *result, gpointer userData);

private:
    AisdkVisionProcessor *delegate_ = nullptr;

    ImageResultCallback promptImageCallback_ = nullptr;

    const char *objectPath_ = "/org/lingmo/aisdk/visionprocessor";
    const char *interfaceName_ = "org.lingmo.aisdk.visionprocessor";
};
#endif
