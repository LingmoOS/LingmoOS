# Distribution {#distributing}

Baloo is developed and tested exclusively for Linux. While it may run on other
unix based systems. It is not recommended, and certainly not tested.

We recommend not packing Baloo for Windows or OSX as both these operating
systems offer their own file searching solutions which better integrate with
the native system than Baloo ever will.

Baloo may run on 32-bit systems, but it has not been tested and may not work
correctly. Please test and let us know by filing a bug at https://bugs.kde.org/enter_bug.cgi?product=frameworks-baloo.

**Supported Kernels:** Linux
**Support Architecture:** x64

## File Indexing Plugins

Baloo relies on KFileMetaData to extract content from the files. KFileMetadata
ships with a number of plugins which can be enabled or disabled. We recommend
shipping all KFileMetaData plugins. Specially ffmpeg by default.

Without the indexers, Baloo cannot function to its full potential.
