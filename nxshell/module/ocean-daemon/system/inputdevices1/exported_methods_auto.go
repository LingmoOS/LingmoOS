// Code generated by "dbusutil-gen em -type InputDevices,Touchpad"; DO NOT EDIT.

package inputdevices1

import (
	"github.com/lingmoos/go-lib/dbusutil"
)

func (v *InputDevices) GetExportedMethods() dbusutil.ExportedMethods {
	return dbusutil.ExportedMethods{
		{
			Name:   "SetWakeupDevices",
			Fn:     v.SetWakeupDevices,
			InArgs: []string{"path", "value"},
		},
	}
}
func (v *Touchpad) GetExportedMethods() dbusutil.ExportedMethods {
	return dbusutil.ExportedMethods{
		{
			Name:   "SetTouchpadEnable",
			Fn:     v.SetTouchpadEnable,
			InArgs: []string{"enabled"},
		},
	}
}
