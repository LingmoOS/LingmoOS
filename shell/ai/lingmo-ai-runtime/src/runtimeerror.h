#ifndef RUNTIMEERROR_H
#define RUNTIMEERROR_H

#include <string>

// 运行时相关错误分类
enum class RuntimeErrorCategory {
    INVALID = -1,

    INITIALIZATION,
    CONNECTIONCAPABILITY,
    TASK
};

// 运行时相关错误码
enum class RuntimeErrorCode {
    INVALID = -1,

    // 初始化相关错误
    FAILEDTOSTARTSERVER,
    FAILEDTOSTARTSERVICE,
    NOMOREMEMORY,

    // 连接相关错误
    NOMORECONNECTION,

    // 任务相关错误
    TASKTIMEOUT
};


class RuntimeError {
public:
    RuntimeError()
        : RuntimeError(RuntimeErrorCategory::INVALID, RuntimeErrorCode::INVALID, "") {}
    RuntimeError(RuntimeErrorCategory category, RuntimeErrorCode code, const std::string& message)
        : category_(category), code_(code), message_(message) {}

    ~RuntimeError() = default;

    const std::string& getModule() const { return module_; }
    RuntimeErrorCategory getCategory() const { return category_; }
    RuntimeErrorCode getCode() const { return code_; }
    const std::string& getMessage() const { return message_; }

private:
    const std::string module_ = "Runtime";
    const RuntimeErrorCategory category_;
    const RuntimeErrorCode code_;
    const std::string message_;
};

#endif