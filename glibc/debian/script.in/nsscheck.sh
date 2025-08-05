	    echo -n "Checking for services that may need to be restarted..."
	    # Only get the ones that are installed, of the same architecture
	    # as libc (or arch all) and configured. Restart openssh-server even
	    # if only half-configured to continue accepting new connections
	    # during the upgrade.
	    check=$(dpkg-query -W -f='${binary:Package} ${Status} ${Architecture}\n' $check 2> /dev/null | \
			grep -E "(^openssh-server .* unpacked|installed) (all|${DPKG_MAINTSCRIPT_ARCH})$" | sed 's/[: ].*//')
	    # some init scripts don't match the package names
	    check=$(echo $check | \
		    sed -e's/\bapache2.2-common\b/apache2/g' \
			-e's/\bat\b/atd/g' \
			-e's/\bdovecot-common\b/dovecot/g' \
			-e's/\bexim4-base\b/exim4/g' \
			-e's/\blpr\b/lpd/g' \
			-e's/\blpr-ppd\b/lpd-ppd/g' \
			-e's/\bmysql-server\b/mysql/g' \
			-e's/\bopenssh-server\b/ssh/g' \
			-e's/\bsasl2-bin\b/saslauthd/g' \
			-e's/\bsamba\b/smbd/g' \
			-e's/\bpostgresql-common\b/postgresql/g' \
		    )
	    echo
	    echo "Checking init scripts..."
	    for service in $check; do
		invoke-rc.d ${service} status >/dev/null 2>/dev/null && status=0 || status=$?
		if [ "$status" = "0" ] || [ "$status" = "2" ] ; then
		    services="$service $services"
		elif [ "$status" = "100" ] ; then
		    echo "WARNING: init script for $service not found."
		fi
	    done
