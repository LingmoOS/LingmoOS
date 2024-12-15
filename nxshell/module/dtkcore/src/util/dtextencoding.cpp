// SPDX-FileCopyrightText: 2022-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtextencoding.h"

#include <QtMath>
#include <QFile>
#include <QLibrary>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#else
#include <QTextCodec>
#endif

#include <climits>
#include <unicode/ucsdet.h>
#include <uchardet/uchardet.h>
#include <iconv.h>

DCORE_BEGIN_NAMESPACE

class LibICU
{
public:
    LibICU();
    ~LibICU();

    bool isValid();
    bool detectEncoding(const QByteArray &content, QByteArrayList &charset);

    UCharsetDetector *(*icu_ucsdet_open)(UErrorCode *status);
    void (*icu_ucsdet_close)(UCharsetDetector *ucsd);
    void (*icu_ucsdet_setText)(UCharsetDetector *ucsd, const char *textIn, int32_t len, UErrorCode *status);
    const UCharsetMatch **(*icu_ucsdet_detectAll)(UCharsetDetector *ucsd, int32_t *matchesFound, UErrorCode *status);
    const char *(*icu_ucsdet_getName)(const UCharsetMatch *ucsm, UErrorCode *status);
    int32_t (*icu_ucsdet_getConfidence)(const UCharsetMatch *ucsm, UErrorCode *status);

private:
    QLibrary *icuuc = nullptr;

    Q_DISABLE_COPY(LibICU)
};

class Libuchardet
{
public:
    Libuchardet();
    ~Libuchardet();

    bool isValid();
    QByteArray detectEncoding(const QByteArray &content);

    uchardet_t (*uchardet_new)(void);
    void (*uchardet_delete)(uchardet_t ud);
    int (*uchardet_handle_data)(uchardet_t ud, const char *data, size_t len);
    void (*uchardet_data_end)(uchardet_t ud);
    void (*uchardet_reset)(uchardet_t ud);
    const char *(*uchardet_get_charset)(uchardet_t ud);

private:
    QLibrary *uchardet = nullptr;

    Q_DISABLE_COPY(Libuchardet)
};

Q_GLOBAL_STATIC(LibICU, LibICUInstance);
Q_GLOBAL_STATIC(Libuchardet, LibuchardetInstance);

LibICU::LibICU()
{
    // Load libicuuc.so
    icuuc = new QLibrary("libicuuc");
    if (!icuuc->load()) {
        delete icuuc;
        icuuc = nullptr;
        return;
    }

    auto initFunctionError = [this]() {
        icuuc->unload();
        delete icuuc;
        icuuc = nullptr;
    };

#define INIT_ICUUC(Name)                                                                                                         \
    icu_##Name = reinterpret_cast<decltype(icu_##Name)>(icuuc->resolve(#Name));                                                  \
    if (!icu_##Name) {                                                                                                           \
        initFunctionError();                                                                                                     \
        return;                                                                                                                  \
    }

    // Note: use prefix 'icu' to avoid "disabled expansion of recursive macro" warning.
    INIT_ICUUC(ucsdet_open);
    INIT_ICUUC(ucsdet_close);
    INIT_ICUUC(ucsdet_setText);
    INIT_ICUUC(ucsdet_detectAll);
    INIT_ICUUC(ucsdet_getName);
    INIT_ICUUC(ucsdet_getConfidence);
}

LibICU::~LibICU()
{
    if (icuuc) {
        delete icuuc;
    }
}

bool LibICU::isValid()
{
    return (icuuc);
}

bool LibICU::detectEncoding(const QByteArray &content, QByteArrayList &charset)
{
    UErrorCode status = U_ZERO_ERROR;
    UCharsetDetector *detector = icu_ucsdet_open(&status);
    if (U_FAILURE(status)) {
        return false;
    }

    icu_ucsdet_setText(detector, content.data(), content.size(), &status);
    if (U_FAILURE(status)) {
        icu_ucsdet_close(detector);
        return false;
    }

    int32_t matchCount = 0;
    const UCharsetMatch **charsetMatch = icu_ucsdet_detectAll(detector, &matchCount, &status);
    if (U_FAILURE(status)) {
        icu_ucsdet_close(detector);
        return false;
    }

    int recordCount = qMin(3, matchCount);
    for (int i = 0; i < recordCount; i++) {
        const char *encoding = icu_ucsdet_getName(charsetMatch[i], &status);
        if (U_FAILURE(status)) {
            icu_ucsdet_close(detector);
            return false;
        }
        charset << QByteArray(encoding);
    }

    icu_ucsdet_close(detector);
    return true;
}

