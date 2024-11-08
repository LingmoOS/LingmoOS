import checksymbolslib.db as m


def test_empty_db():
    db = m.DB()
    assert str(db) == '{}'


def test_one_definition():
    db = m.DB()
    db.add_symbol_definition('LINGMO_foo', 'foo/Config.in', 7)
    assert str(db) == str({
        'LINGMO_foo': {'definition': {'foo/Config.in': [7]}},
        })


def test_three_definitions():
    db = m.DB()
    db.add_symbol_definition('LINGMO_foo', 'foo/Config.in', 7)
    db.add_symbol_definition('LINGMO_foo', 'foo/Config.in', 9)
    db.add_symbol_definition('LINGMO_bar', 'bar/Config.in', 5)
    assert str(db) == str({
        'LINGMO_foo': {'definition': {'foo/Config.in': [7, 9]}},
        'LINGMO_bar': {'definition': {'bar/Config.in': [5]}},
        })


def test_definition_and_usage():
    db = m.DB()
    db.add_symbol_definition('LINGMO_foo', 'foo/Config.in', 7)
    db.add_symbol_usage('LINGMO_foo', 'foo/Config.in', 9)
    assert str(db) == str({
        'LINGMO_foo': {'definition': {'foo/Config.in': [7]}, 'normal usage': {'foo/Config.in': [9]}},
        })


def test_all_entry_types():
    db = m.DB()
    db.add_symbol_choice('LINGMO_foo', 'foo/Config.in', 7)
    db.add_symbol_definition('LINGMO_foo', 'foo/Config.in', 7)
    db.add_symbol_definition('LINGMO_bar', 'bar/Config.in', 700)
    db.add_symbol_helper('LINGMO_bar', 'bar/Config.in', 700)
    db.add_symbol_legacy_definition('LINGMO_baz', 'Config.in.legacy', 7000)
    db.add_symbol_legacy_note('LINGMO_baz', 'Config.in.legacy', 7001)
    db.add_symbol_legacy_usage('LINGMO_bar', 'Config.in.legacy', 7001)
    db.add_symbol_select('LINGMO_bar', 'Config.in.legacy', 7001)
    db.add_symbol_usage('LINGMO_foo', 'foo/Config.in', 9)
    db.add_symbol_usage_in_legacy('LINGMO_bar', 'Config.in.legacy', 9)
    db.add_symbol_virtual('LINGMO_foo', 'foo/Config.in', 7)
    assert str(db) == str({
        'LINGMO_foo': {
            'part of a choice': {'foo/Config.in': [7]},
            'definition': {'foo/Config.in': [7]},
            'normal usage': {'foo/Config.in': [9]},
            'virtual': {'foo/Config.in': [7]}},
        'LINGMO_bar': {
            'definition': {'bar/Config.in': [700]},
            'possible config helper': {'bar/Config.in': [700]},
            'legacy usage': {'Config.in.legacy': [7001]},
            'selected': {'Config.in.legacy': [7001]},
            'usage inside legacy': {'Config.in.legacy': [9]}},
        'LINGMO_baz': {
            'legacy definition': {'Config.in.legacy': [7000]},
            'legacy note': {'Config.in.legacy': [7001]}},
        })


def test_get_symbols_with_pattern():
    db = m.DB()
    db.add_symbol_definition('LINGMO_foo', 'foo/Config.in', 7)
    db.add_symbol_usage('LINGMO_foo', 'foo/Config.in', 9)
    db.add_symbol_definition('LINGMO_bar', 'bar/Config.in', 5)
    assert str(db) == str({
        'LINGMO_foo': {'definition': {'foo/Config.in': [7]}, 'normal usage': {'foo/Config.in': [9]}},
        'LINGMO_bar': {'definition': {'bar/Config.in': [5]}},
        })
    symbols = db.get_symbols_with_pattern('foo')
    assert str(symbols) == str({
        'LINGMO_foo': {'definition': {'foo/Config.in': [7]}, 'normal usage': {'foo/Config.in': [9]}},
        })
    symbols = db.get_symbols_with_pattern('FOO')
    assert str(symbols) == str({
        })
    symbols = db.get_symbols_with_pattern('foo|FOO')
    assert str(symbols) == str({
        'LINGMO_foo': {'definition': {'foo/Config.in': [7]}, 'normal usage': {'foo/Config.in': [9]}},
        })
    symbols = db.get_symbols_with_pattern('^foo')
    assert str(symbols) == str({
        })
    symbols = db.get_symbols_with_pattern('foo|bar')
    assert str(symbols) == str({
        'LINGMO_foo': {'definition': {'foo/Config.in': [7]}, 'normal usage': {'foo/Config.in': [9]}},
        'LINGMO_bar': {'definition': {'bar/Config.in': [5]}},
        })


