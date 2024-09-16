#compdef dde-dconfig

# SPDX-FileCopyrightText: 2024 Uniontech Software Technology Co.,Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

_dde-dconfig() {
  local ret=1
  local -a commands=(
    'list:list模式，列出可配置的应用Id，配置Id，及子目录'
    'get:get模式，用于查询指定配置项的信息'
    'set:set模式，用于设置配置项的值'
    'reset:reset模式，用于重置配置项的值，此会清除对应的缓存值'
    'watch:watch模式，用于监听配置项的更改'
    'gui:gui模式，用于启动GUI工具，需要安装对应的GUI工具dde-dconfig-editor'
  )

  _arguments -C -s -S -n \
    '(* -)'{-v,--version}"[display version information]: :->full" \
    '(- 1 *)'{-h,--help}'[display usage information]: :->full' \
    '1:cmd:->cmds' \
    '*:: :->args' && ret=0

  local last_index=$((${#words[@]}-1))
  local prev=${words[$last_index]}
  case "$state" in
  cmds)
    _describe -t commands 'commands' commands
    ;;
  args)
    case $prev in
    'list' | 'set' | 'get' | 'reset' | 'watch' | '-a')
      local result=($(dde-dconfig list))
      compadd -a result
      ret=0
      ;;
    '-r')
      local a=$words[$(($last_index-1))]
		  local result=($(dde-dconfig list -a $a))
      compadd -a result
      ret=0
      ;;
    '-k')
      local a=$words[$(($last_index-3))]
      local r=$words[$(($last_index-1))]
		  local result=($(dde-dconfig --get -a $a -r $r))
      compadd -a result
      ret=0
      ;;
    *)
      local result=('-a' '-k' '-r')
      compadd -a result
      ret=0
      ;;
    esac
    ;;
  *) ;;
  esac

  return ret
}

_dde-dconfig
