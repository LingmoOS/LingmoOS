<?php
/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

require_once('../src/server/shared/utils.php');

class UtilsTest extends PHPUnit\Framework\TestCase
{
    public static function isValidIdentifier_data()
    {
        return [
            'empty' => [ '', false ],
            'number' => [ 42, false ],
            'numstring' => [ '42', false ],
            'leading number' => [ '1foo', false ],
            'alpha only' => [ 'foo', true ],
            'leading space' => [ ' foo', false ],
            'middle space' => [ 'f o o', false ],
            'trailing space' => [ 'foo ', false ],
            'valid' => [ 'foo42', true ],
            'leading underscore' => [ '_foo', true ],
            'underscore' => [ 'f_o_o', true ],
            'dots and dashes' => [ 'org.kde.unit-test', true ],
            'control' => [ "fo\no", false ]
        ];
    }

    /** @dataProvider isValidIdentifier_data */
    public function testIsValidIdentifier($str, $result)
    {
        $this->assertEquals($result, Utils::isValidIdentifier($str));
    }

    public static function normalize_data()
    {
        return [
            'empty' => [ '', '' ],
            'normal' => [ 'foo', 'foo' ],
            'dot' => [ 'org.kde.foo', 'org_kde_foo' ]
        ];
    }

    /** @dataProvider normalize_data */
    public function testNormalize($input, $output)
    {
        $this->assertEquals($output, Utils::normalizeString($input));
    }

    public static function primaryKeyColumn_data()
    {
        return [
            'sqlite' => [ 'sqlite', 'id', 'id INTEGER PRIMARY KEY AUTOINCREMENT' ],
            'pgsql' => [ 'pgsql', 'id', 'id SERIAL PRIMARY KEY' ],
            'mysql' => [ 'mysql', 'id', 'id INTEGER PRIMARY KEY AUTO_INCREMENT' ]
        ];
    }

    /** @dataProvider primaryKeyColumn_data */
    public function primaryKeyColumn($driver, $name, $output)
    {
        $this->assertEquals($output, Utils::primaryKeyColumnDeclaration($driver, $name));
    }

    public static function sqlStringType_data()
    {
        return [
            'sqlite' => [ 'sqlite', 'VARCHAR' ],
            'mysql' => [ 'mysql', 'VARCHAR(255)' ],
            'pgsql' => [ 'pgsql', 'VARCHAR' ],
        ];
    }

    /** @dataProvider sqlStringType_data */
    public function testSqlStringType($driver, $output)
    {
        $this->assertEquals($output, Utils::sqlStringType($driver));
    }
}
