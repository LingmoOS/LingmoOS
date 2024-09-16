// SPDX-FileCopyrightText: 2022- 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package iso

import "github.com/linuxdeepin/go-lib/dbusutil"

var _manager *ISOManager

func ExportIsoManager(service *dbusutil.Service) error {
	manager := newManager(service)
	_manager = manager
	return service.Export(dbusPath, manager)
}