def test_get_warnings_for_choices_selected():
    db = m.DB()
    db.add_symbol_choice('LINGMO_foo', 'foo/Config.in', 1)
    db.add_symbol_choice('LINGMO_bar', 'bar/Config.in', 1)
    db.add_symbol_select('LINGMO_foo', 'bar/Config.in', 2)
    assert str(db) == str({
        'LINGMO_foo': {'part of a choice': {'foo/Config.in': [1]}, 'selected': {'bar/Config.in': [2]}},
        'LINGMO_bar': {'part of a choice': {'bar/Config.in': [1]}},
        })
    warnings = db.get_warnings_for_choices_selected()
    assert warnings == [
        ('bar/Config.in', 2, 'LINGMO_foo is part of a "choice" and should not be "select"ed'),
        ]


def test_get_warnings_for_legacy_symbols_being_used():
    db = m.DB()
    db.add_symbol_legacy_definition('LINGMO_foo', 'Config.in.legacy', 1)
    db.add_symbol_usage('LINGMO_foo', 'bar/Config.in', 2)
    db.add_symbol_legacy_definition('LINGMO_bar', 'Config.in.legacy', 10)
    db.add_symbol_usage_in_legacy('LINGMO_bar', 'Config.in.legacy', 11)
    assert str(db) == str({
        'LINGMO_foo': {'legacy definition': {'Config.in.legacy': [1]}, 'normal usage': {'bar/Config.in': [2]}},
        'LINGMO_bar': {'legacy definition': {'Config.in.legacy': [10]}, 'usage inside legacy': {'Config.in.legacy': [11]}},
        })
    warnings = db.get_warnings_for_legacy_symbols_being_used()
    assert warnings == [
        ('bar/Config.in', 2, 'LINGMO_foo is a legacy symbol and should not be referenced'),
        ]


def test_get_warnings_for_legacy_symbols_being_defined():
    db = m.DB()
    db.add_symbol_legacy_definition('LINGMO_foo', 'Config.in.legacy', 1)
    db.add_symbol_legacy_definition('LINGMO_bar', 'Config.in.legacy', 10)
    db.add_symbol_definition('LINGMO_foo', 'foo/Config.in', 7)
    db.add_symbol_definition('LINGMO_foo', 'foo/Config.in', 8)
    assert str(db) == str({
        'LINGMO_foo': {'legacy definition': {'Config.in.legacy': [1]}, 'definition': {'foo/Config.in': [7, 8]}},
        'LINGMO_bar': {'legacy definition': {'Config.in.legacy': [10]}},
        })
    warnings = db.get_warnings_for_legacy_symbols_being_defined()
    assert warnings == [
        ('foo/Config.in', 7, 'LINGMO_foo is a legacy symbol and should not be redefined'),
        ('foo/Config.in', 8, 'LINGMO_foo is a legacy symbol and should not be redefined'),
        ]


