// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package shortcuts

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_systemType(t *testing.T) {

	var versions = []string{
		"Community", "Professional", "Home",
		"Server", "Personal", "Desktop",
		"",
	}

	assert.Contains(t, versions, systemType())
}

func Test_arr2set(t *testing.T) {
	testData := []struct {
		input  []string
		output map[string]bool
	}{
		{
			input: []string{"key1", "key2", "key3"},
			output: map[string]bool{
				"key1": true,
				"key2": true,
				"key3": true,
			},
		},
	}

	for _, data := range testData {
		assert.Equal(t, data.output, arr2set(data.input))
	}

}

func Test_strvLower(t *testing.T) {
	testData := []struct {
		in  []string
		out []string
	}{
		{
			in:  []string{"What's", "Your", "Problem"},
			out: []string{"what's", "your", "problem"},
		},
	}

	for _, data := range testData {
		assert.Equal(t, data.out, strvLower(data.in))
	}
}
