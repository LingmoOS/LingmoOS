// Code generated by "dbusutil-gen em -type Manager"; DO NOT EDIT.

package main

import (
	"github.com/lingmoos/go-lib/dbusutil"
)

func (v *Manager) GetExportedMethods() dbusutil.ExportedMethods {
	return dbusutil.ExportedMethods{
		{
			Name:   "UpdateGreeterQtTheme",
			Fn:     v.UpdateGreeterQtTheme,
			InArgs: []string{"fd"},
		},
	}
}
