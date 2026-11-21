#pragma once

#include "base/json.h"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace dm {

using Json = Lingmo::base::json::Value;

// greetd's framing (greetd_ipc::codec): native-endian u32 length, JSON object.
constexpr uint32_t kMaxFrameSize = 64 * 1024;

std::string encodeFrame(const Json &message);

// An oversized frame or one that is not a JSON object fails it for good.
class FrameReader
{
public:
    void append(std::string_view bytes) { m_buffer.append(bytes); }
    std::optional<Json> next();
    bool failed() const { return m_failed; }

private:
    std::string m_buffer;
    bool m_failed = false;
};

// The daemon and its helpers exchange one compact JSON object per line.
std::string encodeLine(const Json &message);
std::optional<Json> decodeLine(std::string_view line);

// Overwrites a buffer that held a password before it is released.
void wipe(std::string &bytes);

} // namespace dm
