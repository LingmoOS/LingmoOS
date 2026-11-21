#include "wire.h"

#include "base/strings.h"

#include <cstring>

namespace dm {

std::string encodeFrame(const Json &message)
{
    std::string payload = message.serialize();
    const uint32_t length = uint32_t(payload.size());

    std::string frame(reinterpret_cast<const char *>(&length), sizeof(length));
    frame.append(payload);
    wipe(payload);
    return frame;
}

std::optional<Json> FrameReader::next()
{
    if (m_failed || m_buffer.size() < sizeof(uint32_t))
        return std::nullopt;

    uint32_t length = 0;
    std::memcpy(&length, m_buffer.data(), sizeof(length));
    if (length > kMaxFrameSize) {
        m_failed = true;
        return std::nullopt;
    }

    if (m_buffer.size() < sizeof(uint32_t) + length)
        return std::nullopt;

    std::string payload = m_buffer.substr(sizeof(uint32_t), length);
    // Only the frame taken out: the bytes after it are the next one.
    explicit_bzero(m_buffer.data(), sizeof(uint32_t) + length);
    m_buffer.erase(0, sizeof(uint32_t) + length);

    auto document = Json::parse(payload);
    wipe(payload);
    if (!document || !document->isObject()) {
        m_failed = true;
        return std::nullopt;
    }

    return document;
}

std::string encodeLine(const Json &message)
{
    return message.serialize() + '\n';
}

std::optional<Json> decodeLine(std::string_view line)
{
    auto document = Json::parse(Lingmo::base::strings::trimmed(line));
    if (!document || !document->isObject())
        return std::nullopt;

    return document;
}

void wipe(std::string &bytes)
{
    if (!bytes.empty())
        explicit_bzero(bytes.data(), bytes.size());
    bytes.clear();
}

} // namespace dm
