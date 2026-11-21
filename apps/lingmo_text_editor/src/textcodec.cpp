// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textcodec.h"
#include "unicode.h"

#include "base/i18n.h"
#include "base/locale.h"
#include "base/strings.h"

#include <unicode/ucnv.h>
#include <unicode/ucnv_cb.h>
#include <unicode/ustring.h>

#include <algorithm>
#include <cstring>
#include <memory>

using Lingmo::i18n::trNoop;

// Enough bytes to judge a legacy encoding by; whole files can be large.
static constexpr size_t sampleSize = 256 * 1024;

namespace
{
struct ConversionState
{
    bool error = false;
};

struct ConverterDeleter
{
    void operator()(UConverter *converter) const { ucnv_close(converter); }
};

using Converter = std::unique_ptr<UConverter, ConverterDeleter>;
}

// Each byte sequence that does not decode becomes U+FFFD and is counted.
static void toUnicodeCallback(const void *context, UConverterToUnicodeArgs *args, const char *, int32_t,
                              UConverterCallbackReason reason, UErrorCode *error)
{
    if (reason > UCNV_IRREGULAR)
        return;
    static_cast<ConversionState *>(const_cast<void *>(context))->error = true;
    *error = U_ZERO_ERROR;
    const UChar replacement = 0xFFFD;
    ucnv_cbToUWriteUChars(args, &replacement, 1, 0, error);
}

static void fromUnicodeCallback(const void *context, UConverterFromUnicodeArgs *args, const UChar *, int32_t,
                                UChar32, UConverterCallbackReason reason, UErrorCode *error)
{
    if (reason > UCNV_IRREGULAR)
        return;
    static_cast<ConversionState *>(const_cast<void *>(context))->error = true;
    *error = U_ZERO_ERROR;
    ucnv_cbFromUWriteSub(args, 0, error);
}

static Converter openConverter(const std::string &encoding)
{
    UErrorCode status = U_ZERO_ERROR;
    Converter converter(ucnv_open(encoding.c_str(), &status));
    if (U_FAILURE(status))
        return nullptr;
    return converter;
}

static bool isUtf16(std::string_view encoding)
{
    return encoding == "UTF-16LE" || encoding == "UTF-16BE";
}

static bool isUtf32(std::string_view encoding)
{
    return encoding == "UTF-32LE" || encoding == "UTF-32BE";
}

static bool isUnicode(std::string_view encoding)
{
    return encoding.substr(0, 4) == "UTF-";
}

// UTF-16 or UTF-32 in either byte order to UTF-8; a leading BOM is dropped.
static std::string decodeWide(std::string_view data, std::string_view encoding, bool *malformed)
{
    const bool wide32 = isUtf32(encoding);
    const bool little = encoding.size() >= 2 && encoding.substr(encoding.size() - 2) == "LE";
    const size_t unit = wide32 ? 4 : 2;
    const auto *bytes = reinterpret_cast<const unsigned char *>(data.data());
    const size_t count = data.size() / unit;
    auto read = [&](size_t i) -> uint32_t {
        const unsigned char *p = bytes + i * unit;
        if (wide32)
            return little ? uint32_t(p[0]) | uint32_t(p[1]) << 8 | uint32_t(p[2]) << 16 | uint32_t(p[3]) << 24
                          : uint32_t(p[3]) | uint32_t(p[2]) << 8 | uint32_t(p[1]) << 16 | uint32_t(p[0]) << 24;
        return little ? uint32_t(p[0]) | uint32_t(p[1]) << 8 : uint32_t(p[1]) | uint32_t(p[0]) << 8;
    };

    std::string result;
    result.reserve(data.size());
    for (size_t i = 0; i < count; ++i) {
        uint32_t c = read(i);
        if (!wide32 && c >= 0xD800 && c <= 0xDBFF) {
            const uint32_t low = i + 1 < count ? read(i + 1) : 0;
            if (low >= 0xDC00 && low <= 0xDFFF) {
                c = 0x10000 + ((c - 0xD800) << 10) + (low - 0xDC00);
                ++i;
            } else {
                c = 0xFFFD;
                *malformed = true;
            }
        } else if ((c >= 0xD800 && c <= 0xDFFF) || c > 0x10FFFF) {
            c = 0xFFFD;
            *malformed = true;
        }
        if (i == 0 && c == 0xFEFF)
            continue;
        Unicode::append(result, char32_t(c));
    }
    if (data.size() % unit != 0) {
        Unicode::append(result, 0xFFFD);
        *malformed = true;
    }
    return result;
}

