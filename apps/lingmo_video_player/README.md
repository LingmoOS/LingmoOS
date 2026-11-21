# Video Player

The video player of Lingmo Desktop. It plays local files and network
streams through libmpv, with subtitles, folder playlists, resume and seek-bar
previews, behind overlay controls that hide while the video plays.

## Dependencies

Video Player uses lingmo-toolkit, libmpv and ICU.

```sh
sudo apt install cmake pkg-config libmpv-dev libicu-dev libgtest-dev
```

The tests make their sample clips with `ffmpeg`.

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
