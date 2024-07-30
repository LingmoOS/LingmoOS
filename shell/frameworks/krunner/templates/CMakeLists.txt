# SPDX-FileCopyrightText: KDE Contributors
# SPDX-License-Identifier: BSD-2-Clause

set(apptemplate_DIRS
    runner6
    runner6python
)

kde_package_app_templates(TEMPLATES ${apptemplate_DIRS} INSTALL_DIR ${KDE_INSTALL_KAPPTEMPLATESDIR})