def test_get_warnings_for_symbols_without_definition():
    db = m.DB()
    db.add_symbol_definition('LINGMO_foo', 'foo/Config.in', 7)
    db.add_symbol_legacy_definition('LINGMO_bar', 'Config.in.legacy', 10)
    db.add_symbol_virtual('LINGMO_baz', 'baz/Config.in', 7)
    db.add_symbol_usage('LINGMO_foo', 'file', 1)
    db.add_symbol_usage('LINGMO_bar', 'file', 1)
    db.add_symbol_usage('LINGMO_baz', 'file', 1)
    db.add_symbol_usage('LINGMO_undef1', 'file', 1)
    db.add_symbol_legacy_usage('LINGMO_undef2', 'file', 2)
    db.add_symbol_usage_in_legacy('LINGMO_undef3', 'file', 3)
    db.add_symbol_usage('LINGMO_undef3', 'another', 1)
    db.add_symbol_legacy_usage('LINGMO_undef3', 'another', 2)
    db.add_symbol_usage('LINGMO_PACKAGE_HOST_undef', 'file', 1)
    db.add_symbol_usage('LINGMO_PACKAGE_HAS_HOST_undef', 'file', 1)
    db.add_symbol_usage('LINGMO_TARGET_ROOTFS_undef_XZ', 'file', 1)
    db.add_symbol_usage('LINGMO_GRAPH_ALT', 'file', 1)
    db.add_symbol_usage('LINGMO_EXTERNAL', 'file', 1)
    db.add_symbol_usage('LINGMO_TARGET_BAREBOX_AUX_BAREBOXENV', 'file', 1)
    db.add_symbol_usage('LINGMO_PACKAGE_HAS_TOOLCHAIN_BUILDROOT', 'file', 1)
    assert str(db) == str({
        'LINGMO_foo': {'definition': {'foo/Config.in': [7]}, 'normal usage': {'file': [1]}},
        'LINGMO_bar': {'legacy definition': {'Config.in.legacy': [10]}, 'normal usage': {'file': [1]}},
        'LINGMO_baz': {'virtual': {'baz/Config.in': [7]}, 'normal usage': {'file': [1]}},
        'LINGMO_undef1': {'normal usage': {'file': [1]}},
        'LINGMO_undef2': {'legacy usage': {'file': [2]}},
        'LINGMO_undef3': {'usage inside legacy': {'file': [3]}, 'normal usage': {'another': [1]}, 'legacy usage': {'another': [2]}},
        'LINGMO_PACKAGE_HOST_undef': {'normal usage': {'file': [1]}},
        'LINGMO_PACKAGE_HAS_HOST_undef': {'normal usage': {'file': [1]}},
        'LINGMO_TARGET_ROOTFS_undef_XZ': {'normal usage': {'file': [1]}},
        'LINGMO_GRAPH_ALT': {'normal usage': {'file': [1]}},
        'LINGMO_EXTERNAL': {'normal usage': {'file': [1]}},
        'LINGMO_TARGET_BAREBOX_AUX_BAREBOXENV': {'normal usage': {'file': [1]}},
        'LINGMO_PACKAGE_HAS_TOOLCHAIN_BUILDROOT': {'normal usage': {'file': [1]}},
        })
    warnings = db.get_warnings_for_symbols_without_definition()
    assert warnings == [
        ('file', 1, 'LINGMO_undef1 referenced but not defined'),
        ('file', 2, 'LINGMO_undef2 referenced but not defined'),
        ('another', 1, 'LINGMO_undef3 referenced but not defined'),
        ('another', 2, 'LINGMO_undef3 referenced but not defined'),
        ('file', 3, 'LINGMO_undef3 referenced but not defined'),
        ]


def test_get_warnings_for_symbols_without_usage():
    db = m.DB()
    db.add_symbol_definition('LINGMO_a', 'a/Config.in', 1)
    db.add_symbol_definition('LINGMO_a', 'a/Config.in', 2)
    db.add_symbol_usage('LINGMO_a', 'file', 1)
    db.add_symbol_usage('LINGMO_a', 'file', 2)
    db.add_symbol_definition('LINGMO_b', 'b/Config.in', 2)
    db.add_symbol_usage_in_legacy('LINGMO_b', 'file', 1)
    db.add_symbol_definition('LINGMO_c', 'c/Config.in', 2)
    db.add_symbol_legacy_usage('LINGMO_c', 'file', 1)
    db.add_symbol_definition('LINGMO_USE_CCACHE', 'file', 1)
    db.add_symbol_definition('LINGMO_PACKAGE_SKELETON', 'file', 1)
    db.add_symbol_definition('LINGMO_d', 'd/Config.in', 2)
    db.add_symbol_helper('LINGMO_d', 'd/Config.in', 2)
    db.add_symbol_definition('LINGMO_e', 'e/Config.in', 2)
    db.add_symbol_choice('LINGMO_e', 'e/Config.in', 2)
    db.add_symbol_definition('LINGMO_f', 'f/Config.in', 2)
    db.add_symbol_definition('LINGMO_g', 'g/Config.in', 2)
    db.add_symbol_definition('LINGMO_g', 'g/Config.in', 3)
    db.add_symbol_legacy_definition('LINGMO_h', 'Config.in.legacy', 1)
    db.add_symbol_usage('LINGMO_h', 'file', 2)
    db.add_symbol_usage('LINGMO_h', 'file', 3)
    db.add_symbol_legacy_definition('LINGMO_i', 'Config.in.legacy', 2)
    db.add_symbol_usage_in_legacy('LINGMO_i', 'file', 2)
    db.add_symbol_legacy_definition('LINGMO_j', 'Config.in.legacy', 2)
    db.add_symbol_legacy_usage('LINGMO_j', 'file', 2)
    db.add_symbol_legacy_definition('LINGMO_k', 'Config.in.legacy', 2)
    db.add_symbol_usage('LINGMO_k', 'file', 5)
    db.add_symbol_usage_in_legacy('LINGMO_k', 'file', 6)
    db.add_symbol_legacy_usage('LINGMO_k', 'file', 7)
    db.add_symbol_legacy_definition('LINGMO_l', 'Config.in.legacy', 2)
    assert str(db) == str({
        'LINGMO_a': {'definition': {'a/Config.in': [1, 2]}, 'normal usage': {'file': [1, 2]}},
        'LINGMO_b': {'definition': {'b/Config.in': [2]}, 'usage inside legacy': {'file': [1]}},
        'LINGMO_c': {'definition': {'c/Config.in': [2]}, 'legacy usage': {'file': [1]}},
        'LINGMO_USE_CCACHE': {'definition': {'file': [1]}},
        'LINGMO_PACKAGE_SKELETON': {'definition': {'file': [1]}},
        'LINGMO_d': {'definition': {'d/Config.in': [2]}, 'possible config helper': {'d/Config.in': [2]}},
        'LINGMO_e': {'definition': {'e/Config.in': [2]}, 'part of a choice': {'e/Config.in': [2]}},
        'LINGMO_f': {'definition': {'f/Config.in': [2]}},
        'LINGMO_g': {'definition': {'g/Config.in': [2, 3]}},
        'LINGMO_h': {'legacy definition': {'Config.in.legacy': [1]}, 'normal usage': {'file': [2, 3]}},
        'LINGMO_i': {'legacy definition': {'Config.in.legacy': [2]}, 'usage inside legacy': {'file': [2]}},
        'LINGMO_j': {'legacy definition': {'Config.in.legacy': [2]}, 'legacy usage': {'file': [2]}},
        'LINGMO_k': {
            'legacy definition': {'Config.in.legacy': [2]},
            'normal usage': {'file': [5]},
            'usage inside legacy': {'file': [6]},
            'legacy usage': {'file': [7]}},
        'LINGMO_l': {'legacy definition': {'Config.in.legacy': [2]}},
        })
    warnings = db.get_warnings_for_symbols_without_usage()
    assert warnings == [
        ('f/Config.in', 2, 'LINGMO_f defined but not referenced'),
        ('g/Config.in', 2, 'LINGMO_g defined but not referenced'),
        ('g/Config.in', 3, 'LINGMO_g defined but not referenced'),
        ('Config.in.legacy', 2, 'LINGMO_l defined but not referenced'),
        ]


