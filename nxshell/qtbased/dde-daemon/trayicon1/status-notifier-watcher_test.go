// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package trayicon

import (
	"fmt"
	"testing"

	dbus "github.com/godbus/dbus/v5"
	ofdbus "github.com/linuxdeepin/go-dbus-factory/session/org.freedesktop.dbus"
	"github.com/stretchr/testify/assert"
)

func TestStatusNotifierWatcher_GetInterfaceName(t *testing.T) {
	s := StatusNotifierWatcher{}
	assert.Equal(t, snwDBusServiceName, s.GetInterfaceName())
}

func TestStatusNotifierWatcher_isDBusServiceRegistered(t *testing.T) {
	type args struct {
		serviceName string
	}
	type test struct {
		name string
		snw  *StatusNotifierWatcher
		args args
		want bool
	}
	tests := []test{
		func() test {
			serviceName := "xxx"

			m := new(ofdbus.MockDBus)
			m.MockInterfaceDbusIfc.On("GetNameOwner", dbus.Flags(0), serviceName).Return("aaa", nil)

			return test{
				name: "isDBusServiceRegistered succ",
				snw: &StatusNotifierWatcher{
					dbusDaemon: m,
				},
				args: args{
					serviceName: serviceName,
				},
				want: true,
			}
		}(),
		func() test {
			serviceName := "xxx"

			m := new(ofdbus.MockDBus)
			m.MockInterfaceDbusIfc.On("GetNameOwner", dbus.Flags(0), serviceName).Return("", nil)

			return test{
				name: "isDBusServiceRegistered empty",
				snw: &StatusNotifierWatcher{
					dbusDaemon: m,
				},
				args: args{
					serviceName: serviceName,
				},
				want: false,
			}
		}(),
		func() test {
			serviceName := "xxx"

			m := new(ofdbus.MockDBus)
			m.MockInterfaceDbusIfc.On("GetNameOwner", dbus.Flags(0), serviceName).Return("", fmt.Errorf("xxx"))

			return test{
				name: "isDBusServiceRegistered error",
				snw: &StatusNotifierWatcher{
					dbusDaemon: m,
				},
				args: args{
					serviceName: serviceName,
				},
				want: false,
			}
		}(),
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got := tt.snw.isDBusServiceRegistered(tt.args.serviceName)
			assert.Equal(t, tt.want, got)
		})
	}
}