// Through an ICU converter to UTF-8.
static std::string decodeLegacy(std::string_view data, UConverter *converter, bool *malformed)
{
    ConversionState state;
    UErrorCode status = U_ZERO_ERROR;
    ucnv_setToUCallBack(converter, toUnicodeCallback, &state, nullptr, nullptr, &status);
    std::u16string utf16(data.size() + 1, u'\0');
    status = U_ZERO_ERROR;
    int32_t length = ucnv_toUChars(converter, reinterpret_cast<UChar *>(utf16.data()), int32_t(utf16.size()),
                                   data.data(), int32_t(data.size()), &status);
    if (status == U_BUFFER_OVERFLOW_ERROR) {
        utf16.assign(size_t(length) + 1, u'\0');
        status = U_ZERO_ERROR;
        ucnv_resetToUnicode(converter);
        length = ucnv_toUChars(converter, reinterpret_cast<UChar *>(utf16.data()), int32_t(utf16.size()), data.data(),
                               int32_t(data.size()), &status);
    }
    if (U_FAILURE(status)) {
        *malformed = true;
        return Unicode::sanitized(data);
    }
    utf16.resize(size_t(length));
    *malformed = state.error;
    return Lingmo::base::strings::utf16ToUtf8(utf16);
}

static bool decodesCleanly(const std::string &encoding, std::string_view sample)
{
    Converter converter = openConverter(encoding);
    if (!converter)
        return false;
    bool malformed = false;
    decodeLegacy(sample, converter.get(), &malformed);
    return !malformed;
}

// A sample cut at a byte boundary may end in half a character.
static std::string_view trimmedSample(std::string_view data)
{
    if (data.size() <= sampleSize)
        return data;
    size_t end = sampleSize;
    while (end > sampleSize - 8 && static_cast<unsigned char>(data[end - 1]) >= 0x80)
        --end;
    return data.substr(0, end);
}

// The sample up to its last whole UTF-8 character, so text in a script of
// multibyte characters only is not cut inside one.
static std::string_view utf8Sample(std::string_view data)
{
    if (data.size() <= sampleSize)
        return data;
    size_t end = sampleSize;
    while (end > sampleSize - 3 && (static_cast<unsigned char>(data[end]) & 0xC0) == 0x80)
        --end;
    return data.substr(0, end);
}

// UTF-16 without a BOM: one byte of nearly every ASCII character is zero.
static std::string detectUtf16(std::string_view sample)
{
    const size_t count = std::min<size_t>(sample.size(), 4096) & ~size_t(1);
    if (count < 4)
        return {};
    int evenZero = 0;
    int oddZero = 0;
    for (size_t i = 0; i < count; i += 2) {
        evenZero += sample[i] == 0;
        oddZero += sample[i + 1] == 0;
    }
    const int pairs = int(count / 2);
    if (oddZero > pairs * 0.4 && evenZero < pairs * 0.05)
        return "UTF-16LE";
    if (evenZero > pairs * 0.4 && oddZero < pairs * 0.05)
        return "UTF-16BE";
    return {};
}

// How much the high bytes look like text in a double-byte encoding: common
// characters weigh 1, rare regions less. Returns the average weight.
static double doubleByteScore(std::string_view sample, std::string_view encoding)
{
    double weight = 0;
    int characters = 0;
    const size_t size = sample.size();
    for (size_t i = 0; i < size; ++i) {
        const auto lead = static_cast<unsigned char>(sample[i]);
        if (lead < 0x80)
            continue;
        const unsigned char trail = i + 1 < size ? static_cast<unsigned char>(sample[i + 1]) : 0;
        ++characters;

        if (encoding == "GB18030") {
            if (trail >= 0x30 && trail <= 0x39) {
                weight += 0.3;
                i += 3;
            } else {
                weight += lead >= 0xA1 && lead <= 0xF7 && trail >= 0xA1 ? 1.0 : 0.2;
                ++i;
            }
        } else if (encoding == "Big5") {
            weight += lead >= 0xA4 && lead <= 0xC6 ? 1.0 : 0.3;
            ++i;
        } else if (encoding == "Shift_JIS") {
            if (lead >= 0xA1 && lead <= 0xDF) {
                weight += 0.2;
            } else {
                weight += lead == 0x81 || lead == 0x82 || lead == 0x83 || (lead >= 0x88 && lead <= 0x9F)
                        ? 1.0 : 0.3;
                ++i;
            }
        } else if (encoding == "EUC-JP") {
            weight += lead == 0xA1 || lead == 0xA4 || lead == 0xA5 || (lead >= 0xB0 && lead <= 0xCF)
                    ? 1.0 : 0.3;
            ++i;
        } else if (encoding == "EUC-KR") {
            weight += (lead >= 0xB0 && lead <= 0xC8) || lead == 0xA1 ? 1.0 : 0.3;
            ++i;
        }
    }
    return characters > 0 ? weight / characters : 0;
}

