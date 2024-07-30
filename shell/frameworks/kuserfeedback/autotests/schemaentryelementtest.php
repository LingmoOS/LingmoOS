<?php
/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

require_once('../src/server/shared/product.php');
require_once('../src/server/shared/schemaentry.php');
require_once('../src/server/shared/schemaentryelement.php');

class SchemaEntryElementTest extends PHPUnit\Framework\TestCase
{
    public static function dataColumnName_data()
    {
        return [
            'normal' => [ 'foo', 'col_data_entry_foo' ],
            'dot' => [ 'my.value', 'col_data_entry_my_value' ]
        ];
    }

    /** @dataProvider dataColumnName_data */
    public function testDataTableName($input, $output)
    {
        $p = new Product;
        $p->name = 'org.kde.TestProduct';
        $se = new SchemaEntry($p);
        $se->name = 'entry';
        $see = new SchemaEntryElement($se);
        $see->name = $input;
        $this->assertEquals($output, $see->dataColumnName());
    }

    public static function invalidJson_data()
    {
        return [
            'empty' => [ '{}' ],
            'empty name' => [ '{ "name": "", "type": "string" }' ],
            'empty type' => [ '{ "name": "foo", "type": "" }' ],
            'invalid type' => [ '{ "name": "foo", "type": "bla" }' ],
            'invalid name' => [ '{ "name": " foo ", "type": "string" }' ],
            'invalid name 2' => [ '{ "name": "1foo ", "type": "string" }' ]
        ];
    }

    /**
     * @dataProvider invalidJson_data
     */
    public function testInvalidJson($input)
    {
        $this->expectException(RESTException::class);
        $this->expectExceptionCode(400);
        $p = new Product;
        $se = new SchemaEntry($p);
        SchemaEntryElement::fromJson(json_decode('[ ' . $input . ' ]'), $se);
    }
}
