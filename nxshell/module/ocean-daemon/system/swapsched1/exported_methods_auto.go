// Code generated by "dbusutil-gen em -type Helper"; DO NOT EDIT.

package swapsched

import (
	"github.com/lingmoos/go-lib/dbusutil"
)

func (v *Helper) GetExportedMethods() dbusutil.ExportedMethods {
	return dbusutil.ExportedMethods{
		{
			Name:   "Prepare",
			Fn:     v.Prepare,
			InArgs: []string{"sessionID"},
		},
	}
}
