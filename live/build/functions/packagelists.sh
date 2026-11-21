#!/bin/sh

## live-build(7) - System Build Scripts
## Copyright (C) 2016-2020 The Debian Live team
## Copyright (C) 2006-2015 Daniel Baumann <mail@daniel-baumann.ch>
##
## This program comes with ABSOLUTELY NO WARRANTY; for details see COPYING.
## This is free software, and you are welcome to redistribute it
## under certain conditions; see COPYING for details.


Expand_packagelist ()
{
	local _LB_EXPAND_QUEUE
	_LB_EXPAND_QUEUE="$(basename "${1}")"

	shift

	while [ -n "${_LB_EXPAND_QUEUE}" ]
	do
		local _LB_LIST_NAME
		local _LB_EXPAND_QUEUE
		_LB_LIST_NAME="$(echo ${_LB_EXPAND_QUEUE} | cut -d" " -f1)"
		_LB_EXPAND_QUEUE="$(echo ${_LB_EXPAND_QUEUE} | cut -s -d" " -f2-)"
		local _LB_LIST_LOCATION=""
		local _LB_NESTED=0
		local _LB_ENABLED=1

		for _LB_SEARCH_PATH in "${@}"; do
			if [ -e "${_LB_SEARCH_PATH}/${_LB_LIST_NAME}" ]
			then
				_LB_LIST_LOCATION="${_LB_SEARCH_PATH}/${_LB_LIST_NAME}"
				break
			fi
		done

		if [ -z "${_LB_LIST_LOCATION}" ]
		then
			Echo_warning "Unknown package list '${_LB_LIST_NAME}'"
			continue
		fi

		printf "$(cat ${_LB_LIST_LOCATION})\n" | while read _LB_LINE
		do
			case "${_LB_LINE}" in
				\!*)
					local _EXEC
					_EXEC="$(echo ${_LB_LINE} | sed -e 's|^!||')"

					case "${LB_BUILD_WITH_CHROOT}" in
						true)
							chroot chroot sh -c "${_EXEC}"
							;;

						false)
							eval ${_EXEC}
							;;
					esac
					;;

				\#if\ *)
					if [ ${_LB_NESTED} -eq 1 ]
					then
						Echo_error "Nesting conditionals is not supported"
						exit 1
					fi
					_LB_NESTED=1

					local _LB_NEEDLE
					local _LB_HAYSTACK
					local _LB_NEEDLE_PART
					local _LB_HAYSTACK_PART
					_LB_NEEDLE="$(echo "${_LB_LINE}" | cut -d' ' -f3-)"
					_LB_HAYSTACK="$(eval "echo \$LB_$(echo "${_LB_LINE}" | cut -d' ' -f2)")"

					_LB_ENABLED=0
					for _LB_NEEDLE_PART in ${_LB_NEEDLE}
					do
						for _LB_HAYSTACK_PART in ${_LB_HAYSTACK}
						do
							if [ "${_LB_NEEDLE_PART}" = "${_LB_HAYSTACK_PART}" ]
							then
								_LB_ENABLED=1
							fi
						done
					done
					;;

				\#nif\ *)
					if [ ${_LB_NESTED} -eq 1 ]
					then
						Echo_error "Nesting conditionals is not supported"
						exit 1
					fi
					_LB_NESTED=1

					local _LB_NEEDLE
					local _LB_HAYSTACK
					local _LB_NEEDLE_PART
					local _LB_HAYSTACK_PART
					_LB_NEEDLE="$(echo "${_LB_LINE}" | cut -d' ' -f3-)"
					_LB_HAYSTACK="$(eval "echo \$LB_$(echo "${_LB_LINE}" | cut -d' ' -f2)")"

					_LB_ENABLED=1
					for _LB_NEEDLE_PART in ${_LB_NEEDLE}
					do
						for _LB_HAYSTACK_PART in ${_LB_HAYSTACK}
						do
							if [ "${_LB_NEEDLE_PART}" = "${_LB_HAYSTACK_PART}" ]
							then
								_LB_ENABLED=0
							fi
						done
					done
					;;

				\#endif*)
					_LB_NESTED=0
					_LB_ENABLED=1
					;;

				\#*)
					# Skip comments
					;;

				*)
					if [ ${_LB_ENABLED} -eq 1 ]
					then
						echo "${_LB_LINE}"
					fi
					;;

			esac
		done
	done
}

Discover_package_architectures ()
{
	local _LB_EXPANDED_PKG_LIST="${1}"
	local _LB_DISCOVERED_ARCHITECTURES=""

	shift

	if [ -e "${_LB_EXPANDED_PKG_LIST}" ] && [ -s "${_LB_EXPANDED_PKG_LIST}" ]
	then
		local _LB_PACKAGE_LINE
		local _LB_PACKAGE_LINE_PART
		while read _LB_PACKAGE_LINE
		do
			# Lines from the expanded package list may have multiple, space-separated packages
			for _LB_PACKAGE_LINE_PART in ${_LB_PACKAGE_LINE}
			do
				# Looking for <package>:<architecture>
				if [ -n "$(echo ${_LB_PACKAGE_LINE_PART} | awk -F"=" '{print $1}' | awk -F':' '{print $2}')" ]
				then
					_LB_DISCOVERED_ARCHITECTURES="${_LB_DISCOVERED_ARCHITECTURES} $(echo ${_LB_PACKAGE_LINE_PART} | awk -F':' '{print $2}')"
				fi
			done
		done < "${_LB_EXPANDED_PKG_LIST}"

		# Output unique architectures, alpha-sorted, one per line
		echo "${_LB_DISCOVERED_ARCHITECTURES}" | tr -s '[:space:]' '\n' | sort | uniq
	fi
}
