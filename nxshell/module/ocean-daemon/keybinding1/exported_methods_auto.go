// Code generated by "dbusutil-gen em -type Manager"; DO NOT EDIT.

package keybinding

import (
	"github.com/lingmoos/go-lib/dbusutil"
)

func (v *Manager) GetExportedMethods() dbusutil.ExportedMethods {
	return dbusutil.ExportedMethods{
		{
			Name:    "Add",
			Fn:      v.Add,
			InArgs:  []string{"name", "action", "keystroke"},
			OutArgs: []string{"ret0", "ret1"},
		},
		{
			Name:    "AddCustomShortcut",
			Fn:      v.AddCustomShortcut,
			InArgs:  []string{"name", "action", "keystroke"},
			OutArgs: []string{"id", "type0"},
		},
		{
			Name:   "AddShortcutKeystroke",
			Fn:     v.AddShortcutKeystroke,
			InArgs: []string{"id", "type0", "keystroke"},
		},
		{
			Name:    "CheckAvaliable",
			Fn:      v.CheckAvaliable,
			InArgs:  []string{"keystroke"},
			OutArgs: []string{"available", "shortcut"},
		},
		{
			Name:   "ClearShortcutKeystrokes",
			Fn:     v.ClearShortcutKeystrokes,
			InArgs: []string{"id", "type0"},
		},
		{
			Name:   "Delete",
			Fn:     v.Delete,
			InArgs: []string{"id", "type0"},
		},
		{
			Name:   "DeleteCustomShortcut",
			Fn:     v.DeleteCustomShortcut,
			InArgs: []string{"id"},
		},
		{
			Name:   "DeleteShortcutKeystroke",
			Fn:     v.DeleteShortcutKeystroke,
			InArgs: []string{"id", "type0", "keystroke"},
		},
		{
			Name:   "Disable",
			Fn:     v.Disable,
			InArgs: []string{"id", "type0"},
		},
		{
			Name:   "EnableSystemShortcut",
			Fn:     v.EnableSystemShortcut,
			InArgs: []string{"shortcuts", "enabled", "isPersistent"},
		},
		{
			Name:    "GetCapsLockState",
			Fn:      v.GetCapsLockState,
			OutArgs: []string{"state"},
		},
		{
			Name:    "GetShortcut",
			Fn:      v.GetShortcut,
			InArgs:  []string{"id", "type0"},
			OutArgs: []string{"shortcut"},
		},
		{
			Name: "GrabScreen",
			Fn:   v.GrabScreen,
		},
		{
			Name:    "List",
			Fn:      v.List,
			OutArgs: []string{"shortcuts"},
		},
		{
			Name:    "ListAllShortcuts",
			Fn:      v.ListAllShortcuts,
			OutArgs: []string{"shortcuts"},
		},
		{
			Name:    "ListShortcutsByType",
			Fn:      v.ListShortcutsByType,
			InArgs:  []string{"type0"},
			OutArgs: []string{"shortcuts"},
		},
		{
			Name:    "LookupConflictingShortcut",
			Fn:      v.LookupConflictingShortcut,
			InArgs:  []string{"keystroke"},
			OutArgs: []string{"shortcut"},
		},
		{
			Name:    "ModifiedAccel",
			Fn:      v.ModifiedAccel,
			InArgs:  []string{"id", "type0", "keystroke", "add"},
			OutArgs: []string{"ret0", "ret1"},
		},
		{
			Name:   "ModifyCustomShortcut",
			Fn:     v.ModifyCustomShortcut,
			InArgs: []string{"id", "name", "cmd", "keystroke"},
		},
		{
			Name:    "Query",
			Fn:      v.Query,
			InArgs:  []string{"id", "type0"},
			OutArgs: []string{"shortcut"},
		},
		{
			Name: "Reset",
			Fn:   v.Reset,
		},
		{
			Name:    "SearchShortcuts",
			Fn:      v.SearchShortcuts,
			InArgs:  []string{"query"},
			OutArgs: []string{"shortcuts"},
		},
		{
			Name: "SelectKeystroke",
			Fn:   v.SelectKeystroke,
		},
		{
			Name:   "SetCapsLockState",
			Fn:     v.SetCapsLockState,
			InArgs: []string{"state"},
		},
		{
			Name:   "SetNumLockState",
			Fn:     v.SetNumLockState,
			InArgs: []string{"state"},
		},
	}
}
