// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package eventlog

import (
	"errors"
	"testing"

	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/stretchr/testify/assert"
)

func Test_Init(t *testing.T) {
	t.Run("Test Init", func(t *testing.T) {
		m := newModule(logger)
		assert.NotNil(t, m)
		session, err := dbusutil.NewSessionService()
		if err != nil {
			t.Skip("session service is not initialized")
		}
		assert.NotNil(t, session)
		assert.NotNil(t, _collectorMap)
		for _, c := range _collectorMap {
			err := c.Init(session, m.writeEventLog)
			if err != nil {
				assert.NoError(t, err)
			}
		}
	})

}

func Test_App_Collect(t *testing.T) {
	t.Run("Test App_Collect", func(t *testing.T) {
		m := newModule(logger)
		assert.NotNil(t, m)
		session, err := dbusutil.NewSessionService()
		if err != nil {
			t.Skip("session service is not initialized")
		}
		appCollector := newAppEventCollector()
		assert.NotNil(t, appCollector)
		appCollector.Init(session, m.writeEventLog)
		err = appCollector.Collect()
		if appCollector.dockObj == nil {
			assert.Equal(t, errors.New("dock init failed"), err)
		}
	})

}
