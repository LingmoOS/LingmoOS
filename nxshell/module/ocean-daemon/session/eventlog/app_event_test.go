// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package eventlog

import (
	"os"
	"path/filepath"
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_getEntry(t *testing.T) {
	t.Run("Test getEntry", func(t *testing.T) {
		c := newAppEventCollector()
		c.items["app0"] = AppEntry{
			Id: "app0",
		}
		c.items["app1"] = AppEntry{
			Id: "app1",
		}
		c.items["app2"] = AppEntry{
			Id: "app2",
		}
		e := c.getEntry("app0")
		assert.Equal(t, e.Id, "app0")
		e = c.getEntry("app3")
		assert.NotNil(t, e)
	})
}

func Test_addEntry(t *testing.T) {
	t.Run("Test addEntry", func(t *testing.T) {
		c := newAppEventCollector()
		c.items["app0"] = AppEntry{
			Id: "app0",
		}
		c.items["app1"] = AppEntry{
			Id: "app1",
		}
		c.items["app2"] = AppEntry{
			Id: "app2",
		}
		assert.True(t, c.addEntry("app3", AppEntry{
			Id: "app3",
		}))
		assert.True(t, !c.addEntry("app2", AppEntry{
			Id: "app2",
		}))
	})
}
func Test_removeEntry(t *testing.T) {
	t.Run("Test removeEntry", func(t *testing.T) {
		c := newAppEventCollector()
		c.items["app0"] = AppEntry{
			Id: "app0",
		}
		c.items["app1"] = AppEntry{
			Id: "app1",
		}
		c.items["app2"] = AppEntry{
			Id: "app2",
		}
		assert.True(t, c.removeEntry("app2"))
		assert.True(t, !c.removeEntry("app3"))
	})

}

func Test_isSymlink(t *testing.T) {
	t.Run("Test isSymlink", func(t *testing.T) {
		dir, err := os.MkdirTemp("", "eventlog")
		assert.NoError(t, err)
		defer os.RemoveAll(dir)
		_, err = os.CreateTemp(dir, "filepath1")
		assert.NoError(t, err)
		assert.True(t, !isSymlink(filepath.Join(dir, "filepath1")))
	})

}

func Test_writeAppEventLog(t *testing.T) {
	t.Run("Test writeAppEventLog", func(t *testing.T) {
		c := newAppEventCollector()
		assert.NoError(t, c.writeAppEventLog(AppEntry{Id: "app0"}))
	})

}
