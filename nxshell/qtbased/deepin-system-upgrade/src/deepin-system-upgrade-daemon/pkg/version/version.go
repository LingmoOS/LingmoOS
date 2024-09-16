// SPDX-FileCopyrightText: 2022- 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package version

import "github.com/linuxdeepin/go-lib/dbusutil"

func ExportVersionManager(service *dbusutil.Service) error {
	m := newManager(service)
	return service.Export(dbusPath, m)
}
