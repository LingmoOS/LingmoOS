# Captive Wi-Fi networks

The Connectivity object at `/org/lingmo/Services/Connectivity`, interface
`org.lingmo.Services.Connectivity`, is owned by `org.lingmo.Services`.
Its read-only properties are `state` (NetworkManager connectivity values:
unknown=0, none=1, portal=2, limited=3, full=4), `checkingEnabled` and `wireless`.
`wireless` means the activated primary route is Wi-Fi; it does not describe
other connected interfaces. State changes emit PropertiesChanged.

The service watches NetworkManager and reads manager and primary-connection
properties asynchronously. Owner loss cancels reads. Generation checks reject
stale replies. A route change requests CheckConnectivity; while Wi-Fi has not
reached full connectivity a 15-second timer requests another check. Errors
are handled by rereading properties. Full connectivity is monitored through
property changes plus periodic snapshots. Disabled/unavailable probing is
reported as unknown, never successful Internet access.

A portal produces one login notification per primary connection. Dismissing
it does not repeatedly open windows. Wi-Fi Settings also provides the current
Internet status and a persistent login button. `openLogin() -> b` opens the
configured HTTP ConnectivityCheckUri through xdg-open, allowing the gateway
to redirect the user's browser. It accepts only HTTP and never opens an
arbitrary redirect target, local file or executable scheme. It returns false
when there is no current Wi-Fi portal or no usable probe URL. DNS failure or
limited connectivity alone does not trigger a login prompt. Authentication
credentials stay in the browser. No browser opens without a user action.

Debian packaging depends on network-manager-config-connectivity-debian to
provide the connectivity probe endpoint and xdg-utils to open the browser.
This uses Debian's existing connectivity check infrastructure and may contact
its probe server; administrator overrides and disabling checks are respected.
Non-Debian installations must configure NetworkManager connectivity probing.
Installing source changes alone does not enable the host's probe configuration.

Validation: build tests/network with CMake, then run CTest. It uses a private
D-Bus NetworkManager mock, not real Wi-Fi or a browser. Real campus gateways,
DNS restrictions, proxy/VPN interaction and browser redirection still require
live testing after installation. A portal on a non-primary Wi-Fi route is not
attributed to that Wi-Fi by this implementation.
