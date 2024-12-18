// Code generated by "dbusutil-gen em -type Manager"; DO NOT EDIT.

package main

import (
	"github.com/lingmoos/go-lib/dbusutil"
)

func (v *Manager) GetExportedMethods() dbusutil.ExportedMethods {
	return dbusutil.ExportedMethods{
		{
			Name:   "AuthenticateUser",
			Fn:     v.AuthenticateUser,
			InArgs: []string{"username"},
		},
		{
			Name:    "CurrentUser",
			Fn:      v.CurrentUser,
			OutArgs: []string{"username"},
		},
		{
			Name:    "IsLiveCD",
			Fn:      v.IsLiveCD,
			InArgs:  []string{"username"},
			OutArgs: []string{"result"},
		},
		{
			Name:   "SwitchToUser",
			Fn:     v.SwitchToUser,
			InArgs: []string{"username"},
		},
		{
			Name:   "UnlockCheck",
			Fn:     v.UnlockCheck,
			InArgs: []string{"username", "password"},
		},
	}
}