def test_get_warnings_for_symbols_with_legacy_note_and_no_comment_on_usage():
    db = m.DB()
    db.add_symbol_legacy_note('LINGMO_foo', 'Config.in.legacy', 1)
    db.add_symbol_legacy_usage('LINGMO_foo', 'package/bar/Config.in', 2)
    db.add_symbol_legacy_note('LINGMO_baz', 'Config.in.legacy', 7001)
    db.add_symbol_usage('LINGMO_baz', 'package/foo/Config.in', 1)
    assert str(db) == str({
        'LINGMO_foo': {'legacy note': {'Config.in.legacy': [1]}, 'legacy usage': {'package/bar/Config.in': [2]}},
        'LINGMO_baz': {'legacy note': {'Config.in.legacy': [7001]}, 'normal usage': {'package/foo/Config.in': [1]}},
        })
    warnings = db.get_warnings_for_symbols_with_legacy_note_and_no_comment_on_usage()
    assert warnings == [
        ('package/foo/Config.in', 1, 'LINGMO_baz missing "# legacy"'),
        ]


def test_get_warnings_for_symbols_with_legacy_note_and_no_usage():
    db = m.DB()
    db.add_symbol_legacy_note('LINGMO_foo', 'Config.in.legacy', 1)
    db.add_symbol_legacy_usage('LINGMO_foo', 'package/bar/Config.in', 2)
    db.add_symbol_legacy_note('LINGMO_bar', 'Config.in.legacy', 1)
    db.add_symbol_usage_in_legacy('LINGMO_bar', 'Config.in.legacy', 7001)
    db.add_symbol_legacy_note('LINGMO_baz', 'Config.in.legacy', 7001)
    db.add_symbol_legacy_note('LINGMO_no_comment', 'Config.in.legacy', 1)
    db.add_symbol_usage('LINGMO_no_comment', 'package/bar/Config.in', 2)
    assert str(db) == str({
        'LINGMO_foo': {'legacy note': {'Config.in.legacy': [1]}, 'legacy usage': {'package/bar/Config.in': [2]}},
        'LINGMO_bar': {'legacy note': {'Config.in.legacy': [1]}, 'usage inside legacy': {'Config.in.legacy': [7001]}},
        'LINGMO_baz': {'legacy note': {'Config.in.legacy': [7001]}},
        'LINGMO_no_comment': {'legacy note': {'Config.in.legacy': [1]}, 'normal usage': {'package/bar/Config.in': [2]}},
        })
    warnings = db.get_warnings_for_symbols_with_legacy_note_and_no_usage()
    assert warnings == [
        ('Config.in.legacy', 1, 'LINGMO_bar not referenced but has a comment stating it is'),
        ('Config.in.legacy', 7001, 'LINGMO_baz not referenced but has a comment stating it is'),
        ]
