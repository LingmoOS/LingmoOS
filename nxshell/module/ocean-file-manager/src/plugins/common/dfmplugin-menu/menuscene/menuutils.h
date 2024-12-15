// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MENUUTILS_H
#define MENUUTILS_H

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/dfm_menu_defines.h>

namespace dfmplugin_menu {

class MenuUtils
{
public:
    static inline QVariantHash perfectMenuParams(const QVariantHash &params)
    {
        const auto &selectUrls = params.value(dfmbase::MenuParamKey::kSelectFiles).value<QList<QUrl>>();
        if (selectUrls.isEmpty())
            return params;

        QVariantHash tmpParams = params;
        if (!params.contains(dfmbase::MenuParamKey::kIsSystemPathIncluded)
            || !params.contains(dfmbase::MenuParamKey::kIsOCEANDesktopFileIncluded)
            || !params.contains(dfmbase::MenuParamKey::kIsFocusOnOCEANDesktopFile)) {

            bool isFocusOnOCEANDesktopFile = (DFMBASE_NAMESPACE::FileUtils::isComputerDesktopFile(selectUrls.first())
                                            || DFMBASE_NAMESPACE::FileUtils::isTrashDesktopFile(selectUrls.first())
                                            || DFMBASE_NAMESPACE::FileUtils::isHomeDesktopFile(selectUrls.first()));
            bool isOCEANDesktopFileIncluded = isFocusOnOCEANDesktopFile;
            bool isSystemPathIncluded = false;

            for (const auto &url : selectUrls) {
                if (!isSystemPathIncluded && DFMBASE_NAMESPACE::SystemPathUtil::instance()->isSystemPath(url.toLocalFile()))
                    isSystemPathIncluded = true;

                if (!isOCEANDesktopFileIncluded
                    && (DFMBASE_NAMESPACE::FileUtils::isComputerDesktopFile(url)
                        || DFMBASE_NAMESPACE::FileUtils::isTrashDesktopFile(url)
                        || DFMBASE_NAMESPACE::FileUtils::isHomeDesktopFile(url)))
                    isOCEANDesktopFileIncluded = true;

                if (isSystemPathIncluded && isOCEANDesktopFileIncluded)
                    break;
            }
            tmpParams[dfmbase::MenuParamKey::kIsSystemPathIncluded] = isSystemPathIncluded;
            tmpParams[dfmbase::MenuParamKey::kIsOCEANDesktopFileIncluded] = isOCEANDesktopFileIncluded;
            tmpParams[dfmbase::MenuParamKey::kIsFocusOnOCEANDesktopFile] = isFocusOnOCEANDesktopFile;
        }

        return tmpParams;
    }
};
}

#endif   // MENUUTILS_H
