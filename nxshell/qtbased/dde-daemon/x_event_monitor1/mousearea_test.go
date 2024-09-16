// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package x_event_monitor

import (
	"testing"

	C "gopkg.in/check.v1"
)

func Test(t *testing.T) {
	C.TestingT(t)
}

type mySuite struct{}

var _ = C.Suite(&mySuite{})

func (s *mySuite) TestUtils(c *C.C) {
	c.Check(hasButtonFlag(2), C.Equals, true)
	c.Check(hasButtonFlag(0), C.Equals, false)

	c.Check(hasKeyFlag(4), C.Equals, true)
	c.Check(hasKeyFlag(0), C.Equals, false)
}
