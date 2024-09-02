# -*- coding: utf-8 -*-
#
# Copyright (C) 2023 Debian Install System Team <debian-boot@lists.debian.org>
# Copyright (C) 2023 Matthias Klumpp <matthias@tenstral.net>
#
# SPDX-License-Identifier: MIT

"""
Flask app which MITM's a legacy-compatibility archive to make it arch:all-only.
"""
import functools
import gzip
import hashlib
import os

import requests
import tempfile
from apt_pkg import TagFile, TagSection
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
    original = requests.get(MIRROR + "/" + url).content.decode('utf-8')
    packages_content = _packages_arch_content(
        f"{DISTRO}/dists/{DIST}/main/binary-{ARCH}/Packages"
    )
    size = str(len(packages_content))
    sums = [
        hash_func(packages_content).hexdigest()
        for hash_func in hash_funcs
    ]
    new_lines = []
    filename = f"main/binary-{ARCH}/Packages"
    for line in original.splitlines():
        if line.startswith('No-Support-for-Architecture-all:'):
            continue
        if line.startswith('Architectures:'):
            if ' all' not in line:
                line += ' all'
            new_lines.append(line)
            continue
        if line.startswith('Acquire-By-Hash:'):
            new_lines.append('Acquire-By-Hash: no')
            continue
        if not line.endswith(filename):
            new_lines.append(line)
            continue
        new_lines.append(" ".join(["", sums.pop(0), size, filename]))

    result = "\n".join(new_lines)
    return result.encode('utf-8')


@functools.lru_cache
def _packages_arch_content(url: str) -> bytes:
    """Given an arch-specific Packages URL, fetch it and filter out arch:all."""
    resp = requests.get(MIRROR + "/" + url + ".gz")
    upstream_content = gzip.decompress(resp.content)

    filtered_sections = []
    with tempfile.NamedTemporaryFile() as tmp_f:
        tmp_f.write(upstream_content)
        tmp_f.flush()

        with TagFile(tmp_f.name) as tf:
            for section in tf:
                if section.get('Architecture') == 'all':
                    continue
                filtered_sections.append(section)

    result = '\n'.join([str(s) for s in filtered_sections])
    return result.encode('utf-8')


@functools.lru_cache
def _packages_indep_content(url: str) -> bytes:
    """Given an arch:all Packages URL, just return its uncompressed contents."""
    resp = requests.get(MIRROR + "/" + url + ".gz")
    upstream_content = gzip.decompress(resp.content)

    return upstream_content


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
    if url == f"{DISTRO}/dists/{DIST}/main/binary-all/Packages":
        return _packages_indep_content(url)
    if url == f"{DISTRO}/dists/{DIST}/main/binary-{ARCH}/Packages":
        # If Packages is being fetched, return our modified version
        return _packages_arch_content(url)
    # For anything we don't need to modify, redirect clients to upstream mirror
    return redirect(f"{MIRROR}/{url}")
