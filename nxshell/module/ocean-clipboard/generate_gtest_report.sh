#!/bin/bash
# ocean-clipboard
#./test-prj-running.sh  ../build-ocean-clipboard-unknown-Release/test/ocean-clipboard tst_ocean-clipboard "*/ocean-clipboard/" "*/tests/* */ocean-clipboard/dbus/*"
lcov -d ../build-ocean-clipboard-unknown-Release/test/ocean-clipboard -c -o ../build-ocean-clipboard-unknown-Release/test/ocean-clipboard/coverage.info
genhtml -o ../build-ocean-clipboard-unknown-Release/test/ocean-clipboard/coverage  ../build-ocean-clipboard-unknown-Release/test/ocean-clipboard/coverage.info

# ocean-clipboardloader
#./test-prj-running.sh  ../build-ocean-clipboard-unknown-Release/test/ocean-clipboardloader tst_ocean-clipboardloader "*/ocean-clipboardloader/*" "*/tests/*"
lcov -d ../build-ocean-clipboard-unknown-Release/test/ocean-clipboardloader -c -o ../build-ocean-clipboard-unknown-Release/test/ocean-clipboardloader/coverage.info
genhtml -o ../build-ocean-clipboard-unknown-Release/test/ocean-clipboardloader/coverage  ../build-ocean-clipboard-unknown-Release/test/ocean-clipboardloader/coverage.info
