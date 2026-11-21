# System Monitor

The system monitor of Lingmo Desktop. It shows CPU, memory, disk, network
and GPU usage, and lists the running processes so they can be stopped.

## Dependencies

System Monitor uses lingmo-toolkit and ICU; GoogleTest for the tests.

```sh
sudo apt install cmake pkg-config libicu-dev libgtest-dev
```

## Build and install

```sh
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr ..
make
sudo make install
```

## Tests

```sh
cmake -DBUILD_TESTING=ON ..
make
dbus-run-session -- ctest --output-on-failure
```

## License

BSD-3-Clause; see [LICENSE](LICENSE).