// Western text puts accented letters one at a time between ASCII ones; text
// in a double-byte encoding read as Latin comes in runs.
static double singleByteScore(std::string_view sample)
{
    int runs = 0;
    int single = 0;
    size_t run = 0;
    for (size_t i = 0; i <= sample.size(); ++i) {
        if (i < sample.size() && static_cast<unsigned char>(sample[i]) >= 0x80) {
            ++run;
            continue;
        }
        if (run > 0) {
            ++runs;
            single += run == 1;
        }
        run = 0;
    }
    return runs > 0 ? double(single) / runs : 0;
}

static bool isDoubleByte(std::string_view encoding)
{
    return encoding == "GB18030" || encoding == "Big5" || encoding == "Shift_JIS" || encoding == "EUC-JP"
            || encoding == "EUC-KR";
}

std::string TextCodec::localeEncoding()
{
    return localeEncoding(Lingmo::base::Locale::system().name());
}

std::string TextCodec::localeEncoding(const std::string &localeName)
{
    const std::string language = localeName.substr(0, localeName.find('_'));
    if (language == "zh") {
        const bool traditional = localeName.find("Hant") != std::string::npos
                || localeName.find("_TW") != std::string::npos || localeName.find("_HK") != std::string::npos
                || localeName.find("_MO") != std::string::npos;
        return traditional ? "Big5" : "GB18030";
    }
    if (language == "ja")
        return "Shift_JIS";
    if (language == "ko")
        return "EUC-KR";
    for (const char *cyrillic : {"ru", "uk", "be", "bg", "mk", "sr"}) {
        if (language == cyrillic)
            return "windows-1251";
    }
    for (const char *central : {"pl", "cs", "sk", "hu", "sl", "hr", "ro"}) {
        if (language == central)
            return "windows-1250";
    }
    if (language == "el")
        return "windows-1253";
    if (language == "tr")
        return "windows-1254";
    if (language == "he")
        return "windows-1255";
    if (language == "ar" || language == "fa")
        return "windows-1256";
    if (language == "lt" || language == "lv" || language == "et")
        return "windows-1257";
    if (language == "vi")
        return "windows-1258";
    if (language == "th")
        return "windows-874";
    return "windows-1252";
}

static bool startsWith(std::string_view data, std::string_view prefix)
{
    return data.size() >= prefix.size() && std::memcmp(data.data(), prefix.data(), prefix.size()) == 0;
}

std::string TextCodec::detect(std::string_view data, bool *bom)
{
    using namespace std::string_view_literals;
    if (bom)
        *bom = true;
    if (startsWith(data, "\xEF\xBB\xBF"sv))
        return "UTF-8";
    if (startsWith(data, "\xFF\xFE\x00\x00"sv))
        return "UTF-32LE";
    if (startsWith(data, "\x00\x00\xFE\xFF"sv))
        return "UTF-32BE";
    if (startsWith(data, "\xFF\xFE"sv))
        return "UTF-16LE";
    if (startsWith(data, "\xFE\xFF"sv))
        return "UTF-16BE";
    if (bom)
        *bom = false;

    const std::string_view sample = trimmedSample(data);
    const std::string utf16 = detectUtf16(sample);
    if (!utf16.empty())
        return utf16;

    // Checked in one piece, so a sequence cut at the end counts as an error too.
    if (Unicode::isValid(utf8Sample(data)))
        return "UTF-8";

    const std::string preferred = localeEncoding();
    std::vector<std::string> candidates{preferred};
    for (const char *name : {"GB18030", "Big5", "Shift_JIS", "EUC-JP", "EUC-KR", "windows-1252"}) {
        if (preferred != name)
            candidates.emplace_back(name);
    }

    std::string best = "ISO-8859-1";
    double bestScore = -1;
    for (const std::string &candidate : candidates) {
        if (!decodesCleanly(candidate, sample))
            continue;
        double score;
        if (isDoubleByte(candidate))
            score = doubleByteScore(sample, candidate);
        else if (candidate == "windows-1252")
            score = singleByteScore(sample);
        else
            score = 0.85;
        if (candidate == preferred)
            score += 0.15;
        if (score > bestScore) {
            best = candidate;
            bestScore = score;
        }
    }
    return best;
}

