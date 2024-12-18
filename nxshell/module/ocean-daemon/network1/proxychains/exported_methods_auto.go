// Code generated by "dbusutil-gen em -type Manager"; DO NOT EDIT.

package proxychains

import (
	"github.com/lingmoos/go-lib/dbusutil"
)

func (v *Manager) GetExportedMethods() dbusutil.ExportedMethods {
	return dbusutil.ExportedMethods{
		{
			Name:   "Set",
			Fn:     v.Set,
			InArgs: []string{"type0", "ip", "port", "user", "password"},
		},
		{
			Name:   "SetEnable",
			Fn:     v.SetEnable,
			InArgs: []string{"enable"},
		},
	}
}
