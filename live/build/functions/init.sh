#!/bin/sh

## live-build(7) - System Build Scripts
## Copyright (C) 2016-2020 The Debian Live team
## Copyright (C) 2006-2015 Daniel Baumann <mail@daniel-baumann.ch>
##
## This program comes with ABSOLUTELY NO WARRANTY; for details see COPYING.
## This is free software, and you are welcome to redistribute it
## under certain conditions; see COPYING for details.


Auto_build_config ()
{
	# Automatically build config
	if [ -x auto/config ] && ! Stagefile_exists config; then
		Echo_message "Automatically populating config tree."
		lb config
	fi
}

Init_config_data ()
{
	Arguments "${@}"

	Read_conffiles $(Common_conffiles)
	Prepare_config

	# Validating config
	if [ "${CONFIG_VALIDATED}" != "true" ]; then
		Validate_config
		export CONFIG_VALIDATED="true"
	fi

	# Apt v2.0.1 introduced color support, but it needs to be explicitly enabled.
	# This is done here so that the injection of the option does not get stored in the config file.
	if [ "${_COLOR_OUT}" = "true" ] && [ "${_COLOR_ERR}" = "true" ]; then
		APT_OPTIONS="${APT_OPTIONS} -o APT::Color=true"
		APTITUDE_OPTIONS="${APTITUDE_OPTIONS} -o APT::Color=true"
	else
		APT_OPTIONS="${APT_OPTIONS} -o APT::Color=false"
		APTITUDE_OPTIONS="${APTITUDE_OPTIONS} -o APT::Color=false"
	fi
}

# "Auto" script redirection.
#
# As a matter of convenience users can have a set of saved commandline options
# which will be automatically included in every execution of live-build. How
# this works is that the save file is itself a shell script saved in the config
# directory (one per top-level live-build command in fact). When `lb config`,
# `lb build` or `lb clean` is run, these scripts, if they see that an "auto"
# file exists in the config, they run that file, passing along any user
# arguments, and terminate once that ends. The "auto" script simply re-executes
# the same command (e.g. `lb config`), only with a first param of "noauto",
# used to stop an infinite loop of further redirection, then a fixed saved set
# of command line options, as saved in the file by the user, then any
# additional command line arguments passed into the script. This is simply a
# means of injecting a saved set of command line options into the execution of
# live-build.
#
# As for this function, it is a simple helper, used by the top-level commands
# to perform the redirection if the relevant "auto" file exists. It should only
# be called if the calling command script was not run with "noauto" as the
# first argument (the purpose of which was just described).
Maybe_auto_redirect ()
{
	local TYPE="${1}"; shift

	case "${TYPE}" in
		clean|config|build)
			;;
		*)
			Echo_error "Unknown auto redirect type"
			exit 1
			;;
	esac

	local AUTO_SCRIPT="auto/${TYPE}"
	if [ -x "${AUTO_SCRIPT}" ]; then
		Echo_message "Executing ${AUTO_SCRIPT} script."
		./"${AUTO_SCRIPT}" "${@}"
		exit ${?}
	elif [ -f "${AUTO_SCRIPT}" ]; then
		Echo_warning "The ${AUTO_SCRIPT} script exists but is not executable, ignoring."
	fi
}
