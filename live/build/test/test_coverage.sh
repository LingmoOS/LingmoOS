#!/bin/bash

# Test the code coverage using bashcov (https://github.com/infertux/bashcov)
#
# Install bashcov with: gem install bashcov

try_this()
{
	local EXPECTED_RETURN_VALUE
	local TEST_NAME

	EXPECTED_RETURN_VALUE=${1}
	TEST_NAME="${2}"
	shift 2

	COUNTER=$((COUNTER + 1))
	if [ ${COUNTER} -eq ${SINGLE_TEST} -o ${SINGLE_TEST} -eq 0 ]; then
		echo "Starting test #${COUNTER} ${TEST_NAME}"
		bashcov --root ${LIVE_BUILD} -- "$@"
		RETVAL=${?}
		if [ ${RETVAL} -ne ${EXPECTED_RETURN_VALUE} ]; then
			echo "Error: the test does not have the expected return value (expected: ${EXPECTED_RETURN_VALUE} actual: ${RETVAL})"
			exit 1
		fi
		# Cleanup after the test
		rm -fr auto config local .build
	else
		echo "Skipping test #${COUNTER} ${TEST_NAME}"
	fi
}

# The outermost wrapper layer
test_wrapper_layer()
{
	try_this 0 "Default lb config" ${LIVE_BUILD}/frontend/lb config

	# Increase coverage by adding error scenarios
	try_this 1 "No argument to lb" ${LIVE_BUILD}/frontend/lb
	try_this 1 "Unknown argument to lb" ${LIVE_BUILD}/frontend/lb does_not_exist
}

# The configuration generator
test_lb_config()
{
	try_this 0 "Default config" ${LIVE_BUILD}/scripts/build/config
}

# Testing various proxy settings
test_proxy()
{
	export http_proxy

	http_proxy=""; try_this 0 "All proxy options turned off" ${LIVE_BUILD}/scripts/build/config --apt-options "-o Acquire::http::Proxy-Auto-Detect= -o Acquire::http::ProxyAutoDetect= -o Acquire::http::Proxy="
	http_proxy=""; try_this 0 "All proxy options turned off, with proxy scripts returning an error code" ${LIVE_BUILD}/scripts/build/config --apt-options "-o Acquire::http::Proxy-Auto-Detect=/bin/false -o Acquire::http::ProxyAutoDetect=/bin/false -o Acquire::http::Proxy="

	cat > ${LIVE_BUILD}/tmp.sh << EOF
#!/bin/sh
echo ''
EOF
	chmod u+x ${LIVE_BUILD}/tmp.sh
	http_proxy=""; try_this 0 "All proxy options turned off, with proxy scripts returning an empty line" ${LIVE_BUILD}/scripts/build/config --apt-options "-o Acquire::http::Proxy-Auto-Detect=${LIVE_BUILD}/tmp.sh -o Acquire::http::ProxyAutoDetect=${LIVE_BUILD}/tmp.sh -o Acquire::http::Proxy="

	cat > ${LIVE_BUILD}/tmp.sh << EOF
#!/bin/sh
echo 'DIRECT'
EOF
	chmod u+x ${LIVE_BUILD}/tmp.sh
	http_proxy=""; try_this 0 "All proxy options turned off, with proxy scripts returning 'DIRECT'" ${LIVE_BUILD}/scripts/build/config --apt-options "-o Acquire::http::Proxy-Auto-Detect=${LIVE_BUILD}/tmp.sh -o Acquire::http::ProxyAutoDetect=${LIVE_BUILD}/tmp.sh -o Acquire::http::Proxy="

	cat > ${LIVE_BUILD}/tmp1.sh << EOF
#!/bin/sh
echo 'location1'
EOF
	chmod u+x ${LIVE_BUILD}/tmp1.sh
	cat > ${LIVE_BUILD}/tmp2.sh << EOF
#!/bin/sh
echo 'location2'
EOF
	chmod u+x ${LIVE_BUILD}/tmp2.sh
	http_proxy=""; try_this 1 "Error: mismatch Acquire::http::Proxy-Auto-Detect and Acquire::http::ProxyAutoDetect" ${LIVE_BUILD}/scripts/build/config --apt-options "-o Acquire::http::Proxy-Auto-Detect=${LIVE_BUILD}/tmp1.sh -o Acquire::http::ProxyAutoDetect=${LIVE_BUILD}/tmp2.sh -o Acquire::http::Proxy="
	http_proxy=""; try_this 1 "Error: mismatch Acquire::http::Proxy-Auto-Detect and Acquire::http::Proxy" ${LIVE_BUILD}/scripts/build/config --apt-options "-o Acquire::http::Proxy-Auto-Detect=${LIVE_BUILD}/tmp1.sh -o Acquire::http::ProxyAutoDetect= -o Acquire::http::Proxy=location3"
	http_proxy="location4"; try_this 1 "Error: mismatch Acquire::http::Proxy-Auto-Detect and http_proxy" ${LIVE_BUILD}/scripts/build/config --apt-options "-o Acquire::http::Proxy-Auto-Detect=${LIVE_BUILD}/tmp1.sh -o Acquire::http::ProxyAutoDetect= -o Acquire::http::Proxy="
	http_proxy=""; try_this 1 "Error: mismatch Acquire::http::Proxy-Auto-Detect and command line --apt-http-proxy" ${LIVE_BUILD}/scripts/build/config --apt-options "-o Acquire::http::Proxy-Auto-Detect=${LIVE_BUILD}/tmp1.sh -o Acquire::http::ProxyAutoDetect= -o Acquire::http::Proxy=" --apt-http-proxy=location5
	# Reset at the end
	export -n http_proxy
}

# Use a path relative to the git code
export LIVE_BUILD=$(realpath $(dirname $BASH_SOURCE)/..)

# Allow fine-grained test selection. If desired, pass along the test number on the command line
COUNTER=0
SINGLE_TEST=${1:-0}

rm -fr ${LIVE_BUILD}/coverage
# Invoke the outermost wrapper layer
test_wrapper_layer

# Bashcov somehow has issues in following the paths,
# so invoke config directly
# This is not the recommended way to invoke live-build
test_lb_config

# Testing various proxy settings
test_proxy