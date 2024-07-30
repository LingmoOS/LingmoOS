#compdef systemsettings kcmshell5 kcmshell6 kinfocenter

# SPDX-FileCopyrightText: 2022 ivan tkachenko <me@ratijas.tk>
#
# SPDX-License-Identifier: GPL-2.0-or-later

# standard stuff for `_arguments -C`
local context state state_descr line
typeset -A opt_args

local ret=1 kcm_line kcm_name kcm_desc quantity
local -a lines kcm_list kcmshell
local -A kcm_assoc

if [[ "$service" == "kcmshell5" || "$service" == "kcmshell6" ]]; then
  kcmshell=(
    '--caption=[Use a specific caption for the window]:caption:'
    '--icon=[Use a specific icon for the window]:icon:'
  )
  quantity='*'
  if [[ "$service" == "kcmshell6" ]]; then
    kcmshell+=(
      '--highlight[Show an indicator when settings have changed from their default value]'
    )
  fi
else
  quantity='1'
fi

_arguments -C \
  $kcmshell \
  '(- *)--help[Displays help on commandline options.]' \
  '(- *)--list[List all possible modules]' \
  '--args=[Arguments for the module]:arguments:' \
  "$quantity:module:->kcm" \
  && ret=0

case $state in
  kcm)
    lines=( ${(f)"$(_call_program kcm-list $service --list)"} )
    lines[1]=()  # first line is header, drop it

    kcm_assoc=()
    for kcm_line in $lines ; do
      # split at hypen into name and description, and strip whitespace
      kcm_name=${kcm_line%% *}
      kcm_desc=${kcm_line##*- }
      # skip already specified KCMs except for current word
      if (( $words[(Ie)$kcm_name] && $words[(Ie)$kcm_name] != $CURRENT )); then
        continue
      fi
      # some kcm names may get duplicates e.g. because of development setup,
      # store them in hash to automatically deduplicate
      kcm_assoc[$kcm_name]=$kcm_desc
    done

    kcm_list=()
    for kcm_name kcm_desc in ${(@kv)kcm_assoc} ; do  # iterate over array (@) of keys (k) and values (v)
      kcm_desc=${kcm_desc//:/\\:}  # espace double colons for _describe
      kcm_list+=( "${kcm_name}:${kcm_desc}" )
    done

    _describe -t kcm 'settings module' kcm_list && ret=0
    ;;
esac

return $ret
