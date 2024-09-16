#!/bin/bash

# SPDX-FileCopyrightText: 2024 Uniontech Software Technology Co.,Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

_dde_dconfig() {
	local cur prev words cword
	_init_completion || return

	local opts=("-a -r -k")

	case $prev in
	'dde-dconfig')
		COMPREPLY=($(compgen -W 'help list set get reset gui watch' -- $cur))
		return
		;;
	'list' | 'set' | 'reset' | 'watch')
		local result=($(dde-dconfig list))
		COMPREPLY=($(compgen -W "${result[*]}" -- $cur))
		return
		;;
	'get')
		if [[ $cur == [-]* ]]; then
			COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
			return
		fi
		local result=($(dde-dconfig list))
		COMPREPLY=($(compgen -W "${result[*]}" -- $cur))
		return
		;;
	'-a')
		local result=($(dde-dconfig list))
		COMPREPLY=($(compgen -W "${result[*]}" -- $cur))
		return
		;;
	'-r')
		local a=${COMP_WORDS[COMP_CWORD - 2]}
		local result=($(dde-dconfig list -a $a))
		COMPREPLY=($(compgen -W "${result[*]}" -- $cur))
		return
		;;
	'-k')
		local a=${COMP_WORDS[COMP_CWORD - 4]}
		local r=${COMP_WORDS[COMP_CWORD - 2]}
		local result=($(dde-dconfig --get -a $a -r $r))
		COMPREPLY=($(compgen -W "${result[*]}" -- $cur))
		return
		;;
	esac

	if [[ $cur == [-]* ]]; then
		COMPREPLY=($(compgen -W "${opts}" -- ${cur}))
	fi

	if ((${#COMPREPLY[@]} != 0)); then
		# this removes any options from the list of completions that have
		# already been specified somewhere on the command line, as long as
		# these options can only be used once.
		local -A onlyonce=([-a]=1 [-r]=1 [-k]=1)
		local j
		for i in "${words[@]}"; do
			[[ $i && -v onlyonce["$i"] ]] || continue
			for j in "${!COMPREPLY[@]}"; do
				[[ ${COMPREPLY[j]} == "$i" ]] && unset 'COMPREPLY[j]'
			done
		done
	fi

} &&
	complete -F _dde_dconfig dde-dconfig
