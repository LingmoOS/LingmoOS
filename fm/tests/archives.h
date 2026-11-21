// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <archive.h>
#include <archive_entry.h>

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <unistd.h>

namespace fmtest {

// Writes the files into an archive of libarchive's: format "zip" or "7zip",
// options such as "zip:encryption=aes256" with a passphrase.
inline bool writeArchive(const std::string& path, const std::string& format,
                         const std::map<std::string, std::string>& files, const std::string& options = {},
                         const std::string& passphrase = {})
{
    struct archive* writer = archive_write_new();
    bool ok = archive_write_set_format_by_name(writer, format.c_str()) == ARCHIVE_OK
              && (options.empty() || archive_write_set_options(writer, options.c_str()) == ARCHIVE_OK)
              && (passphrase.empty() || archive_write_set_passphrase(writer, passphrase.c_str()) == ARCHIVE_OK)
              && archive_write_open_filename(writer, path.c_str()) == ARCHIVE_OK;
    for (auto it = files.begin(); ok && it != files.end(); ++it) {
        struct archive_entry* entry = archive_entry_new();
        archive_entry_set_pathname(entry, it->first.c_str());
        archive_entry_set_filetype(entry, AE_IFREG);
        archive_entry_set_perm(entry, 0644);
        archive_entry_set_size(entry, la_int64_t(it->second.size()));
        ok = archive_write_header(writer, entry) == ARCHIVE_OK
             && archive_write_data(writer, it->second.data(), it->second.size()) == la_ssize_t(it->second.size());
        archive_entry_free(entry);
    }
    ok = archive_write_close(writer) == ARCHIVE_OK && ok;
    archive_write_free(writer);
    return ok;
}

// Cuts the file into parts named after it with ".001", ".002", … and
// removes it; the number of parts, 0 on failure.
inline int splitFile(const std::string& path, size_t partSize)
{
    std::ifstream input(path, std::ios::binary);
    const std::string data((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    int parts = 0;
    for (size_t offset = 0; offset < data.size(); offset += partSize) {
        char suffix[8];
        std::snprintf(suffix, sizeof(suffix), ".%03d", ++parts);
        std::ofstream part(path + suffix, std::ios::binary | std::ios::trunc);
        part << data.substr(offset, partSize);
        if (!part)
            return 0;
    }
    ::unlink(path.c_str());
    return parts;
}

// Bytes that do not compress, so an archive of them spans several parts.
inline std::string noise(size_t size)
{
    std::string data(size, '\0');
    uint32_t state = 2463534242u;
    for (char& byte : data) {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        byte = char(state);
    }
    return data;
}

} // namespace fmtest