bool TextCodec::isBinary(std::string_view data)
{
    const size_t count = std::min<size_t>(data.size(), 64 * 1024);
    size_t control = 0;
    for (size_t i = 0; i < count; ++i) {
        const auto byte = static_cast<unsigned char>(data[i]);
        if (byte == 0)
            return true;
        if (byte < 0x20 && byte != '\t' && byte != '\n' && byte != '\r' && byte != '\f' && byte != '\v'
                && byte != 0x1B && byte != 0x08)
            ++control;
    }
    return count > 0 && control > count / 10;
}

bool TextCodec::isSupported(const std::string &encoding)
{
    if (encoding.empty())
        return false;
    return isUnicode(encoding) ? encoding == "UTF-8" || isUtf16(encoding) || isUtf32(encoding)
                               : openConverter(encoding) != nullptr;
}

std::vector<std::string> TextCodec::splitLines(std::string_view text, LineEnding *dominant, bool *mixed)
{
    std::vector<std::string> lines;
    lines.reserve(text.size() / 40 + 1);
    size_t lf = 0;
    size_t crlf = 0;
    size_t cr = 0;

    const char *data = text.data();
    const size_t size = text.size();
    size_t start = 0;
    for (size_t i = 0; i < size; ++i) {
        const char c = data[i];
        if (c != '\n' && c != '\r')
            continue;
        lines.emplace_back(data + start, i - start);
        if (c == '\r' && i + 1 < size && data[i + 1] == '\n') {
            ++crlf;
            ++i;
        } else if (c == '\r') {
            ++cr;
        } else {
            ++lf;
        }
        start = i + 1;
    }
    lines.emplace_back(data + start, size - start);

    if (dominant) {
        *dominant = LineEnding::Unix;
        if (crlf > lf && crlf >= cr)
            *dominant = LineEnding::Windows;
        else if (cr > lf && cr > crlf)
            *dominant = LineEnding::ClassicMac;
    }
    if (mixed)
        *mixed = (lf > 0) + (crlf > 0) + (cr > 0) > 1;
    return lines;
}

DecodedText TextCodec::decode(std::string_view data, const std::string &encoding)
{
    DecodedText result;
    bool bom = false;
    result.encoding = encoding.empty() ? detect(data, &bom) : encoding;
    if (!encoding.empty()) {
        bool detectedBom = false;
        const std::string detected = detect(data, &detectedBom);
        bom = detectedBom && detected == encoding;
    }
    result.bom = bom;
    result.binary = !isUtf16(result.encoding) && !isUtf32(result.encoding) && isBinary(data);

    Converter converter;
    if (!isUnicode(result.encoding)) {
        converter = openConverter(result.encoding);
        if (!converter)
            result.encoding = "UTF-8";
    } else if (result.encoding != "UTF-8" && !isUtf16(result.encoding) && !isUtf32(result.encoding)) {
        result.encoding = "UTF-8";
    }

    std::string decoded;
    std::string_view text;
    if (result.encoding == "UTF-8") {
        std::string_view bytes = data;
        if (startsWith(bytes, "\xEF\xBB\xBF"))
            bytes.remove_prefix(3);
        if (Unicode::isValid(bytes)) {
            text = bytes;
        } else {
            decoded = Unicode::sanitized(bytes);
            result.malformed = true;
            text = decoded;
        }
    } else if (isUtf16(result.encoding) || isUtf32(result.encoding)) {
        decoded = decodeWide(data, result.encoding, &result.malformed);
        text = decoded;
    } else {
        decoded = decodeLegacy(data, converter.get(), &result.malformed);
        text = decoded;
    }
    result.lines = splitLines(text, &result.lineEnding, &result.mixedLineEndings);
    return result;
}

std::string_view TextCodec::lineEndingText(LineEnding lineEnding)
{
    switch (lineEnding) {
    case LineEnding::Windows:
        return "\r\n";
    case LineEnding::ClassicMac:
        return "\r";
    case LineEnding::Unix:
        break;
    }
    return "\n";
}

static void appendWide(std::string &out, uint32_t value, size_t unit, bool little)
{
    for (size_t i = 0; i < unit; ++i) {
        const size_t shift = little ? i * 8 : (unit - 1 - i) * 8;
        out += char((value >> shift) & 0xFF);
    }
}

