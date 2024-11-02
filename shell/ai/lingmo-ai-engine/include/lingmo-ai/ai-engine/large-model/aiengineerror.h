#ifndef AIENGINEERROR_H
#define AIENGINEERROR_H

#include <memory>
#include <string>
#include "aienginedefs.h"

namespace ai_engine
{
namespace lm
{
// 引擎相关错误分类
enum class EngineErrorCategory {
    Invalid = -1,
    Initialization,
    InputError,
    OutputError
};

#define PLUGIN_COMMON_ERROR_CODE   \
    /*通用初始化相关错误*/      \
    NomoreMemory,               \
                                \
    /*本地模型初始化相关错误*/  \
    FileNotFound,               \
    VersionMismatched,          \
    FilePermissionDenied,       \
                                \
    /*云端模型初始化相关错误*/  \
    NetworkDisconnected,        \
    NetworkLatencyTooLong,      \
    FailedToConnectServer,      \
    ServerNoResponse,           \
    NoServerService,            \
    Unauthorized,               \
                                \
    /*输入相关错误*/            \
    TooLittleParams,            \
    TooManyParams,              \
    InvalidParamType,           \
    TooLittleData,              \
    TooManyData,                \
    TooFrequentRequest,         \
    TooManyRequest,

// NLP引擎相关错误码
enum class NlpEngineErrorCode {
    PLUGIN_COMMON_ERROR_CODE

    // 输出相关错误
    FailedToAnswer,
    InvalidWords
};

// 语音引擎相关错误码
enum class SpeechEngineErrorCode {
    PLUGIN_COMMON_ERROR_CODE

    // 输出相关错误
    UnsupportedLanguage,
    UnsupportedScript,
    InvalidWakeupWords
};

// 图像引擎相关错误码
enum class VisionEngineErrorCode {
    PLUGIN_COMMON_ERROR_CODE

    // 输出相关错误
    Invalidwords,
    ImageGenerationFailed,
    ImageCensorshipFailed
};

class EngineError {
public:
    EngineError() = default;
    EngineError(AiCapability capability, EngineErrorCategory category, int code, const std::string& message)
        : capability_(capability), category_(category), code_(code), message_(message) {}

    const std::string& getModule() const { return module_; }
    AiCapability getCapability() const { return capability_; }
    EngineErrorCategory getCategory() const { return category_; }
    int getCode() const { return code_; }
    const std::string& getMessage() const { return message_; }

private:
    std::string module_ { "AI Engine" };
    AiCapability capability_ { AiCapability::Invalid };
    EngineErrorCategory category_ { EngineErrorCategory::Invalid };
    int code_ { -1 };
    std::string message_;
};
}
}

#endif