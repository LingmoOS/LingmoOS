// Code generated by "dbusutil-gen em -type Manager"; DO NOT EDIT.

package power_manager

import (
	"github.com/lingmoos/go-lib/dbusutil"
)

func (v *Manager) GetExportedMethods() dbusutil.ExportedMethods {
	return dbusutil.ExportedMethods{
		{
			Name:    "CanHibernate",
			Fn:      v.CanHibernate,
			OutArgs: []string{"can"},
		},
		{
			Name:    "CanReboot",
			Fn:      v.CanReboot,
			OutArgs: []string{"can"},
		},
		{
			Name:    "CanShutdown",
			Fn:      v.CanShutdown,
			OutArgs: []string{"can"},
		},
		{
			Name:    "CanSuspend",
			Fn:      v.CanSuspend,
			OutArgs: []string{"can"},
		},
	}
}
