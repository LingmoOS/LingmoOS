<?php
/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

require_once('datastoretesthelper.php');
require_once('../src/server/shared/product.php');
require_once('../src/server/shared/sample.php');

class SampleTest extends PHPUnit\Framework\TestCase
{
    private static $db;

    public static function setUpBeforeClass(): void
    {
        self::$db = DatastoreTestHelper::setup();
    }

    public function testListSampleInsert()
    {
        $p = Product::productByName(self::$db, 'org.kde.UnitTest');
        $this->assertNotNull($p);
        $p->name = 'org.kde.MyListProduct';
        $p->insert(self::$db); // HACK create a new product, so the data tables are created correctly

        $sample = '{
            "entry1": {
                "element11": "aString",
                "element12": true
            },
            "entry2": [
                { "element21": 14, "element22": 1.5 },
                { "element21": 16, "element22": 1.7 }
            ]
        }';

        Sample::insert(self::$db, $sample, $p);
        Sample::insert(self::$db, $sample, $p);

        Sample::echoDataAsJson(self::$db, $p);
        $data = json_decode($this->getActualOutput(), false);
        $this->assertTrue(is_array($data));
        $this->assertCount(2, $data);
        $d0 = $data[0];
        $this->assertObjectHasAttribute('timestamp', $d0);
        $this->assertObjectHasAttribute('entry1', $d0);
        $this->assertObjectHasAttribute('entry2', $d0);
        $d01 = $d0->{'entry2'};
        $this->assertCount(2, $d01);
    }

    public function testMapSampleInsert()
    {
        $p = Product::productByName(self::$db, 'org.kde.UnitTest');
        $this->assertNotNull($p);
        $p->name = 'org.kde.MyMapProduct';
        $p->schema[1]->type = SchemaEntry::MAP_TYPE;
        $p->insert(self::$db);

        $sample = '{
            "entry1": {
                "element11": "aString",
                "element12": true
            },
            "entry2": {
                "key1": { "element21": 14, "element22": 1.5 },
                "key2": { "element21": 16, "element22": 1.7 }
            }
        }';

        Sample::insert(self::$db, $sample, $p);

        Sample::echoDataAsJson(self::$db, $p);
        $data = json_decode($this->getActualOutput(), false);

        $this->assertTrue(is_array($data));
        $this->assertCount(1, $data);
        $d0 = $data[0];
        $this->assertObjectHasAttribute('timestamp', $d0);
        $this->assertObjectHasAttribute('entry1', $d0);
        $this->assertObjectHasAttribute('entry2', $d0);
        $d01 = $d0->{'entry2'};
        $this->assertObjectHasAttribute('key1', $d01);
        $this->assertObjectHasAttribute('key2', $d01);
    }

    public function testEmptyInsert()
    {
        $p = Product::productByName(self::$db, 'org.kde.UnitTest');
        $this->assertNotNull($p);
        $p->name = 'org.kde.MyEmptyProduct';
        $p->insert(self::$db);

        $sample = '{
            "someRandomStuff": "not part of the schema",
            "someOtherStuff": 42
        }';

        Sample::insert(self::$db, $sample, $p);
        Sample::echoDataAsJson(self::$db, $p);
        $data = json_decode($this->getActualOutput(), false);
        $this->assertTrue(is_array($data));
        $this->assertCount(1, $data);
        $d0 = $data[0];
        $this->assertObjectHasAttribute('timestamp', $d0);
        $this->assertObjectNotHasAttribute('entry1', $d0);
        $this->assertObjectNotHasAttribute('someRandomStuff', $d0);
    }

    public static function invalidInsert_data()
    {
        return [
            'empty' => [ '' ],
            'array' => [ '[]' ],
            'missing id' => [ '{ "timestamp": "2016-12-18 12:42:35" }' ],
            'missing timestamp' => [ '{ "id": 42 }' ]
        ];
    }

    /**
     * @dataProvider invalidInsert_data
     */
    public function testInvalidInsert($input)
    {
        $this->expectException(RESTException::class);
        $this->expectExceptionCode(400);
        $p = Product::productByName(self::$db, 'org.kde.UnitTest');
        $this->assertNotNull($p);

        Sample::insert(self::$db, $input, $p);
    }

    public function testImport()
    {
        $p = Product::productByName(self::$db, 'org.kde.UnitTest');
        $this->assertNotNull($p);
        $p->name = 'org.kde.MyCleanProduct';
        $p->insert(self::$db);

        $input = '[{
            "timestamp": "2016-12-18 12:42:35",
            "entry1": { "element11": "firstString" },
            "entry2": [ { "element21": 12 } ]
        }, {
            "timestamp": "2016-12-19 15:12:10",
            "entry1": { "element12": true },
            "entry2": [ { "element22": 1.3 } ]
        }]';
        Sample::import(self::$db, $input, $p);

        Sample::echoDataAsJson(self::$db, $p);
        $jsonOutput = json_decode($this->getActualOutput(), true);
        unset($jsonOutput[0]['id']);
        unset($jsonOutput[1]['id']);
        $this->assertJsonStringEqualsJsonString($input, json_encode($jsonOutput));
    }

    public static function invalidImport_data()
    {
        return [
            'nothing' => [ '' ],
            'empty' => [ '{}' ],
            'object' => [ '{ "id": 42, "timestamp": "2016-12-18 12:42:35" }' ],
            'missing timestamp' => [ '[{ "id": 42 }]' ],
            'id present' => [ '[{ "id": 42, "timestamp": "2016-12-18 12:42:35" }]' ]
        ];
    }

    /**
     * @dataProvider invalidImport_data
     */
    public function testInvalidImport($input)
    {
        $this->expectException(RESTException::class);
        $this->expectExceptionCode(400);
        $p = Product::productByName(self::$db, 'org.kde.UnitTest');
        $this->assertNotNull($p);

        Sample::import(self::$db, $input, $p);
    }
}
