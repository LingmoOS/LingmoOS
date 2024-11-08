#include "services/visionprocessorproxy.h"

#include <iostream>

#include "services/serverproxy.h"

void onPromptImageResult(GDBusConnection *connection,
                         const gchar *sender_name,
                         const gchar *object_path,
                         const gchar *interface_name,
                         const gchar *signal_name,
                         GVariant *parameters,
                         gpointer user_data) {
    printf("proxy---onPromptImageResult---\n");
    auto *proxy = static_cast<VisionProcessorProxy *>(user_data);

    GVariantIter iter;
    g_variant_iter_init(&iter, parameters);

    GVariant *array_variant = g_variant_iter_next_value(&iter); // 获取字节数组
    GVariant *int_format = g_variant_iter_next_value(&iter); // 获取整数
    GVariant *int_height = g_variant_iter_next_value(&iter);
    GVariant *int_width = g_variant_iter_next_value(&iter);
    GVariant *int_total = g_variant_iter_next_value(&iter);
    GVariant *int_index = g_variant_iter_next_value(&iter);
    GVariant *int_error = g_variant_iter_next_value(&iter);

    int *format = (int *)g_variant_get_data(int_format);
    int *height = (int *)g_variant_get_data(int_height);
    int *width = (int *)g_variant_get_data(int_width);
    int *total = (int *)g_variant_get_data(int_total);
    int *index = (int *)g_variant_get_data(int_index);
    int *error = (int *)g_variant_get_data(int_error);
    std::cout << "onPromptImageResult---format---" << *format
              << "---height---" << *height << "---width---" << *width
              << "---total---" << *total << "---index---" << *index
              << "---error---" << *error << std::endl;

    std::cout << "Type " << g_variant_get_type_string(array_variant) << std::endl;
    std::cout << "size " << g_variant_get_size(array_variant) << std::endl;

    gchar *arr = (gchar*)g_variant_get_data(array_variant);
    std::vector<char> vec { arr, arr + g_variant_get_size(array_variant) - 1 };

    VisionImageData visionImageData {
        (VisionImageFormat)*format, *height, *width, *total,
        *index, (unsigned char*)vec.data(), (int)vec.size(), *error };
    proxy->promptImageCallback_(visionImageData, nullptr);
}

VisionProcessorProxy::VisionProcessorProxy() { init(); }

VisionProcessorProxy::~VisionProcessorProxy() { destroy(); }

void VisionProcessorProxy::initEngine(gchar *engineName, gchar *appId,
                                      gchar *apiKey, gchar *secretKey, gpointer userData) {
    std::cout << "proxy---initEngine---" << std::endl;
    aisdk_vision_processor_call_init(
        delegate_, engineName, appId, apiKey, secretKey, nullptr,
        reinterpret_cast<GAsyncReadyCallback>(
            VisionProcessorProxy::onInitEngineCallback),
        this);
}

void VisionProcessorProxy::setPromptImageSize(int width, int height) {
    std::cout << "proxy---setPromptImageSize---" << std::endl;
    aisdk_vision_processor_call_set_prompt_image_size(
       delegate_, width, height, nullptr,
       reinterpret_cast<GAsyncReadyCallback>(
            VisionProcessorProxy::onPromptImageSizeCallback),
       this);
}

void VisionProcessorProxy::setPromptImageNumber(int number) {
    std::cout << "proxy---setPromptImageNumber---" << std::endl;
    aisdk_vision_processor_call_set_prompt_image_number(
        delegate_, number, nullptr,
        reinterpret_cast<GAsyncReadyCallback>(
            VisionProcessorProxy::onPromptImageNumberCallback),
        this);
}

void VisionProcessorProxy::setPromptImageStyle(VisionImageStyle style) {
    std::cout << "proxy---setPromptImageStyle---" << std::endl;
    aisdk_vision_processor_call_set_prompt_image_style(
        delegate_, style, nullptr,
        reinterpret_cast<GAsyncReadyCallback>(
            VisionProcessorProxy::onPromptImageStyleCallback),
        this);
}

