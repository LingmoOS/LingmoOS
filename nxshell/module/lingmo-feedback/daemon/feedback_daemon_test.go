//+build dev

/**
 * Copyright (C) 2015 Lingmo Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

package main

import (
	C "launchpad.net/gocheck"
	"testing"
)

var daemon = NewFeedbackDaemon()

func Test(t *testing.T) { C.TestingT(t) }

type testWrapper struct{}

func init() {
	// override lingmo-feedback-cli
	lingmoFeedbackCliExe = "../cli/lingmo-feedback-cli.sh"
	C.Suite(&testWrapper{})
}

func (*testWrapper) TestGetDistroInfo(c *C.C) {
	distroName, distroRelease, _ := daemon.GetDistroInfo()
	c.Check(distroName, C.Equals, "lingmo")
	c.Check(distroRelease, C.Equals, "15.1.1")
}