Libuchardet::Libuchardet()
{
    uchardet = new QLibrary("libuchardet", "0");
    if (!uchardet->load()) {
        delete uchardet;
        uchardet = nullptr;
        return;
    }

    auto initFunctionError = [this]() {
        uchardet->unload();
        delete uchardet;
        uchardet = nullptr;
    };

#define INIT_UCHARDET(Name)                                                                                                      \
    Name = reinterpret_cast<decltype(Name)>(uchardet->resolve(#Name));                                                           \
    if (!Name) {                                                                                                                 \
        initFunctionError();                                                                                                     \
        return;                                                                                                                  \
    }

    INIT_UCHARDET(uchardet_new);
    INIT_UCHARDET(uchardet_delete);
    INIT_UCHARDET(uchardet_handle_data);
    INIT_UCHARDET(uchardet_data_end);
    INIT_UCHARDET(uchardet_reset);
    INIT_UCHARDET(uchardet_get_charset);
}

Libuchardet::~Libuchardet()
{
    if (uchardet) {
        delete uchardet;
    }
}

bool Libuchardet::isValid()
{
    return uchardet;
}

QByteArray Libuchardet::detectEncoding(const QByteArray &content)
{
    QByteArray charset;

    uchardet_t handle = uchardet_new();
    if (0 == uchardet_handle_data(handle, content.data(), static_cast<size_t>(content.size()))) {
        uchardet_data_end(handle);
        charset = QByteArray(uchardet_get_charset(handle));
    }
    uchardet_delete(handle);

    return charset;
}

QByteArray selectCharset(const QByteArray &charset, const QByteArrayList &icuCharsetList)
{
    if (icuCharsetList.isEmpty()) {
        return charset;
    }

    static QByteArray encodingGB18030("GB18030");
    if (charset.isEmpty()) {
        return icuCharsetList.contains(encodingGB18030) ? encodingGB18030 : icuCharsetList[0];
    } else {
        if (charset.contains(icuCharsetList[0])) {
            return charset;
        } else {
            return icuCharsetList[0].contains(charset) ? icuCharsetList[0] : charset;
        }
    }
}

QByteArray DTextEncoding::detectTextEncoding(const QByteArray &content)
{
    if (content.isEmpty()) {
        return QByteArray("UTF-8");
    }

    QByteArray charset;
    if (LibuchardetInstance()->isValid()) {
        charset = LibuchardetInstance()->detectEncoding(content);
    }

    if (LibICUInstance()->isValid()) {
        QByteArrayList icuCharsetList;
        if (LibICUInstance()->detectEncoding(content, icuCharsetList)) {
            if (charset.isEmpty() && !icuCharsetList.isEmpty()) {
                charset = icuCharsetList.first();
            } else {
                // Improve GB18030 encoding recognition rate.
                charset = selectCharset(charset, icuCharsetList);
            }
        }
    }

    if (charset.isEmpty()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        std::optional<QStringConverter::Encoding> encoding = QStringConverter::encodingForData(content);
        if (encoding) {
            return QStringConverter::nameForEncoding(encoding.value());
        }
#else
        QTextCodec *codec = QTextCodec::codecForUtfText(content);
        if (codec) {
            return codec->name();
        }
#endif
    }

    // Use default encoding.
    if (charset.isEmpty() || charset.contains("ASCII")) {
        charset = QByteArray("UTF-8");
    }

    return charset;
}

QByteArray DTextEncoding::detectFileEncoding(const QString &fileName, bool *isOk)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        if (isOk) {
            *isOk = false;
        }
        return QByteArray();
    }

    // At most 64Kb data.
    QByteArray content = file.read(qMin<int>(static_cast<int>(file.size()), USHRT_MAX));
    file.close();

    if (isOk) {
        *isOk = true;
    }
    return detectTextEncoding(content);
}

bool DTextEncoding::convertTextEncoding(
    QByteArray &content, QByteArray &outContent, const QByteArray &toEncoding, const QByteArray &fromEncoding, QString *errString)
{
    return convertTextEncodingEx(content, outContent, toEncoding, fromEncoding, errString);
}

