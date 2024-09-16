#!/bin/sh

## lingmo-system-build(7) - System Build Scripts
## Copyright (C) 2016-2020 The Debian Live team
## Copyright (C) 2006-2015 Daniel Baumann <mail@daniel-baumann.ch>
##
## This program comes with ABSOLUTELY NO WARRANTY; for details see COPYING.
## This is free software, and you are welcome to redistribute it
## under certain conditions; see COPYING for details.


Expand_packagelist ()
{
	local _LINGMO_SYS_BUILD_EXPAND_QUEUE
	_LINGMO_SYS_BUILD_EXPAND_QUEUE="$(basename "${1}")"

	shift

	while [ -n "${_LINGMO_SYS_BUILD_EXPAND_QUEUE}" ]
	do
		local _LINGMO_SYS_BUILD_LIST_NAME
		local _LINGMO_SYS_BUILD_EXPAND_QUEUE
		_LINGMO_SYS_BUILD_LIST_NAME="$(echo ${_LINGMO_SYS_BUILD_EXPAND_QUEUE} | cut -d" " -f1)"
		_LINGMO_SYS_BUILD_EXPAND_QUEUE="$(echo ${_LINGMO_SYS_BUILD_EXPAND_QUEUE} | cut -s -d" " -f2-)"
		local _LINGMO_SYS_BUILD_LIST_LOCATION=""
		local _LINGMO_SYS_BUILD_NESTED=0
		local _LINGMO_SYS_BUILD_ENABLED=1

		for _LINGMO_SYS_BUILD_SEARCH_PATH in "${@}"; do
			if [ -e "${_LINGMO_SYS_BUILD_SEARCH_PATH}/${_LINGMO_SYS_BUILD_LIST_NAME}" ]
			then
				_LINGMO_SYS_BUILD_LIST_LOCATION="${_LINGMO_SYS_BUILD_SEARCH_PATH}/${_LINGMO_SYS_BUILD_LIST_NAME}"
				break
			fi
		done

		if [ -z "${_LINGMO_SYS_BUILD_LIST_LOCATION}" ]
		then
			Echo_warning "Unknown package list '${_LINGMO_SYS_BUILD_LIST_NAME}'"
			continue
		fi

		printf "$(cat ${_LINGMO_SYS_BUILD_LIST_LOCATION})\n" | while read _LINGMO_SYS_BUILD_LINE
		do
			case "${_LINGMO_SYS_BUILD_LINE}" in
				\!*)
					local _EXEC
					_EXEC="$(echo ${_LINGMO_SYS_BUILD_LINE} | sed -e 's|^!||')"

					case "${LINGMO_SYS_BUILD_BUILD_WITH_CHROOT}" in
						true)
							chroot chroot sh -c "${_EXEC}"
							;;

						false)
							eval ${_EXEC}
							;;
					esac
					;;

				\#if\ *)
					if [ ${_LINGMO_SYS_BUILD_NESTED} -eq 1 ]
					then
						Echo_error "Nesting conditionals is not supported"
						exit 1
					fi
					_LINGMO_SYS_BUILD_NESTED=1

					local _LINGMO_SYS_BUILD_NEEDLE
					local _LINGMO_SYS_BUILD_HAYSTACK
					local _LINGMO_SYS_BUILD_NEEDLE_PART
					local _LINGMO_SYS_BUILD_HAYSTACK_PART
					_LINGMO_SYS_BUILD_NEEDLE="$(echo "${_LINGMO_SYS_BUILD_LINE}" | cut -d' ' -f3-)"
					_LINGMO_SYS_BUILD_HAYSTACK="$(eval "echo \$LINGMO_SYS_BUILD_$(echo "${_LINGMO_SYS_BUILD_LINE}" | cut -d' ' -f2)")"

					_LINGMO_SYS_BUILD_ENABLED=0
					for _LINGMO_SYS_BUILD_NEEDLE_PART in ${_LINGMO_SYS_BUILD_NEEDLE}
					do
						for _LINGMO_SYS_BUILD_HAYSTACK_PART in ${_LINGMO_SYS_BUILD_HAYSTACK}
						do
							if [ "${_LINGMO_SYS_BUILD_NEEDLE_PART}" = "${_LINGMO_SYS_BUILD_HAYSTACK_PART}" ]
							then
								_LINGMO_SYS_BUILD_ENABLED=1
							fi
						done
					done
					;;

				\#nif\ *)
					if [ ${_LINGMO_SYS_BUILD_NESTED} -eq 1 ]
					then
						Echo_error "Nesting conditionals is not supported"
						exit 1
					fi
					_LINGMO_SYS_BUILD_NESTED=1

					local _LINGMO_SYS_BUILD_NEEDLE
					local _LINGMO_SYS_BUILD_HAYSTACK
					local _LINGMO_SYS_BUILD_NEEDLE_PART
					local _LINGMO_SYS_BUILD_HAYSTACK_PART
					_LINGMO_SYS_BUILD_NEEDLE="$(echo "${_LINGMO_SYS_BUILD_LINE}" | cut -d' ' -f3-)"
					_LINGMO_SYS_BUILD_HAYSTACK="$(eval "echo \$LINGMO_SYS_BUILD_$(echo "${_LINGMO_SYS_BUILD_LINE}" | cut -d' ' -f2)")"

					_LINGMO_SYS_BUILD_ENABLED=1
					for _LINGMO_SYS_BUILD_NEEDLE_PART in ${_LINGMO_SYS_BUILD_NEEDLE}
					do
						for _LINGMO_SYS_BUILD_HAYSTACK_PART in ${_LINGMO_SYS_BUILD_HAYSTACK}
						do
							if [ "${_LINGMO_SYS_BUILD_NEEDLE_PART}" = "${_LINGMO_SYS_BUILD_HAYSTACK_PART}" ]
							then
								_LINGMO_SYS_BUILD_ENABLED=0
							fi
						done
					done
					;;

				\#endif*)
					_LINGMO_SYS_BUILD_NESTED=0
					_LINGMO_SYS_BUILD_ENABLED=1
					;;

				\#*)
					# Skip comments
					;;

				*)
					if [ ${_LINGMO_SYS_BUILD_ENABLED} -eq 1 ]
					then
						echo "${_LINGMO_SYS_BUILD_LINE}"
					fi
					;;

			esac
		done
	done
}

