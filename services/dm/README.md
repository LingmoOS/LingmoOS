# lingmo-dm

The Lingmo display manager. It runs the login screen and starts the Wayland
session the user picks.

After the configured account completes Lingmo initial setup, the root-owned
display manager removes the setup session entry so it is no longer offered on
the login screen.

## Dependencies

The base library of [lingmo-toolkit](https://github.com/lingmoos/lingmo-toolkit)
and PAM; `lingmo-aurora` at runtime. GoogleTest for the tests.

## Build

```shell
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build
sudo cmake --install build
```

A source install needs the login screen's user once:

```shell
sudo adduser --system --group --home /var/lib/lingmo-greeter lingmo-greeter
```

## Enable

```shell
sudo systemctl disable sddm
sudo systemctl enable lingmo-dm
```

To log a user in without a password once per boot, as a live system does,
add to `/etc/lingmo-dm.conf`:

```ini
[Autologin]
User=lingmo
# Optional; the default session otherwise.
Session=lingmo-wayland
```

## Tests

```shell
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```
