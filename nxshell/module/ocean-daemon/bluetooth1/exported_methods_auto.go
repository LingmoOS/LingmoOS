// Code generated by "dbusutil-gen em -type Bluetooth,agent,obexAgent"; DO NOT EDIT.

package bluetooth

import (
	"github.com/lingmoos/go-lib/dbusutil"
)

func (v *Bluetooth) GetExportedMethods() dbusutil.ExportedMethods {
	return dbusutil.ExportedMethods{
		{
			Name:   "CancelTransferSession",
			Fn:     v.CancelTransferSession,
			InArgs: []string{"sessionPath"},
		},
		{
			Name: "ClearUnpairedDevice",
			Fn:   v.ClearUnpairedDevice,
		},
		{
			Name:   "Confirm",
			Fn:     v.Confirm,
			InArgs: []string{"device", "accept"},
		},
		{
			Name:   "ConnectDevice",
			Fn:     v.ConnectDevice,
			InArgs: []string{"device", "apath"},
		},
		{
			Name:    "DebugInfo",
			Fn:      v.DebugInfo,
			OutArgs: []string{"info"},
		},
		{
			Name:   "DisconnectDevice",
			Fn:     v.DisconnectDevice,
			InArgs: []string{"device"},
		},
		{
			Name:   "FeedPasskey",
			Fn:     v.FeedPasskey,
			InArgs: []string{"device", "accept", "passkey"},
		},
		{
			Name:   "FeedPinCode",
			Fn:     v.FeedPinCode,
			InArgs: []string{"device", "accept", "pinCode"},
		},
		{
			Name:    "GetAdapters",
			Fn:      v.GetAdapters,
			OutArgs: []string{"adaptersJSON"},
		},
		{
			Name:    "GetDevices",
			Fn:      v.GetDevices,
			InArgs:  []string{"adapter"},
			OutArgs: []string{"devicesJSON"},
		},
		{
			Name:   "RemoveDevice",
			Fn:     v.RemoveDevice,
			InArgs: []string{"adapter", "device"},
		},
		{
			Name:   "RequestDiscovery",
			Fn:     v.RequestDiscovery,
			InArgs: []string{"adapter"},
		},
		{
			Name:    "SendFiles",
			Fn:      v.SendFiles,
			InArgs:  []string{"devAddress", "files"},
			OutArgs: []string{"sessionPath"},
		},
		{
			Name:   "SetAdapterAlias",
			Fn:     v.SetAdapterAlias,
			InArgs: []string{"adapter", "alias"},
		},
		{
			Name:   "SetAdapterDiscoverable",
			Fn:     v.SetAdapterDiscoverable,
			InArgs: []string{"adapter", "discoverable"},
		},
		{
			Name:   "SetAdapterDiscoverableTimeout",
			Fn:     v.SetAdapterDiscoverableTimeout,
			InArgs: []string{"adapter", "discoverableTimeout"},
		},
		{
			Name:   "SetAdapterDiscovering",
			Fn:     v.SetAdapterDiscovering,
			InArgs: []string{"adapter", "discovering"},
		},
		{
			Name:   "SetAdapterPowered",
			Fn:     v.SetAdapterPowered,
			InArgs: []string{"adapter", "powered"},
		},
		{
			Name:   "SetDeviceAlias",
			Fn:     v.SetDeviceAlias,
			InArgs: []string{"device", "alias"},
		},
		{
			Name:   "SetDeviceTrusted",
			Fn:     v.SetDeviceTrusted,
			InArgs: []string{"device", "trusted"},
		},
	}
}
func (v *agent) GetExportedMethods() dbusutil.ExportedMethods {
	return dbusutil.ExportedMethods{
		{
			Name:   "AuthorizeService",
			Fn:     v.AuthorizeService,
			InArgs: []string{"device", "uuid"},
		},
		{
			Name: "Cancel",
			Fn:   v.Cancel,
		},
		{
			Name:   "DisplayPasskey",
			Fn:     v.DisplayPasskey,
			InArgs: []string{"device", "passkey", "entered"},
		},
		{
			Name:   "DisplayPinCode",
			Fn:     v.DisplayPinCode,
			InArgs: []string{"device", "pinCode"},
		},
		{
			Name: "Release",
			Fn:   v.Release,
		},
		{
			Name:   "RequestAuthorization",
			Fn:     v.RequestAuthorization,
			InArgs: []string{"device"},
		},
		{
			Name:   "RequestConfirmation",
			Fn:     v.RequestConfirmation,
			InArgs: []string{"device", "passkey"},
		},
		{
			Name:    "RequestPasskey",
			Fn:      v.RequestPasskey,
			InArgs:  []string{"device"},
			OutArgs: []string{"passkey"},
		},
		{
			Name:    "RequestPinCode",
			Fn:      v.RequestPinCode,
			InArgs:  []string{"device"},
			OutArgs: []string{"pinCode"},
		},
		{
			Name:   "SendNotify",
			Fn:     v.SendNotify,
			InArgs: []string{"arg"},
		},
	}
}
func (v *obexAgent) GetExportedMethods() dbusutil.ExportedMethods {
	return dbusutil.ExportedMethods{
		{
			Name:    "AuthorizePush",
			Fn:      v.AuthorizePush,
			InArgs:  []string{"transferPath"},
			OutArgs: []string{"filename"},
		},
		{
			Name: "Cancel",
			Fn:   v.Cancel,
		},
	}
}
