#!/bin/bash
lupdate ./ -ts -no-obsolete -locations none translations/dde-session-shell_en.ts
#tx push -s -b m23 # Push the files that need to be translated
#tx pull -s -a -f -b m23 #Pull the translated files
# 注意：使用tx init的时候，传本地文件参数为translations/dde-session-shell_en.ts,如果使用dde-session-shell.ts，上传push到服务器一直报错。
# 执行了lupdate ...，将translations/dde-session-shell_en.ts手动改为translations/dde-session-shell.ts；再将.tx/config中修改为dde-session-shell.tx及dde-session-shell_<land>.ts，再push即可。
