# Update retry regression

This test builds the real UpdateManager with the Framework UpdateStatus client and runs them on an isolated session bus. PATH contains only temporary test doubles for curl, pkexec and apt: it never runs real package-management commands or accesses the update website.

Covered scenarios:

- API transport failure and malformed response expose a diagnostic.
- The API check sends BUILD_VERSION and reads updateAvailable, summary and releaseDate.
- Retrying a failed check reaches an available-update state and clears the old error.
- Failed APT refresh or package listing remains a failure, rather than reporting stale data as up to date.
- Upgrade failure exposes stderr and can recover after a successful new check.
- A system upgrade runs dist-upgrade; a software update runs upgrade.
- A successful upgrade reports RestartRequired and later checks do not clear it.
- Upgrade progress and the restart prompt are sent as notifications, with a
  restart action on the final one.
- D-Bus method errors are returned to the client instead of silently ignored.

Build with the adjacent Framework checkout (override FRAMEWORK_DIR if necessary):

~~~sh
cmake -S lingmo-core/tests/update -B build-update-test -DCMAKE_PREFIX_PATH=/path/to/sdk
cmake --build build-update-test
ctest --test-dir build-update-test --output-on-failure
~~~

The Settings UI regression is settings/tests/ui/update_test.cpp and is included when SETTINGS_PAGES contains update. It checks immediate pending feedback, failed request retry, and disabled controls while the service is absent or checking.
