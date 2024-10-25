DNS SRV record support in apt
=============================

Apt supports a subset of the DNS SRV server records protocol as
described in [RFC 2782](https://tools.ietf.org/html/rfc2782) for service discovery.

Before connecting to the requested server APT will send a SRV
record request of the form `_$protocol._tcp._$host`, e.g.
`_http._tcp.ftp.debian.org` or `_http._tcp.security.debian.org`.

If the server sends SRV records
as a reply APT will use those to connect to the server(s) in
this reply. It will honor the `priority` field in the reply.

However it does not implement the `weight` algorithm as described
in RFC 2782. It will use an equal weight for each server of the
same priority.

If connecting to a server fails APT will retry with the next one
and remove the server from the list of valid servers for this
session.