static std::string encodeWide(std::string_view text, std::string_view encoding, bool bom)
{
    const bool wide32 = isUtf32(encoding);
    const bool little = encoding.substr(encoding.size() - 2) == "LE";
    const size_t unit = wide32 ? 4 : 2;
    std::string out;
    out.reserve(text.size() * unit + unit);
    if (bom)
        appendWide(out, 0xFEFF, unit, little);
    for (size_t i = 0; i < text.size(); i = Unicode::next(text, i)) {
        const char32_t c = Unicode::at(text, i);
        if (!wide32 && c >= 0x10000) {
            appendWide(out, 0xD800 + ((c - 0x10000) >> 10), unit, little);
            appendWide(out, 0xDC00 + ((c - 0x10000) & 0x3FF), unit, little);
        } else {
            appendWide(out, c, unit, little);
        }
    }
    return out;
}

std::string TextCodec::encode(const std::vector<std::string> &lines, const std::string &encoding, bool bom,
                              LineEnding lineEnding, bool *lossy)
{
    const std::string_view separator = lineEndingText(lineEnding);
    size_t size = 0;
    for (const std::string &line : lines)
        size += line.size() + separator.size();
    std::string text;
    text.reserve(size + 3);
    for (size_t i = 0; i < lines.size(); ++i) {
        if (i > 0)
            text += separator;
        text += lines[i];
    }

    if (lossy)
        *lossy = false;
    if (isUtf16(encoding) || isUtf32(encoding))
        return encodeWide(text, encoding, bom);
    Converter converter = isUnicode(encoding) ? nullptr : openConverter(encoding);
    if (!converter) {
        if (bom)
            text.insert(0, "\xEF\xBB\xBF");
        return text;
    }

    const std::u16string utf16 = Lingmo::base::strings::utf8ToUtf16(text);
    ConversionState state;
    UErrorCode status = U_ZERO_ERROR;
    ucnv_setFromUCallBack(converter.get(), fromUnicodeCallback, &state, nullptr, nullptr, &status);
    std::string bytes(size_t(UCNV_GET_MAX_BYTES_FOR_STRING(int32_t(utf16.size()), ucnv_getMaxCharSize(converter.get()))) + 1,
                      '\0');
    status = U_ZERO_ERROR;
    const int32_t length = ucnv_fromUChars(converter.get(), bytes.data(), int32_t(bytes.size()),
                                           reinterpret_cast<const UChar *>(utf16.data()), int32_t(utf16.size()), &status);
    bytes.resize(U_SUCCESS(status) ? size_t(length) : 0);
    if (lossy) {
        *lossy = state.error || U_FAILURE(status);
        // Not every converter reports unmappable characters; a round trip does.
        if (!*lossy) {
            bool malformed = false;
            Converter back = openConverter(encoding);
            *lossy = !back || decodeLegacy(bytes, back.get(), &malformed) != text;
        }
    }
    return bytes;
}

const std::vector<EncodingInfo> &TextCodec::encodings()
{
    static const std::vector<EncodingInfo> list{
        // TRANSLATORS: an encoding menu entry; the codec name in parentheses stays as it is.
        {"UTF-8", trNoop("Unicode (UTF-8)")},
        {"UTF-16LE", trNoop("Unicode (UTF-16 LE)")},
        {"UTF-16BE", trNoop("Unicode (UTF-16 BE)")},
        {"GB18030", trNoop("Chinese Simplified (GB18030)")},
        {"Big5", trNoop("Chinese Traditional (Big5)")},
        {"Big5-HKSCS", trNoop("Chinese Traditional (Big5-HKSCS)")},
        {"Shift_JIS", trNoop("Japanese (Shift_JIS)")},
        {"EUC-JP", trNoop("Japanese (EUC-JP)")},
        {"EUC-KR", trNoop("Korean (EUC-KR)")},
        {"windows-1252", trNoop("Western (Windows-1252)")},
        {"ISO-8859-1", trNoop("Western (ISO-8859-1)")},
        {"ISO-8859-15", trNoop("Western (ISO-8859-15)")},
        {"windows-1250", trNoop("Central European (Windows-1250)")},
        {"ISO-8859-2", trNoop("Central European (ISO-8859-2)")},
        {"windows-1251", trNoop("Cyrillic (Windows-1251)")},
        {"KOI8-R", trNoop("Cyrillic (KOI8-R)")},
        {"KOI8-U", trNoop("Cyrillic (KOI8-U)")},
        {"windows-1253", trNoop("Greek (Windows-1253)")},
        {"windows-1254", trNoop("Turkish (Windows-1254)")},
        {"windows-1255", trNoop("Hebrew (Windows-1255)")},
        {"windows-1256", trNoop("Arabic (Windows-1256)")},
        {"windows-1257", trNoop("Baltic (Windows-1257)")},
        {"windows-1258", trNoop("Vietnamese (Windows-1258)")},
        {"windows-874", trNoop("Thai (Windows-874)")},
    };
    return list;
}
