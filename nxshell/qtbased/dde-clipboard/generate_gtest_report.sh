#!/bin/bash
# dde-clipboard
#./test-prj-running.sh  ../build-dde-clipboard-unknown-Release/test/dde-clipboard tst_dde-clipboard "*/dde-clipboard/" "*/tests/* */dde-clipboard/dbus/*"
lcov -d ../build-dde-clipboard-unknown-Release/test/dde-clipboard -c -o ../build-dde-clipboard-unknown-Release/test/dde-clipboard/coverage.info
genhtml -o ../build-dde-clipboard-unknown-Release/test/dde-clipboard/coverage  ../build-dde-clipboard-unknown-Release/test/dde-clipboard/coverage.info

# dde-clipboardloader
#./test-prj-running.sh  ../build-dde-clipboard-unknown-Release/test/dde-clipboardloader tst_dde-clipboardloader "*/dde-clipboardloader/*" "*/tests/*"
lcov -d ../build-dde-clipboard-unknown-Release/test/dde-clipboardloader -c -o ../build-dde-clipboard-unknown-Release/test/dde-clipboardloader/coverage.info
genhtml -o ../build-dde-clipboard-unknown-Release/test/dde-clipboardloader/coverage  ../build-dde-clipboard-unknown-Release/test/dde-clipboardloader/coverage.info