Discover_package_architectures ()
{
	local _LINGMO_SYS_BUILD_EXPANDED_PKG_LIST="${1}"
	local _LINGMO_SYS_BUILD_DISCOVERED_ARCHITECTURES=""

	shift

	if [ -e "${_LINGMO_SYS_BUILD_EXPANDED_PKG_LIST}" ] && [ -s "${_LINGMO_SYS_BUILD_EXPANDED_PKG_LIST}" ]
	then
		local _LINGMO_SYS_BUILD_PACKAGE_LINE
		local _LINGMO_SYS_BUILD_PACKAGE_LINE_PART
		while read _LINGMO_SYS_BUILD_PACKAGE_LINE
		do
			# Lines from the expanded package list may have multiple, space-separated packages
			for _LINGMO_SYS_BUILD_PACKAGE_LINE_PART in ${_LINGMO_SYS_BUILD_PACKAGE_LINE}
			do
				# Looking for <package>:<architecture>
				if [ -n "$(echo ${_LINGMO_SYS_BUILD_PACKAGE_LINE_PART} | awk -F"=" '{print $1}' | awk -F':' '{print $2}')" ]
				then
					_LINGMO_SYS_BUILD_DISCOVERED_ARCHITECTURES="${_LINGMO_SYS_BUILD_DISCOVERED_ARCHITECTURES} $(echo ${_LINGMO_SYS_BUILD_PACKAGE_LINE_PART} | awk -F':' '{print $2}')"
				fi
			done
		done < "${_LINGMO_SYS_BUILD_EXPANDED_PKG_LIST}"

		# Output unique architectures, alpha-sorted, one per line
		echo "${_LINGMO_SYS_BUILD_DISCOVERED_ARCHITECTURES}" | tr -s '[:space:]' '\n' | sort | uniq
	fi
}
