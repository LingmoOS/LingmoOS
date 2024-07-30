<?php
/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

require_once('../src/server/shared/product.php');
require_once('../src/server/shared/schemaentry.php');

class SchemaEntryTest extends PHPUnit\Framework\TestCase
{
    public function dataTableName_data()
    {
        return [
            'normal' => [ 'foo', 'pd2_org_kde_testproduct__foo' ],
            'dot' => [ 'my.value', 'pd2_org_kde_testproduct__my_value' ],
            'uppercase' => [ 'UPPER', 'pd2_org_kde_testproduct__upper' ]
        ];
    }

    /** @dataProvider dataTableName_data */
    public function testDataTableName($input, $output)
    {
        $p = new Product;
        $p->name = 'org.kde.TestProduct';
        $se = new SchemaEntry($p);
        $se->name = $input;
        $this->assertEquals($output, $se->dataTableName());
    }

    public function invalidJson_data()
    {
        return [
            'empty' => [ '{}' ],
            'empty name' => [ '{ "name": "", "type": "scalar", "aggregationType": "none" }' ],
            'empty type' => [ '{ "name": "foo", "type": "", "aggregationType": "none" }' ],
            'invalid type' => [ '{ "name": "foo", "type": "bla", "aggregationType": "none" }' ],
            'invalid name' => [ '{ "name": " foo ", "type": "scalar", "aggregationType": "none" }' ],
            'invalid name 2' => [ '{ "name": "1foo ", "type": "scalar", "aggregationType": "none" }' ]
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
        $se = SchemaEntry::fromJson(json_decode('[ ' . $input . ' ]'), $p);
    }
}

