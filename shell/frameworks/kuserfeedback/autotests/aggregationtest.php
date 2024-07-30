<?php
/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

require_once('datastoretesthelper.php');
require_once('../src/server/shared/aggregation.php');

class AggregationTest extends PHPUnit\Framework\TestCase
{
    private static $db;

    public static function setUpBeforeClass(): void
    {
        self::$db = DatastoreTestHelper::setup();
    }

    public function testFromJson()
    {
        $json = '[]';
        $aggrs = Aggregation::fromJson(json_decode($json));
        $this->assertCount(0, $aggrs);

        $json = '[
            { "type": "category", "name": "category elem11", "elements": [ { "type": "value", "schemaEntry": "entry1", "schemaEntryElement": "elem11" } ] },
            { "type": "numeric", "name": "size: entry2", "elements": [ { "type": "size", "schemaEntry": "entry2" } ] }
        ]';
        $aggrs = Aggregation::fromJson(json_decode($json));
        $this->assertCount(2, $aggrs);

        $a = $aggrs[0];
        $this->assertInstanceOf(Aggregation::class, $a);
        $this->assertEquals('category', $a->type);
        $this->assertEquals('category elem11', $a->name);
        $this->assertCount(1, $a->elements);
        $a = $aggrs[1];
        $this->assertInstanceOf(Aggregation::class, $a);
        $this->assertEquals('numeric', $a->type);
        $this->assertEquals('size: entry2', $a->name);
        $this->assertCount(1, $a->elements);
    }

    public function testLoad()
    {
        $p = Product::productByName(self::$db, 'org.kde.UnitTest');
        $this->assertNotNull($p);

        $aggrs = Aggregation::aggregationsForProduct(self::$db, $p);
        $this->assertCount(2, $aggrs);

        $a = $aggrs[0];
        $this->assertInstanceOf(Aggregation::class, $a);
        $this->assertEquals('category', $a->type);
        $this->assertCount(1, $a->elements);

        $a = $aggrs[1];
        $this->assertInstanceOf(Aggregation::class, $a);
        $this->assertEquals('numeric', $a->type);
        $this->assertCount(1, $a->elements);
    }

    public function testWrite()
    {
        $p = Product::productByName(self::$db, 'org.kde.UnitTest');
        $this->assertNotNull($p);

        $a = new Aggregation;
        $a->type = 'xy';
        $a->name = 'n1';
        $a->elements = json_decode('[
            { "type": "value", "schemaEntry": "entry2", "schemaEntryElement": "element21" },
            { "type": "value", "schemaEntry": "entry2", "schemaEntryElement": "element22" }
        ]');
        Aggregation::update(self::$db, $p, array(0 => $a));

        $aggrs = Aggregation::aggregationsForProduct(self::$db, $p);
        $this->assertCount(1, $aggrs);

        $a = $aggrs[0];
        $this->assertInstanceOf(Aggregation::class, $a);
        $this->assertEquals('xy', $a->type);
        $this->assertEquals('n1', $a->name);
        $this->assertCount(2, $a->elements);
    }

    public function testDelete()
    {
        $p = Product::productByName(self::$db, 'org.kde.UnitTest');
        $this->assertNotNull($p);

        $aggrs = Aggregation::aggregationsForProduct(self::$db, $p);
        $this->assertNotEmpty($aggrs);

        Aggregation::delete(self::$db, $p);
        $aggrs = Aggregation::aggregationsForProduct(self::$db, $p);
        $this->assertEmpty($aggrs);
    }

    public static function invalidJson_data()
    {
        return [
            'nothing' => [ '' ],
            'object' => [ '{}' ],
            'array of non-objects' => [ '[1, 2, 3]' ],
            'missing type' => [ '[{ "elements": [] }]' ],
            'missing name' => [ '[{ "type": "category", "elements": [] }]' ]
        ];
    }

    /**
     * @dataProvider invalidJson_data
     */
    public function testInvalidJson($input)
    {
        $this->expectException(RESTException::class);
        $this->expectExceptionCode(400);
        $aggrs = Aggregation::fromJson(json_decode($input));
    }
}
