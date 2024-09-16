// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ddesktopentry.h"

#include <QDir>
#include <QFileInfo>
#include <QMutex>
#include <QTemporaryFile>
#include <QDebug>
#include <QSaveFile>

DCORE_BEGIN_NAMESPACE

enum { Space = 0x1, Special = 0x2 };

static const char charTraits[256] = {
    // Space: '\t', '\n', '\r', ' '
    // Special: '\n', '\r', ';', '=', '\\', '#'
    // Please note that '"' is NOT a special character

    0, 0, 0, 0, 0, 0, 0, 0, 0, Space, Space | Special, 0, 0, Space | Special, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    Space, 0, 0, Special, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, Special, 0, Special, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, Special, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

bool readLineFromData(const QByteArray &data, int &dataPos, int &lineStart, int &lineLen, int &equalsPos)
{
    int dataLen = data.length();

    equalsPos = -1;

    lineStart = dataPos;
    while (lineStart < dataLen && (charTraits[uint(uchar(data.at(lineStart)))] & Space))
        ++lineStart;

    int i = lineStart;
    while (i < dataLen) {
        while (!(charTraits[uint(uchar(data.at(i)))] & Special)) {
            if (++i == dataLen)
                goto break_out_of_outer_loop;
        }

        char ch = data.at(i++);
        if (ch == '=') {
            if (equalsPos == -1)
                equalsPos = i - 1;
        } else if (ch == '\n' || ch == '\r') {
            if (i == lineStart + 1) {
                ++lineStart;
            } else {
                --i;
                goto break_out_of_outer_loop;
            }
        } else if (ch == '\\') {
            if (i < dataLen) {
                char ch = data.at(i++);
                if (i < dataLen) {
                    char ch2 = data.at(i);
                    // \n, \r, \r\n, and \n\r are legitimate line terminators in INI files
                    if ((ch == '\n' && ch2 == '\r') || (ch == '\r' && ch2 == '\n'))
                        ++i;
                }
            }
        } else if (ch == ';') {
            // The multiple values should be separated by a semicolon and the value of the key
            // may be optionally terminated by a semicolon. Trailing empty strings must always
            // be terminated with a semicolon. Semicolons in these values need to be escaped
            // using \; .
            // Don't need to do anything here.
        } else {
            Q_ASSERT(ch == '#');

            if (i == lineStart + 1) {
                char ch;
                while (i < dataLen && (((ch = data.at(i)) != '\n') && ch != '\r'))
                    ++i;
                lineStart = i;
            }
        }
    }

break_out_of_outer_loop:
    dataPos = i;
    lineLen = i - lineStart;
    return lineLen > 0;
}

QString &doEscape(QString& str, const QHash<QChar,QChar> &repl)
{
    // First we replace slash.
    str.replace(QLatin1Char('\\'), QLatin1String("\\\\"));

    QHashIterator<QChar,QChar> i(repl);
    while (i.hasNext()) {
        i.next();
        if (i.key() != QLatin1Char('\\'))
            str.replace(i.key(), QString::fromLatin1("\\\\%1").arg(i.value()));
    }

    return str;
}

QString &doUnescape(QString& str, const QHash<QChar,QChar> &repl)
{
    int n = 0;
    while (1) {
        n=str.indexOf(QLatin1String("\\"), n);
        if (n < 0 || n > str.length() - 2)
            break;

        if (repl.contains(str.at(n+1))) {
            str.replace(n, 2, repl.value(str.at(n+1)));
        }

        n++;
    }

    return str;
}

/*! \internal */
class DDesktopEntrySection
{
public:
    DDesktopEntrySection() {}

    QString name;
    QMap<QString, QString> valuesMap;
    QByteArray unparsedDatas;
    int sectionPos = 99;

    inline operator QString() const {
        return QLatin1String("DDesktopEntrySection(") + name + QLatin1String(")");
    }

    QByteArray sectionData() const {
        if (unparsedDatas.isEmpty()) {
            // construct data and return
            QByteArray data;

            data.append(QString("[%1]\n").arg(name).toLocal8Bit());

            QMap<QString, QString>::const_iterator i;
            for (i = valuesMap.begin(); i != valuesMap.end(); i++) {
                data.append(QString("%1=%2\n").arg(i.key(), i.value()).toLocal8Bit());
            }

            return data;
        } else {
            return unparsedDatas;
        }
    }

    bool ensureSectionDataParsed() {
        if (unparsedDatas.isEmpty()) return true;

        valuesMap.clear();

        // for readLineFromFileData()
        int dataPos = 0;
        int lineStart;
        int lineLen;
        int equalsPos;

        while(readLineFromData(unparsedDatas, dataPos, lineStart, lineLen, equalsPos)) {
            if (unparsedDatas.at(lineStart) == '[') continue; // section name already parsed

            if (equalsPos != -1) {
                QString key = unparsedDatas.mid(lineStart, equalsPos - lineStart).trimmed();
                QString rawValue = unparsedDatas.mid(equalsPos + 1, lineStart + lineLen - equalsPos - 1).trimmed();

                valuesMap[key] = rawValue;
            }
        }

        unparsedDatas.clear();

        return true;
    }

    bool contains(const QString &key) const {
        const_cast<DDesktopEntrySection*>(this)->ensureSectionDataParsed();
        return valuesMap.contains(key);
    }

    QStringList allKeys() const {
        const_cast<DDesktopEntrySection*>(this)->ensureSectionDataParsed();
        return valuesMap.keys();
    }

    QString get(const QString &key, QString &defaultValue) {
        if (this->contains(key)) {
            return valuesMap[key];
        } else {
            return defaultValue;
        }
    }

    bool set(const QString &key, const QString &value) {
        if (this->contains(key)) {
            valuesMap.remove(key);
        }
        valuesMap[key] = value;
        return true;
    }

    bool remove(const QString &key) {
        if (this->contains(key)) {
            valuesMap.remove(key);
            return true;
        }
        return false;
    }
};

typedef QMap<QString, DDesktopEntrySection> SectionMap;

class DDesktopEntryPrivate
{
public:
    DDesktopEntryPrivate(const QString &filePath, DDesktopEntry *qq);
    ~DDesktopEntryPrivate();

    bool isWritable() const;
    bool fuzzyLoad();
    bool initSectionsFromData(const QByteArray &data);
    void setStatus(const DDesktopEntry::Status &newStatus) const;
    bool write(QIODevice &device) const;

    int sectionPos(const QString &sectionName) const;
    bool contains(const QString &sectionName, const QString &key) const;
    QStringList keys(const QString &sectionName) const;
    bool get(const QString &sectionName, const QString &key, QString *value);
    bool set(const QString &sectionName, const QString &key, const QString &value);
    bool remove(const QString &sectionName, const QString &key);

protected:
    QString filePath;
    QMutex fileMutex;
    SectionMap sectionsMap;
    mutable DDesktopEntry::Status status;

private:
    bool __padding[4];
    DDesktopEntry *q_ptr = nullptr;

    Q_DECLARE_PUBLIC(DDesktopEntry)
};

DDesktopEntryPrivate::DDesktopEntryPrivate(const QString &filePath, DDesktopEntry *qq)
    : filePath(filePath), q_ptr(qq)
{
    fuzzyLoad();
}

DDesktopEntryPrivate::~DDesktopEntryPrivate()
{

}

bool DDesktopEntryPrivate::isWritable() const
{
    QFileInfo fileInfo(filePath);

#ifndef QT_NO_TEMPORARYFILE
    if (fileInfo.exists()) {
#endif
        QFile file(filePath);
        return file.open(QFile::ReadWrite);
#ifndef QT_NO_TEMPORARYFILE
    } else {
        // Create the directories to the file.
        QDir dir(fileInfo.absolutePath());
        if (!dir.exists()) {
            if (!dir.mkpath(dir.absolutePath()))
                return false;
        }

        // we use a temporary file to avoid race conditions
        QTemporaryFile file(filePath);
        return file.open();
    }
#endif
}

bool DDesktopEntryPrivate::fuzzyLoad()
{
    QFile file(filePath);
    QFileInfo fileInfo(filePath);

    if (fileInfo.exists() && !file.open(QFile::ReadOnly)) {
        setStatus(DDesktopEntry::AccessError);
        return false;
    }

    if (file.isReadable() && file.size() != 0) {
        bool ok = false;
        QByteArray data = file.readAll();

        ok = initSectionsFromData(data);

        if (!ok) {
            setStatus(DDesktopEntry::FormatError);
            return false;
        }
    }

    setStatus(DDesktopEntry::NoError);
    return true;
}

bool DDesktopEntryPrivate::initSectionsFromData(const QByteArray &data)
{
    sectionsMap.clear();

    QString lastSectionName;
    int lastSectionStart = 0;
    bool formatOk = true;
    int sectionIdx = 0;
    // for readLineFromFileData()
    int dataPos = 0;
    int lineStart;
    int lineLen;
    int equalsPos;

    auto commitSection = [=](const QString &name, int sectionStartPos, int sectionLength, int sectionIndex) {
        DDesktopEntrySection lastSection;
        lastSection.name = name;
        lastSection.unparsedDatas = data.mid(sectionStartPos, sectionLength);
        lastSection.sectionPos = sectionIndex;
        sectionsMap[name] = lastSection;
    };

    // TODO: here we only need to find the section start, so things like equalsPos are useless here.
    //       maybe we can do some optimization here via adding extra argument to readLineFromData().
    while(readLineFromData(data, dataPos, lineStart, lineLen, equalsPos)) {
        // qDebug() << "CurrentLine:" << data.mid(lineStart, lineLen);
        if (data.at(lineStart) == '[') {
            // commit the last section we've ever read before we read the new one.
            if (!lastSectionName.isEmpty()) {
                commitSection(lastSectionName, lastSectionStart, lineStart - lastSectionStart, sectionIdx);
                sectionIdx++;
            }
            // process section name line
            QByteArray sectionName;
            int idx = data.indexOf(']', lineStart);
            if (idx == -1 || idx >= lineStart + lineLen) {
                qWarning() << "Bad desktop file format while reading line:" << data.mid(lineStart, lineLen);
                formatOk = false;
                sectionName = data.mid(lineStart + 1, lineLen - 1).trimmed();
            } else {
                sectionName = data.mid(lineStart + 1, idx - lineStart - 1).trimmed();
            }
            lastSectionName = sectionName;
            lastSectionStart = lineStart;
        }
    }

    Q_ASSERT(lineStart == data.length());
    if (!lastSectionName.isEmpty()) {
        commitSection(lastSectionName, lastSectionStart, lineStart - lastSectionStart, sectionIdx);
    }

    return formatOk;
}

// Always keep the first meet error status. and allowed clear the status.
void DDesktopEntryPrivate::setStatus(const DDesktopEntry::Status &newStatus) const
{
    if (newStatus == DDesktopEntry::NoError || this->status == DDesktopEntry::NoError) {
        this->status = newStatus;
    }
}

bool DDesktopEntryPrivate::write(QIODevice &device) const
{
    Q_Q(const DDesktopEntry);

    QStringList sortedKeys = q->allGroups(true);

    for (const QString &key : sortedKeys) {
        qint64 ret = device.write(sectionsMap[key].sectionData());
        if (ret == -1) return false;
    }

    return true;
}

int DDesktopEntryPrivate::sectionPos(const QString &sectionName) const
{
    if (sectionsMap.contains(sectionName)) {
        return sectionsMap[sectionName].sectionPos;
    }

    return -1;
}

bool DDesktopEntryPrivate::contains(const QString &sectionName, const QString &key) const
{
    if (sectionName.isNull() || key.isNull()) {
        return false;
    }

    if (sectionsMap.contains(sectionName)) {
        return sectionsMap[sectionName].contains(key);
    }

    return false;
}

QStringList DDesktopEntryPrivate::keys(const QString &sectionName) const
{
    if (sectionName.isNull()) {
        return {};
    }

    if (sectionsMap.contains(sectionName)) {
        return sectionsMap[sectionName].allKeys();
    }

    return {};
}

// return true if we found the value, and set the value to *value
bool DDesktopEntryPrivate::get(const QString &sectionName, const QString &key, QString *value)
{
    if (!this->contains(sectionName, key)) {
        return false;
    }

    if (sectionsMap.contains(sectionName)) {
        QString &&result = sectionsMap[sectionName].get(key, *value);
        *value = result;
        return true;
    }

    return false;
}

bool DDesktopEntryPrivate::set(const QString &sectionName, const QString &key, const QString &value)
{
    if (sectionsMap.contains(sectionName)) {
        bool result = sectionsMap[sectionName].set(key, value);
        return result;
    } else {
        // create new section.
        DDesktopEntrySection newSection;
        newSection.name = sectionName;
        newSection.set(key, value);
        sectionsMap[sectionName] = newSection;
        return true;
    }

    return false;
}

bool DDesktopEntryPrivate::remove(const QString &sectionName, const QString &key)
{
    if (this->contains(sectionName, key)) {
        return sectionsMap[sectionName].remove(key);
    }
    return false;
}

/*!
@~english
  @class Dtk::Core::DDesktopEntry
  \inmodule dtkcore
  @brief Handling desktop entry files.
  
  DDesktopEntry provide method for handling XDG desktop entry read and write. The interface
  of this class is similar to QSettings.
  
  For more details about the spec itself, please refer to:
  https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-latest.html
 */

DDesktopEntry::DDesktopEntry(const QString &filePath) noexcept
    : d_ptr(new DDesktopEntryPrivate(filePath, this))
{

}

DDesktopEntry::~DDesktopEntry()
{

}

/*!
@~english
  @brief Write back data to the desktop entry file.
  @return true if write success; otherwise returns false.
 */
bool DDesktopEntry::save() const
{
    Q_D(const DDesktopEntry);

    // write to file.
    if (d->isWritable()) {
        bool ok = false;
        bool createFile = false;
        QFileInfo fileInfo(d->filePath);

#if !defined(QT_BOOTSTRAPPED) && QT_CONFIG(temporaryfile)
        QSaveFile sf(d->filePath);
        sf.setDirectWriteFallback(true);
#else
        QFile sf(d->filePath);
#endif
        if (!sf.open(QIODevice::WriteOnly)) {
            d->setStatus(DDesktopEntry::AccessError);
            return false;
        }

        ok = d->write(sf);

#if !defined(QT_BOOTSTRAPPED) && QT_CONFIG(temporaryfile)
        if (ok) {
            ok = sf.commit();
        }
#endif

        if (ok) {
            // If we have created the file, apply the file perms
            if (createFile) {
                QFile::Permissions perms = fileInfo.permissions() | QFile::ReadOwner | QFile::WriteOwner
                                                                  | QFile::ReadGroup | QFile::ReadOther;
                QFile(d->filePath).setPermissions(perms);
            }
            return true;
        } else {
            d->setStatus(DDesktopEntry::AccessError);
            return false;
        }
    }

    return false;
}

/*!
@~english
  @brief Get data parse status
  
  @return Returns a status code indicating the first error that was met by DDesktopEntry, or QSettings::NoError if no error occurred.
  
  Be aware that DDesktopEntry delays performing some operations.
 */
DDesktopEntry::Status DDesktopEntry::status() const
{
    Q_D(const DDesktopEntry);
    return d->status;
}

/*!
@~english
  @brief Get a list of all section keys inside the given \a section.
  
  @return all available section keys.
 */
QStringList DDesktopEntry::keys(const QString &section) const
{
    Q_D(const DDesktopEntry);

    if (section.isEmpty()) {
        qWarning("DDesktopEntry::keys: Empty section name passed");
        return {};
    }

    return d->keys(section);
}

/*!
@~english
  @brief Get a list of all section groups inside the desktop entry.
  
  If \a sorted is set to true, the returned result will keep the order as-is when reading the entry file.
  
  @return all available section groups.
 */
QStringList DDesktopEntry::allGroups(bool sorted) const
{
    Q_D(const DDesktopEntry);

    if (!sorted) {
        return d->sectionsMap.keys();
    } else {
        using StrIntPair = QPair<QString, int>;

        QStringList keys = d->sectionsMap.keys();
        QList<StrIntPair> result;

        for (const QString & key : keys) {
            result << StrIntPair(key, d->sectionPos(key));
        }

        std::sort(result.begin(), result.end(), [](const StrIntPair& a, const StrIntPair& b) -> bool {
            return a.second < b.second;
        });

        keys.clear();

        for (const StrIntPair& pair : result) {
            keys << pair.first;
        }

        return keys;
    }
}

/*!
@~english
  @brief Check if the desktop entry file have the given \a section contains the given \a key
  
  @return true if the desktop entry contains the \a key in \a section; otherwise returns false.
 */
bool DDesktopEntry::contains(const QString &key, const QString &section) const
{
    Q_D(const DDesktopEntry);

    if (key.isEmpty() || section.isEmpty()) {
        qWarning("DDesktopEntry::contains: Empty key or section passed");
        return false;
    }

    return d->contains(section, key);
}

/*!
@~english
  @brief Returns the localized string value of the "Name" key under "Desktop Entry" section.
  
  It's equivalent to calling localizedValue("Name").

  @return Returns the localized string value of the "Name" key under "Desktop Entry" section.
  
  @sa localizedValue(), genericName(), ddeDisplayName()
 */
QString DDesktopEntry::name() const
{
    return localizedValue(QStringLiteral("Name"));
}

/*!
@~english
  @brief Returns the localized string value of the "GenericName" key under "Desktop Entry" section.
  
  It's equivalent to calling localizedValue("GenericName"). It will NOT fallback to "Name" if "GenericName"
  is not existed.
  
  @return Returns the localized string value of the "GenericName" key under "Desktop Entry" section.

  @sa localizedValue(), name(), ddeDisplayName()
 */
QString DDesktopEntry::genericName() const
{
    return localizedValue(QStringLiteral("GenericName"));
}

/*!
@~english
  @brief Display name specially for DDE applications.
  
  This will check "X-Deepin-Vendor" and will return the localized string value of "GenericName" if
  "X-Deepin-Vendor" is "deepin", or it will return the localized string value of "Name".

  @return Returns the display name specially for DDE applications.
  
  @sa localizedValue(), name(), genericName()
 */
QString DDesktopEntry::ddeDisplayName() const
{
    QString deepinVendor = stringValue("X-Deepin-Vendor");
    QString genericNameStr = genericName();
    if (deepinVendor == QStringLiteral("deepin") && !genericNameStr.isEmpty()) {
        return genericNameStr;
    }

    return name();
}

/*!
@~english
  @brief Returns the localized string value of the "Comment" key under "Desktop Entry" section.
  
  It's equivalent to calling localizedValue("Comment").

  @return Returns the localized string value of the "Comment" key under "Desktop Entry" section.
  
  @sa localizedValue()
 */
QString DDesktopEntry::comment() const
{
    return localizedValue(QStringLiteral("Comment"));
}

/*!
@~english
  @brief Returns the raw string value associated with the given \a key in \a section.
  
  If the entry contains no item with the key, the function returns a constructed \a defaultValue.

  @return Returns the raw string value associated with the given \a key in \a section.
  
  @sa stringValue() localizedValue() stringListValue()
 */
QString DDesktopEntry::rawValue(const QString &key, const QString &section, const QString &defaultValue) const
{
    Q_D(const DDesktopEntry);
    QString result = defaultValue;
    if (key.isEmpty() || section.isEmpty()) {
        qWarning("DDesktopEntry::value: Empty key or section passed");
        return result;
    }
    const_cast<DDesktopEntryPrivate *>(d)->get(section, key, &result); // FIXME: better way than const_cast?
    return result;
}

/*!
@~english
  @brief Returns the unescaped string value associated with the given \a key in \a section.
  
  If the entry contains no item with the key, the function returns a constructed \a defaultValue.
  
  @return Returns the unescaped string value associated with the given \a key in \a section.

  @sa rawValue() localizedValue() stringListValue()
 */
QString DDesktopEntry::stringValue(const QString &key, const QString &section, const QString &defaultValue) const
{
    QString rawResult = rawValue(key, section, defaultValue);
    rawResult = DDesktopEntry::unescape(rawResult);
    return rawResult;
}

/*!
@~english
  @brief Returns the localized string value associated with the given \a key and \a localeKey in \a section.
  
  If the given \a localeKey can't be found, it will fallback to "C", if still cannot found, will fallback to the
  key without localeKey.
  
  If the entry contains no item with the key, the function returns a constructed \a defaultValue.

  @return Returns the localized string value associated with the given \a key and \a localeKey in \a section.
  
  @sa rawValue() stringValue() stringListValue()
 */
QString DDesktopEntry::localizedValue(const QString &key, const QString &localeKey, const QString &section, const QString &defaultValue) const
{
    Q_D(const DDesktopEntry);
    QString result = defaultValue;
    QString actualLocaleKey = QLatin1String("C");
    if (key.isEmpty() || section.isEmpty()) {
        qWarning("DDesktopEntry::localizedValue: Empty key or section passed");
        return result;
    }

    QStringList possibleKeys;

    // 此处添加 bcp47Name() 是为了兼容 desktop 文件中的语言长短名解析。
    // 比如芬兰语，有 [fi] 和 [fi_FI] 两种情况，QLocale::name() 对应 fi_FI，QLocale::bcp47Name() 对应 fi。
    if (!localeKey.isEmpty()) {
        if (localeKey == "empty") {
            possibleKeys << key;
        } else if (localeKey == "default") {
            possibleKeys << QString("%1[%2]").arg(key, QLocale().name());
            possibleKeys << QString("%1[%2]").arg(key, QLocale().bcp47Name());
        } else if (localeKey == "system") {
            possibleKeys << QString("%1[%2]").arg(key, QLocale::system().name());
            possibleKeys << QString("%1[%2]").arg(key, QLocale::system().bcp47Name());
        } else {
            possibleKeys << QString("%1[%2]").arg(key, localeKey);
        }
    }

    if (!actualLocaleKey.isEmpty()) {
        possibleKeys << QString("%1[%2]").arg(key, actualLocaleKey);
    }
    possibleKeys << QString("%1[%2]").arg(key, "C");
    possibleKeys << key;

    for (const QString &oneKey : possibleKeys) {
        if (d->contains(section, oneKey)) {
            const_cast<DDesktopEntryPrivate *>(d)->get(section, oneKey, &result);
            break;
        }
    }

    return result;
}

/*!
@~english
  @brief Returns the localized string value associated with the given \a key and \a locale in \a section.
  
  If the given \a locale can't be found, it will fallback to "C", if still cannot found, will fallback to the
  key without a locale key.
  
  If the entry contains no item with the key, the function returns a default-constructed value.

  @return Returns the localized string value associated with the given \a key and \a locale in \a section.
  
  @sa rawValue() stringValue() stringListValue()
 */
QString DDesktopEntry::localizedValue(const QString &key, const QLocale &locale, const QString &section, const QString &defaultValue) const
{
    return localizedValue(key, locale.name(), section, defaultValue);
}

/*!
@~english
  @brief Returns a list of strings associated with the given \a key in the given \a section.
  
  If the entry contains no item with the key, the function returns a empty string list.
  
  @return Returns a list of strings associated with the given \a key in the given \a section.

  @sa rawValue() stringValue() localizedValue()
 */
QStringList DDesktopEntry::stringListValue(const QString &key, const QString &section) const
{
    Q_D(const DDesktopEntry);

    QString value;

    const_cast<DDesktopEntryPrivate *>(d)->get(section, key, &value);

    if (value.endsWith(';')) {
        value = value.left(value.length() - 1);
    }
    QStringList&& strings = value.split(';');

    QString combine;
    QStringList result;
    for (QString oneStr : strings) {
        if (oneStr.endsWith('\\')) {
            combine = combine + oneStr + ';';
            continue;
        }
        if (!combine.isEmpty()) {
            oneStr = combine + oneStr;
            combine.clear();
        }
        result << DDesktopEntry::unescape(oneStr, true);
    }

    return result;
}

bool DDesktopEntry::setRawValue(const QString &value, const QString &key, const QString &section)
{
    Q_D(DDesktopEntry);
    if (key.isEmpty() || section.isEmpty()) {
        qWarning("DDesktopEntry::setRawValue: Empty key or section passed");
        return false;
    }

    bool result = d->set(section, key, value);
    return result;
}

bool DDesktopEntry::setStringValue(const QString &value, const QString &key, const QString &section)
{
    QString escapedValue = value;
    DDesktopEntry::escape(escapedValue);
    bool result = setRawValue(escapedValue, key, section);
    return result;
}

bool DDesktopEntry::setLocalizedValue(const QString &value, const QString &localeKey, const QString &key, const QString &section)
{
    Q_D(DDesktopEntry);
    if (key.isEmpty() || section.isEmpty()) {
        qWarning("DDesktopEntry::setLocalizedValue: Empty key or section passed");
        return false;
    }

    QString actualKey = localeKey.isEmpty() ? key : QString("%1[%2]").arg(key, localeKey);

    bool result = d->set(section, actualKey, value);
    return result;
}

bool DDesktopEntry::removeEntry(const QString &key, const QString &section)
{
    Q_D(DDesktopEntry);
    if (key.isEmpty() || section.isEmpty()) {
        qWarning("DDesktopEntry::setLocalizedValue: Empty key or section passed");
        return false;
    }
    bool result = d->remove(section, key);
    return result;
}

/************************************************
 The escape sequences \s, \n, \t, \r, and \\ are supported for values
 of type string and localestring, meaning ASCII space, newline, tab,
 carriage return, and backslash, respectively.
 ************************************************/
QString &DDesktopEntry::escape(QString &str)
{
    QHash<QChar,QChar> repl;
    repl.insert(QLatin1Char('\n'),  QLatin1Char('n'));
    repl.insert(QLatin1Char('\t'),  QLatin1Char('t'));
    repl.insert(QLatin1Char('\r'),  QLatin1Char('r'));

    return doEscape(str, repl);
}

/************************************************
 Quoting must be done by enclosing the argument between double quotes and
 escaping the
    double quote character,
    backtick character ("`"),
    dollar sign ("$") and
    backslash character ("\")
by preceding it with an additional backslash character.
Implementations must undo quoting before expanding field codes and before
passing the argument to the executable program.

Note that the general escape rule for values of type string states that the
backslash character can be escaped as ("\\") as well and that this escape
rule is applied before the quoting rule. As such, to unambiguously represent a
literal backslash character in a quoted argument in a desktop entry file
requires the use of four successive backslash characters ("\\\\").
Likewise, a literal dollar sign in a quoted argument in a desktop entry file
is unambiguously represented with ("\\$").
 ************************************************/
QString &DDesktopEntry::escapeExec(QString &str)
{
    QHash<QChar,QChar> repl;
    // The parseCombinedArgString() splits the string by the space symbols,
    // we temporarily replace them on the special characters.
    // Replacement will reverse after the splitting.
    repl.insert(QLatin1Char('"'), QLatin1Char('"'));    // double quote,
    repl.insert(QLatin1Char('\''), QLatin1Char('\''));  // single quote ("'"),
    repl.insert(QLatin1Char('\\'), QLatin1Char('\\'));  // backslash character ("\"),
    repl.insert(QLatin1Char('$'), QLatin1Char('$'));    // dollar sign ("$"),

    return doEscape(str, repl);
}

/*
  The escape sequences \s, \n, \t, \r, and \\ are supported for values of type string and localestring,
  meaning ASCII space, newline, tab, carriage return, and backslash, respectively.
  
  Some keys can have multiple values. In such a case, the value of the key is specified as a plural: for
  example, string(s). The multiple values should be separated by a semicolon and the value of the key may
  be optionally terminated by a semicolon. Trailing empty strings must always be terminated with a semicolon.
  Semicolons in these values need to be escaped using \;.
  
  https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-latest.html#value-types
*/
QString &DDesktopEntry::unescape(QString &str, bool unescapeSemicolons)
{
    QHash<QChar,QChar> repl;
    repl.insert(QLatin1Char('\\'), QLatin1Char('\\'));
    repl.insert(QLatin1Char('s'),  QLatin1Char(' '));
    repl.insert(QLatin1Char('n'),  QLatin1Char('\n'));
    repl.insert(QLatin1Char('t'),  QLatin1Char('\t'));
    repl.insert(QLatin1Char('r'),  QLatin1Char('\r'));

    if (unescapeSemicolons) {
        repl.insert(QLatin1Char(';'),  QLatin1Char(';'));
    }

    return doUnescape(str, repl);
}

/************************************************
 Quoting must be done by enclosing the argument between double quotes and
 escaping the
    double quote character,
    backtick character ("`"),
    dollar sign ("$") and
    backslash character ("\")
by preceding it with an additional backslash character.
Implementations must undo quoting before expanding field codes and before
passing the argument to the executable program.

Reserved characters are
    space (" "),
    tab,
    newline,
    double quote,
    single quote ("'"),
    backslash character ("\"),
    greater-than sign (">"),
    less-than sign ("<"),
    tilde ("~"),
    vertical bar ("|"),
    ampersand ("&"),
    semicolon (";"),
    dollar sign ("$"),
    asterisk ("*"),
    question mark ("?"),
    hash mark ("#"),
    parenthesis ("(") and (")")
    backtick character ("`").

Note that the general escape rule for values of type string states that the
backslash character can be escaped as ("\\") as well and that this escape
rule is applied before the quoting rule. As such, to unambiguously represent a
literal backslash character in a quoted argument in a desktop entry file
requires the use of four successive backslash characters ("\\\\").
Likewise, a literal dollar sign in a quoted argument in a desktop entry file
is unambiguously represented with ("\\$").
 ************************************************/
QString &DDesktopEntry::unescapeExec(QString &str)
{
    unescape(str);
    QHash<QChar,QChar> repl;
    // The parseCombinedArgString() splits the string by the space symbols,
    // we temporarily replace them on the special characters.
    // Replacement will reverse after the splitting.
    repl.insert(QLatin1Char(' '), QChar::fromLatin1(01));   // space
    repl.insert(QLatin1Char('\t'), QChar::fromLatin1(02));  // tab
    repl.insert(QLatin1Char('\n'), QChar::fromLatin1(03));  // newline,

    repl.insert(QLatin1Char('"'), QLatin1Char('"'));    // double quote,
    repl.insert(QLatin1Char('\''), QLatin1Char('\''));  // single quote ("'"),
    repl.insert(QLatin1Char('\\'), QLatin1Char('\\'));  // backslash character ("\"),
    repl.insert(QLatin1Char('>'), QLatin1Char('>'));    // greater-than sign (">"),
    repl.insert(QLatin1Char('<'), QLatin1Char('<'));    // less-than sign ("<"),
    repl.insert(QLatin1Char('~'), QLatin1Char('~'));    // tilde ("~"),
    repl.insert(QLatin1Char('|'), QLatin1Char('|'));    // vertical bar ("|"),
    repl.insert(QLatin1Char('&'), QLatin1Char('&'));    // ampersand ("&"),
    repl.insert(QLatin1Char(';'), QLatin1Char(';'));    // semicolon (";"),
    repl.insert(QLatin1Char('$'), QLatin1Char('$'));    // dollar sign ("$"),
    repl.insert(QLatin1Char('*'), QLatin1Char('*'));    // asterisk ("*"),
    repl.insert(QLatin1Char('?'), QLatin1Char('?'));    // question mark ("?"),
    repl.insert(QLatin1Char('#'), QLatin1Char('#'));    // hash mark ("#"),
    repl.insert(QLatin1Char('('), QLatin1Char('('));    // parenthesis ("(")
    repl.insert(QLatin1Char(')'), QLatin1Char(')'));    // parenthesis (")")
    repl.insert(QLatin1Char('`'), QLatin1Char('`'));    // backtick character ("`").

    return doUnescape(str, repl);
}

bool DDesktopEntry::setStatus(const DDesktopEntry::Status &status)
{
    Q_D(DDesktopEntry);
    d->setStatus(status);

    return true;
}

DCORE_END_NAMESPACE
