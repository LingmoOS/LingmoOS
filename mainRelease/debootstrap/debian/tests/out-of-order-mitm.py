# -*- coding: utf-8 -*-
#
# Copyright (C) 2023 Debian Install System Team <debian-boot@lists.debian.org>
#
# SPDX-License-Identifier: MIT

"""Flask app which MITM's an archive to generate out-of-order apt lists.

Specifically, it prepends an additional Packages file stanza for a non-existent
lower version of apt: a fixed version of debootstrap will find the second
(correct) apt stanza and succeed; a broken version of debootstrap will find
only the first (non-existent) apt stanza and fail.
"""
import functools
import gzip
import hashlib
import os

import requests
from debian.deb822 import Packages
from flask import Flask, redirect

app = Flask(__name__)

ARCH = os.environ.get("FLASK_ARCH", "amd64")
DIST = os.environ.get("FLASK_DIST", "trixie")
DISTRO = os.environ.get("FLASK_DISTRO", "debian")
MIRROR = os.environ.get("FLASK_MIRROR", "http://deb.debian.org")


if DISTRO in ("debian", "pureos"):
    hash_funcs = [hashlib.md5, hashlib.sha256]
else:
    # Ubuntu includes SHA1 still
    hash_funcs = [hashlib.md5, hashlib.sha1, hashlib.sha256]


def _munge_release_file(url: str) -> bytes:
    """Given a Release file URL, rewrite it for our modified Packages content."""
    original = requests.get(MIRROR + "/" + url).content
    packages_content = _packages_content(
        f"{DISTRO}/dists/{DIST}/main/binary-{ARCH}/Packages"
    )
    size = bytes(str(len(packages_content)), "ascii")
    sums = [
        bytes(hash_func(packages_content).hexdigest(), "ascii")
        for hash_func in hash_funcs
    ]
    new_lines = []
    filename = f"main/binary-{ARCH}/Packages".encode("ascii")
    for line in original.splitlines():
        if not line.endswith(filename):
            new_lines.append(line)
            continue
        new_lines.append(b" ".join([b"", sums.pop(0), size, filename]))
    return b"\n".join(new_lines)


@functools.lru_cache
def _packages_content(url: str) -> bytes:
    """Given a Packages URL, fetch it and prepend a broken apt stanza."""
    resp = requests.get(MIRROR + "/" + url + ".gz")
    upstream_content = gzip.decompress(resp.content)

    # Find the first `apt` stanza
    for stanza in Packages.iter_paragraphs(upstream_content):
        if stanza["Package"] == "apt":
            break

    # Generate the broken stanza
    new_version = stanza["Version"] + "~test"
    stanza["Filename"] = stanza["Filename"].replace(stanza["Version"], new_version)
    stanza["Version"] = new_version

    # Prepend the stanza to the upstream content
    return bytes(stanza) + b"\n" + upstream_content


@app.route("/<path:url>", methods=["GET", "POST"])
def root(url):
    """Handler for all requests."""
    if (
        url == f"{DISTRO}/dists/{DIST}/InRelease"
        or "by-hash" in url
        or "Packages.xz" in url
        or "Packages.gz" in url
    ):
        # 404 these URLs to force clients to fetch by path and without compression, to
        # make MITM easier
        return "", 404
    if url == f"{DISTRO}/dists/{DIST}/Release":
        # If Release is being fetched, return our modified version
        return _munge_release_file(url)
    if url == f"{DISTRO}/dists/{DIST}/main/binary-{ARCH}/Packages":
        # If Packages is being fetched, return our modified version
        return _packages_content(url)
    # For anything we don't need to modify, redirect clients to upstream mirror
    return redirect(f"{MIRROR}/{url}")
