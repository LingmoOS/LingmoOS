/*
    SPDX-License-Identifier: LGPL-2.0-or-later
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
*/

#ifndef KCRASH_METADATA_H
#define KCRASH_METADATA_H

#include <QtGlobal>

#include <array>

class QByteArray;

namespace KCrash
{
// A metadata writer interface.
class MetadataWriter
{
public:
    enum BoolValue { No = false, Yes = true };

    virtual void add(const char *key, const char *value, BoolValue boolValue) = 0;
    virtual void close() = 0;

protected:
    MetadataWriter() = default;
    virtual ~MetadataWriter() = default;

private:
    Q_DISABLE_COPY_MOVE(MetadataWriter)
};

#ifdef Q_OS_LINUX
// This writes the metadata file. Only really useful on Linux for now as this needs
// cleanup by a helper daemon later. Also, this is only ever useful when coredump is in use.
class MetadataINIWriter : public MetadataWriter
{
public:
    explicit MetadataINIWriter(const QByteArray &path);
    ~MetadataINIWriter() override = default;

    void add(const char *key, const char *value, BoolValue boolValue) override;
    void close() override;

    // open or not, all functions are generally save to call without checking this
    [[nodiscard]] bool isWritable() const;

private:
    bool writable = false;
    int fd = -1;

    Q_DISABLE_COPY_MOVE(MetadataINIWriter)
};
#endif

// Compile the crash metadata. These are the primary ARGV metadata, but additional
// metadata writers may be added behind it to (e.g.) write the data to a file as well.
//   man 7 signal-safety
class Metadata : public MetadataWriter
{
public:
    explicit Metadata(const char *cmd);
    ~Metadata() override = default;

    // Add an additional writer that should receive write calls as well. Do not call this after having called add.
    void setAdditionalWriter(MetadataWriter *writer);

    void add(const char *key, const char *value);
    void addBool(const char *key);

    // Also closes the backing writer.
    void close() override;

    // WARNING: DO NOT FORGET TO BUMP AND SHRINK THE CAPACITY
    // - boolean values increase the argv by 1 slot
    // - non-boolean values increase the argv by 2 slots
    // - this should always be the maximum of potentially used slots
    // - if you re-count slots, don't forget that the 'cmd' and terminal NULL take a slot each
    std::array<const char *, 34> argv{};
    std::size_t argc = 0;

private:
    void add(const char *key, const char *value, BoolValue boolValue) override;

    // Obviously if we should ever need more writers, refactor to std::initializer_list or something similar.
    MetadataWriter *m_writer = nullptr;

    Q_DISABLE_COPY_MOVE(Metadata)
};

} // namespace KCrash

#endif // KCRASH_METADATA_H