void VisionProcessorProxy::promptImage(const char* prompt) {
    std::cout << "proxy---promptImage---" << std::endl;
    aisdk_vision_processor_call_prompt_image(
        delegate_, prompt, nullptr,
        reinterpret_cast<GAsyncReadyCallback>(
            VisionProcessorProxy::onPromptImageCallback),
        this);
}

void VisionProcessorProxy::init() {
    GError *error = nullptr;
    if (!ServerProxy::getInstance().available()) {
        g_printerr("Error creating vision processor proxy: Server proxy "
                   "connection is unavailable.\n");
        return;
    }

    auto *connection = ServerProxy::getInstance().getConnection();
    delegate_ = aisdk_vision_processor_proxy_new_sync(
        connection, G_DBUS_PROXY_FLAGS_NONE, nullptr, objectPath_,
        nullptr, &error);
    if (delegate_ == nullptr) {
        g_printerr("Error creating vision processor proxy %s: %s\n",
                   objectPath_, error->message);
        g_error_free(error);
    }

    g_dbus_connection_signal_subscribe(connection, nullptr,
        interfaceName_, "PromptImageResult", objectPath_, NULL,
        G_DBUS_SIGNAL_FLAGS_NONE, onPromptImageResult, this, nullptr);
}

void VisionProcessorProxy::destroy() {
    if (delegate_ != nullptr) {
        g_object_unref(delegate_);
    }
}

void VisionProcessorProxy::printErrorMessage(GError *error, std::string callMethodName)
{
    g_printerr("Error get vision processor proxy %s result: %s\n",
                callMethodName.c_str(), error->message);
    g_error_free(error);
}

void VisionProcessorProxy::onInitEngineCallback(
    AisdkVisionProcessor *delegate, GAsyncResult *result, gpointer userData) {
    std::cout << "----onInitEngineCallback------" << std::endl;
    GError *error = nullptr;
    aisdk_vision_processor_call_init_finish(delegate, result, &error);
}

void VisionProcessorProxy::onPromptImageSizeCallback(
    AisdkVisionProcessor *delegate, GAsyncResult *result, gpointer userData) {
    std::cout << "----onPromptImageSizeCallback------" << std::endl;
    GError *error = nullptr;
    aisdk_vision_processor_call_set_prompt_image_size_finish(
        delegate, result, &error);
}

void VisionProcessorProxy::onPromptImageNumberCallback(
        AisdkVisionProcessor *delegate, GAsyncResult *result, gpointer userData) {
    std::cout << "----onPromptImageNumberCallback------" << std::endl;
    GError *error = nullptr;
    aisdk_vision_processor_call_set_prompt_image_number_finish(
        delegate, result, &error);
}

void VisionProcessorProxy::onPromptImageStyleCallback(AisdkVisionProcessor *delegate,
                                                      GAsyncResult *result, gpointer userData) {
    std::cout << "----onPromptImageStyleCallback------" << std::endl;
    GError *error = nullptr;
    aisdk_vision_processor_call_set_prompt_image_style_finish(
        delegate, result, &error);
}

void VisionProcessorProxy::onPromptImageCallback(
    AisdkVisionProcessor *delegate, GAsyncResult *result, gpointer userData) {
    std::cout << "----onPromptImageCallback------" << std::endl;
    auto *proxy = static_cast<VisionProcessorProxy *>(userData);
    if (proxy->promptImageCallback_ == nullptr) {
        return;
    }

    gint outFormat;
    gint outHeight;
    gint outWidth;
    gint outTotal;
    gint outIndex;
    gchar *outData = nullptr;
    gint outDataSize;
    gint outErrorCode;
    GError *error = nullptr;
    bool success = aisdk_vision_processor_call_prompt_image_finish(
        delegate, &outFormat, &outHeight, &outWidth, &outTotal,
        &outIndex, &outData, &outDataSize, &outErrorCode, result, &error);
    if (!success) {
        printErrorMessage(error, "prompt_image");
        return;
    }
}
