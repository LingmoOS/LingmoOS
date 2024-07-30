Per-process Network Usage Plugin
================================

This plugin tries to track per-process network usage and feeds that back to
ksysguard. Unfortunately, at the moment there is no unprivileged API available
for this information, so this plugin uses a small helper application to work
around that. The helper uses libpcap to do packet capture. To do the packet
capture it needs `cap_net_raw`, but nothing else. To ensure the helper has
`cap_net_raw`, run `setcap cap_net_raw+ep ksgrd_network_helper` as root.

The helper only tracks TCP and UDP traffic, on IPv4 or IPv6 networks. Only the
beginning of each packet is captured, so we only get the packet headers. These
are processed to extract the source and destination IP address and port, which
are matched with sockets and processes.

The matching uses information parsed from `/proc/net/tcp{,6}` and
`/proc/net/udp{,6}` for the sockets, which are mapped to processes by listing
fds from `/proc/${pid}/fd/` and reading their symlink targets. Entries matching
`socket:[${port}]` are used to track socket to process mapping.

Once mapped, we store how much data was received for each process by
accumulating the packet sizes for each socket. Every second this information is
printed to the helper's stdout using the format
`00:00:00|PID|0000|IN|000|OUT|000` or just `00:00:00` if there was no data that
second. The helper's stdout is read and parsed by the network plugin and fed
into ksysguard.
