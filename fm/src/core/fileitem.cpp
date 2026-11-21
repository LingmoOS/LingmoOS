// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/fileitem.h"

#include "lingmo/files/file_item.h"
#include "lingmo/files/mime_types.h"

namespace fm {

bool FileItem::sameMetadata(const FileItem& other) const
{
    return url == other.url && text() == other.text() && sameStat(other) && isHidden == other.isHidden
            && isWritable == other.isWritable && mimeType == other.mimeType && iconName == other.iconName
            && targetUrl == other.targetUrl && typeDescription == other.typeDescription
            && originalPath == other.originalPath && deletionTime == other.deletionTime;
}

std::string formatSize(uint64_t bytes)
{
    return files::convertSize(bytes);
}

std::string mimeComment(const std::string& mimeType)
{
    return files::mime::description(mimeType);
}

} // namespace fm
