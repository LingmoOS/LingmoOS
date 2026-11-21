#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
# SPDX-FileCopyrightText: 2026 LingmoOS Contributors
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Makes hfsplus.img.gz, the HFS+ volume the disk image tests read. Run as root on
# Linux: an empty volume is laid out here, then the kernel's hfsplus driver fills
# it, so the files are written by an implementation other than the reader's.
# Compressed files get their decmpfs attribute through the driver; their
# compressed flag and resource fork are patched in afterwards, since the driver
# sets neither. A resource fork is written as resources/<name> first and its
# record then names the same blocks.

import ctypes
import gzip
import os
import struct
import subprocess
import sys
import tempfile
import zlib

BLOCK = 4096
TOTAL_BLOCKS = 1024
NODE = 4096
HFS_EPOCH = 2082844800
# What is laid out here carries a fixed time; the driver stamps what it writes.
NOW = 1767225600 + HFS_EPOCH
VOLUME_NAME = 'Fixture Disk'


def pattern(label, size):
    """Compressible bytes that still differ from block to block."""
    out = bytearray()
    for block in range(0, size, BLOCK):
        line = f'{label} block {block // BLOCK:06d}\n'.encode()
        out += (line * (BLOCK // len(line) + 1))[:BLOCK]
    return bytes(out[:size])


def fork(logical, start, count):
    return struct.pack('>QII', logical, count * BLOCK, count) + struct.pack('>II', start, count) + bytes(56)


def unistr(name):
    encoded = name.encode('utf-16-be')
    return struct.pack('>H', len(encoded) // 2) + encoded


def header_node(depth, root, leaf_records, first_leaf, last_leaf, max_key, total_nodes, used_nodes,
                compare, attributes):
    node = bytearray(NODE)
    struct.pack_into('>IIbBHH', node, 0, 0, 0, 1, 0, 3, 0)
    struct.pack_into('>HIIIIHHIIHIBBI', node, 14, depth, root, leaf_records, first_leaf, last_leaf, NODE,
                     max_key, total_nodes, total_nodes - used_nodes, 0, BLOCK, 0, compare, attributes)
    map_offset = 14 + 106 + 128
    for n in range(used_nodes):
        node[map_offset + n // 8] |= 0x80 >> (n % 8)
    offsets = [14, 120, 248, NODE - 8]
    for i, offset in enumerate(offsets):
        struct.pack_into('>H', node, NODE - 2 * (i + 1), offset)
    return node


def leaf_node(records):
    node = bytearray(NODE)
    struct.pack_into('>IIbBHH', node, 0, 0, 0, -1, 1, len(records), 0)
    offset = 14
    for i, record in enumerate(records):
        struct.pack_into('>H', node, NODE - 2 * (i + 1), offset)
        node[offset:offset + len(record)] = record
        offset += len(record)
    struct.pack_into('>H', node, NODE - 2 * (len(records) + 1), offset)
    return node


def blank_volume():
    allocation = (1, 1)
    extents = (2, 4)
    catalog = (6, 16)
    attributes = (22, 4)
    used = [0, TOTAL_BLOCKS - 1]
    for start, count in (allocation, extents, catalog, attributes):
        used += range(start, start + count)

    image = bytearray(TOTAL_BLOCKS * BLOCK)

    bitmap = bytearray(BLOCK)
    for block in used:
        bitmap[block // 8] |= 0x80 >> (block % 8)
    image[allocation[0] * BLOCK:(allocation[0] + 1) * BLOCK] = bitmap

    def put_tree(place, nodes):
        for i, node in enumerate(nodes):
            at = place[0] * BLOCK + i * NODE
            image[at:at + NODE] = node

    put_tree(extents, [header_node(0, 0, 0, 0, 0, 10, extents[1], 1, 0, 0x2)])
    put_tree(attributes, [header_node(0, 0, 0, 0, 0, 266, attributes[1], 1, 0, 0x6)])

    root_key = struct.pack('>I', 1) + unistr(VOLUME_NAME)
    folder = struct.pack('>hHII', 1, 0, 0, 2) + struct.pack('>IIIII', NOW, NOW, NOW, NOW, 0)
    folder += struct.pack('>IIBBHI', 0, 0, 0, 0, 0o40755, 0) + bytes(32) + struct.pack('>II', 0, 0)
    thread_key = struct.pack('>I', 2) + unistr('')
    thread = struct.pack('>hhI', 3, 0, 1) + unistr(VOLUME_NAME)
    records = [struct.pack('>H', len(root_key)) + root_key + folder,
               struct.pack('>H', len(thread_key)) + thread_key + thread]
    put_tree(catalog, [header_node(1, 1, 2, 1, 1, 516, catalog[1], 2, 0xCF, 0x6), leaf_node(records)])

    header = bytearray(512)
    struct.pack_into('>HHIII', header, 0, 0x482B, 4, 0x100, 0x31302E30, 0)
    struct.pack_into('>IIIIIIIIIIIIIIQ', header, 16, NOW, NOW, 0, NOW, 0, 0, BLOCK,
                     TOTAL_BLOCKS, TOTAL_BLOCKS - len(used), attributes[0] + attributes[1], BLOCK, BLOCK, 16, 1, 1)
    header[112:192] = fork(BLOCK, *allocation)
    header[192:272] = fork(extents[1] * BLOCK, *extents)
    header[272:352] = fork(catalog[1] * BLOCK, *catalog)
    header[352:432] = fork(attributes[1] * BLOCK, *attributes)
    image[1024:1536] = header
    image[len(image) - 1024:len(image) - 512] = header
    return image


def decmpfs(kind, size, payload=b''):
    return b'fpmc' + struct.pack('<IQ', kind, size) + payload


lzfse = ctypes.CDLL('liblzfse.so')
lzfse.lzfse_encode_buffer.restype = ctypes.c_size_t
lzfse.lzfse_encode_buffer.argtypes = [ctypes.c_char_p, ctypes.c_size_t, ctypes.c_char_p, ctypes.c_size_t,
                                      ctypes.c_void_p]


def lzfse_encode(data):
    out = ctypes.create_string_buffer(len(data) + 4096)
    written = lzfse.lzfse_encode_buffer(out, len(out), data, len(data), None)
    assert written > 0
    return out.raw[:written]


def lzvn_encode(data):
    # The encoder takes LZVN for small inputs; decmpfs stores its payload without the framing.
    framed = lzfse_encode(data)
    assert framed[:4] == b'bvxn' and framed[-4:] == b'bvx$', framed[:4]
    return framed[12:-4]


def blocks_table(blocks):
    offset = (len(blocks) + 1) * 4
    table = b''
    for block in blocks:
        table += struct.pack('<I', offset)
        offset += len(block)
    return table + struct.pack('<I', offset) + b''.join(blocks)


def zlib_resource(blocks):
    """A resource map holding one 'cmpf' resource, as decmpfs type 4 stores it."""
    body = struct.pack('<I', len(blocks))
    offset = 4 + 8 * len(blocks)
    entries = b''
    for block in blocks:
        entries += struct.pack('<II', offset, len(block))
        offset += len(block)
    body += entries + b''.join(blocks)
    data = struct.pack('>I', len(body)) + body
    header = struct.pack('>IIII', 256, 256 + len(data), len(data), 50)
    return header + bytes(256 - 16) + data + bytes(50)


def chunks(data, size=65536):
    return [data[i:i + size] for i in range(0, len(data), size)]


COMPRESSED = {
    'zlib-inline.txt': lambda d: (decmpfs(3, len(d), zlib.compress(d)), None),
    'stored-inline.txt': lambda d: (decmpfs(3, len(d), b'\xff' + d), None),
    'zlib-resource.txt': lambda d: (decmpfs(4, len(d)), zlib_resource([zlib.compress(c) for c in chunks(d)])),
    'lzvn-inline.txt': lambda d: (decmpfs(7, len(d), lzvn_encode(d)), None),
    'lzvn-resource.txt': lambda d: (decmpfs(8, len(d)), blocks_table([lzvn_encode(d)])),
    'lzfse-resource.txt': lambda d: (decmpfs(12, len(d)), blocks_table([lzfse_encode(c) for c in chunks(d)])),
}

COMPRESSED_SIZES = {
    'zlib-inline.txt': 3000,
    'stored-inline.txt': 700,
    'zlib-resource.txt': 200000,
    'lzvn-inline.txt': 2000,
    'lzvn-resource.txt': 3000,
    'lzfse-resource.txt': 150000,
}


def populate(root):
    def write(path, data):
        with open(os.path.join(root, path), 'wb') as f:
            f.write(data)

    write('hello.txt', b'Hello, disk image.\n')
    write('empty', b'')
    os.makedirs(os.path.join(root, 'Folder/nested'))
    write('Folder/nested/deep.txt', pattern('deep', 5000))
    write('Caf\u00e9 \u00fcn\u00efcode.txt', b'unicode\n')
    # Left alone by the decomposition; the ohm sign is no omega.
    write('\u2126 \u00c5ngstr\u00f6m.txt', b'ohm\n')
    # The driver stores ':' as the '/' the catalog keeps.
    write('a:b.txt', b'colon\n')
    os.symlink('hello.txt', os.path.join(root, 'link'))
    os.symlink('/Applications', os.path.join(root, 'Applications'))
    write('original.bin', pattern('linked', 9000))
    os.link(os.path.join(root, 'original.bin'), os.path.join(root, 'Folder/hardlink.bin'))
    os.chmod(os.path.join(root, 'hello.txt'), 0o640)

    os.makedirs(os.path.join(root, 'resources'))
    for name, make in COMPRESSED.items():
        data = pattern(name, COMPRESSED_SIZES[name])
        value, resource = make(data)
        write(name, b'')
        os.setxattr(os.path.join(root, name), 'osx.com.apple.decmpfs', value)
        if resource:
            write(f'resources/{name}', resource)

    # Fill the volume with one-block files, free every other one and write a file into
    # the holes, so it needs more extents than its catalog record holds.
    os.makedirs(os.path.join(root, 'filler'))
    count = 0
    try:
        while True:
            write(f'filler/{count:04d}', pattern(f'filler {count}', BLOCK))
            os.sync()
            count += 1
    except OSError:
        if os.path.exists(os.path.join(root, f'filler/{count:04d}')):
            os.remove(os.path.join(root, f'filler/{count:04d}'))
    for i in range(0, count, 2):
        os.remove(os.path.join(root, f'filler/{i:04d}'))
    os.sync()
    with open(os.path.join(root, 'fragmented.bin'), 'wb') as f:
        data = pattern('fragmented', 40 * BLOCK)
        for i in range(0, len(data), BLOCK):
            f.write(data[i:i + BLOCK])
            f.flush()
            os.fsync(f.fileno())


def catalog_files(image):
    """The catalog's file records as {name: offset in the image}, walked leaf by leaf."""
    def fork_extents(data):
        return [struct.unpack_from('>II', data, 16 + i * 8) for i in range(8)]

    def mapper(extents):
        def offset(position):
            for start, count in extents:
                if position < count * BLOCK:
                    return start * BLOCK + position
                position -= count * BLOCK
            raise ValueError(position)
        return offset

    def leaves(offset):
        node = struct.unpack_from('>I', image, offset(0) + 24)[0]
        while node:
            base = offset(node * NODE)
            yield base
            node = struct.unpack_from('>I', image, base)[0]

    header = 1024
    catalog = image[header + 272:header + 352]
    extents = fork_extents(catalog)
    # The catalog may go on in the extents overflow file, whose own extents all fit its header.
    overflow = mapper(fork_extents(image[header + 192:header + 272]))
    more = []
    for base in leaves(overflow):
        for i in range(struct.unpack_from('>H', image, base + 10)[0]):
            record = base + struct.unpack_from('>H', image, base + NODE - 2 * (i + 1))[0]
            fork_type, file_id, start = struct.unpack_from('>BxII', image, record + 2)
            if file_id == 4 and fork_type == 0:
                more.append((start, [struct.unpack_from('>II', image, record + 12 + j * 8) for j in range(8)]))
    for _, records in sorted(more):
        extents += records
    extents = [extent for extent in extents if extent[1]]
    assert sum(count for _, count in extents) == struct.unpack_from('>I', catalog, 12)[0]
    offset = mapper(extents)

    files = {}
    for base in leaves(offset):
        count = struct.unpack_from('>H', image, base + 10)[0]
        for i in range(count):
            record = base + struct.unpack_from('>H', image, base + NODE - 2 * (i + 1))[0]
            key_length = struct.unpack_from('>H', image, record)[0]
            name_length = struct.unpack_from('>H', image, record + 6)[0]
            name = image[record + 8:record + 8 + 2 * name_length].decode('utf-16-be')
            data = record + 2 + key_length
            if struct.unpack_from('>h', image, data)[0] == 2:
                parent = struct.unpack_from('>I', image, record + 2)[0]
                files[(parent, name)] = data
    return files


def patch_compressed(image):
    files = catalog_files(image)
    for name in COMPRESSED:
        found = [(parent, data) for (parent, n), data in files.items() if n == name]
        empty = [data for _, data in found if struct.unpack_from('>Q', image, data + 88)[0] == 0]
        stored = [data for _, data in found if struct.unpack_from('>Q', image, data + 88)[0] != 0]
        assert len(empty) == 1 and len(stored) <= 1, (name, found)
        image[empty[0] + 41] |= 0x20
        if stored:
            image[empty[0] + 168:empty[0] + 248] = image[stored[0] + 88:stored[0] + 168]


def main():
    output = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'hfsplus.img.gz')
    with tempfile.TemporaryDirectory() as work:
        path = os.path.join(work, 'volume.img')
        with open(path, 'wb') as f:
            f.write(blank_volume())

        mount_point = os.path.join(work, 'mnt')
        os.mkdir(mount_point)
        device = subprocess.check_output(['losetup', '-f', '--show', path], text=True).strip()
        try:
            subprocess.check_call(['mount', '-t', 'hfsplus', device, mount_point])
            try:
                populate(mount_point)
            finally:
                subprocess.check_call(['umount', mount_point])
        finally:
            subprocess.check_call(['losetup', '-d', device])

        with open(path, 'rb') as f:
            image = bytearray(f.read())
        patch_compressed(image)
        with open(output, 'wb') as f:
            f.write(gzip.compress(bytes(image), 9, mtime=0))
    print(output)


if __name__ == '__main__':
    sys.exit(main())