bool DTextEncoding::convertTextEncodingEx(QByteArray &content,
                                          QByteArray &outContent,
                                          const QByteArray &toEncoding,
                                          const QByteArray &fromEncoding,
                                          QString *errString,
                                          int *convertedBytes)
{
    if (content.isEmpty() || fromEncoding == toEncoding) {
        return true;
    }

    if (toEncoding.isEmpty()) {
        if (errString) {
            *errString = QStringLiteral("The encode that convert to is empty.");
        }
        return false;
    }

    QByteArray contentEncoding = fromEncoding;
    if (contentEncoding.isEmpty()) {
        contentEncoding = detectTextEncoding(content);
    }

    // iconv set errno when failed.
    iconv_t handle = iconv_open(toEncoding.data(), contentEncoding.data());
    if (reinterpret_cast<iconv_t>(-1) != handle) {
        size_t inBytesLeft = static_cast<size_t>(content.size());
        char *inbuf = content.data();
        size_t outBytesLeft = inBytesLeft * 4;
        char *outbuf = new char[outBytesLeft];

        char *bufferHeader = outbuf;
        size_t maxBufferSize = outBytesLeft;

        size_t ret = iconv(handle, &inbuf, &inBytesLeft, &outbuf, &outBytesLeft);
        int convertError = 0;
        if (static_cast<size_t>(-1) == ret) {
            convertError = errno;
            int converted = content.size() - static_cast<int>(inBytesLeft);
            if (convertedBytes) {
                *convertedBytes = converted;
            }

            if (errString) {
                switch (convertError) {
                    case EILSEQ:
                        *errString = QString("An invalid multibyte sequence has been encountered in the input."
                                             "Converted byte index: %1")
                                         .arg(converted);
                        break;
                    case EINVAL:
                        *errString = QString("An incomplete multibyte sequence has been encountered in the input. "
                                             "Converted byte index: %1")
                                         .arg(converted);
                        break;
                    case E2BIG:
                        *errString = QString("There is not sufficient room at *outbuf. Converted byte index: %1").arg(converted);
                        break;
                    default:
                        break;
                }
            }
        }
        iconv_close(handle);

        // Use iconv converted byte count.
        size_t realConvertSize = maxBufferSize - outBytesLeft;
        outContent = QByteArray(bufferHeader, static_cast<int>(realConvertSize));

        delete[] bufferHeader;
        // For errors, user decides to keep or remove converted text.
        return 0 == convertError;

    } else {
        if (EINVAL == errno && errString) {
            *errString = QStringLiteral("The conversion from fromcode to tocode is not supported by the implementation.");
        }

        return false;
    }
}

bool DTextEncoding::convertFileEncoding(const QString &fileName,
                                        const QByteArray &toEncoding,
                                        const QByteArray &fromEncoding,
                                        QString *errString)
{
    if (fromEncoding == toEncoding) {
        return true;
    }

    QFile file(fileName);
    if (!file.open(QFile::ReadWrite | QFile::Text)) {
        if (errString) {
            *errString = file.errorString();
            file.error();
        }
        return false;
    }

    QByteArray content = file.readAll();
    QByteArray outContent;
    if (!convertTextEncoding(content, outContent, toEncoding, fromEncoding, errString)) {
        file.close();
        return false;
    }

    file.seek(0);
    file.write(outContent);
    file.resize(outContent.size());
    file.close();

    if (QFile::NoError != file.error()) {
        if (errString) {
            *errString = file.errorString();
        }
        return false;
    }
    return true;
}

bool DTextEncoding::convertFileEncodingTo(const QString &fromFile,
                                          const QString &toFile,
                                          const QByteArray &toEncoding,
                                          const QByteArray &fromEncoding,
                                          QString *errString)
{
    if (fromEncoding == toEncoding) {
        return true;
    }

    if (fromFile == toFile) {
        return convertFileEncoding(fromFile, toEncoding, fromEncoding, errString);
    }

    // Check from file and to file before convert.
    QFile readFile(fromFile);
    if (!readFile.open(QFile::ReadOnly | QFile::Text)) {
        if (errString) {
            *errString = QString("Open convert from file failed, %1").arg(readFile.errorString());
        }
        return false;
    }

    QFile writeFile(toFile);
    if (!writeFile.open(QFile::WriteOnly | QFile::Text)) {
        readFile.close();
        if (errString) {
            *errString = QString("Open convert to file failed, %1").arg(writeFile.errorString());
        }
        return false;
    }

    QByteArray content = readFile.readAll();
    readFile.close();
    QByteArray outContent;

    if (!convertTextEncoding(content, outContent, toEncoding, fromEncoding, errString)) {
        writeFile.close();
        writeFile.remove();
        return false;
    }

    writeFile.write(outContent);
    writeFile.close();

    if (QFile::NoError != writeFile.error()) {
        if (errString) {
            *errString = writeFile.errorString();
        }
        return false;
    }
    return true;
}

DCORE_END_NAMESPACE
