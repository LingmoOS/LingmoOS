<?php
/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

require_once('datastoretesthelper.php');
require_once('../src/server/shared/product.php');

class ProductTest extends PHPUnit\Framework\TestCase
{
    private static $db;

    public static function setUpBeforeClass(): void
    {
        self::$db = DatastoreTestHelper::setup();
    }

    public static function tableName_data()
    {
        return [
            'normal' => [ 'foo', 'pd_foo' ],
            'dot' => [ 'org.kde.foo', 'pd_org_kde_foo' ]
        ];
    }

    /** @dataProvider tableName_data */
    public function testTableName($input, $output)
    {
        $p = new Product;
        $p->name = $input;
        $this->assertEquals($output, $p->dataTableName());
    }

    public function testFromJson()
    {
        $p = Product::fromJson('{
            "name": "org.kde.product",
            "schema": [
                {
                    "name": "entry1",
                    "type": "scalar",
                    "elements" : [
                        { "name": "element1", "type": "int" },
                        { "name": "element2", "type": "bool" }
                    ]
                }
            ],
            "aggregation": [ { "type": "category", "name": "n1", "elements": [
                { "type": "value", "schemaEntry": "entry1", "schemaEntryElement": "element1" }
            ] } ]
        }');
        $this->assertEquals($p->name, 'org.kde.product');
        $this->assertEquals(1, count($p->schema));
        $this->assertEquals('pd_org_kde_product', $p->dataTableName());
        $entry = $p->schema[0];
        $this->assertEquals($entry->name, 'entry1');
        $this->assertEquals($entry->type, 'scalar');

        $this->assertEquals(2, count($entry->elements));
        $elem = $entry->elements[1];
        $this->assertInstanceOf(SchemaEntryElement::class, $elem);
        $this->assertEquals('element2', $elem->name);
        $this->assertEquals('bool', $elem->type);

        $this->assertCount(1, $p->aggregation);
    }

    public function testToJson()
    {
        $p = new Product();
        $p->name = 'org.kde.product';
        $entry = new SchemaEntry($p);
        $entry->name = 'entry1';
        $entry->type = 'list';
        $elem = new SchemaEntryElement($entry);
        $elem->name = 'element3';
        $elem->type = 'number';
        array_push($entry->elements, $elem);
        array_push($p->schema, $entry);
        $a = new Aggregation;
        $a->type = 'numeric';
        $a->name = 'n1';
        $a->elements = json_decode('[{ "type": "size", "schemaEntry": "entry1" }]');
        array_push($p->aggregation, $a);

        $this->assertJsonStringEqualsJsonString(json_encode($p), '{
            "name": "org.kde.product",
            "schema": [
                {
                    "name": "entry1",
                    "type": "list",
                    "elements" : [
                        { "name": "element3", "type": "number" }
                    ]
                }
            ],
            "aggregation": [ { "type": "numeric", "name": "n1", "elements": [
                { "type": "size", "schemaEntry": "entry1" }
            ] } ]
        }');
    }

    public function testAllProducts()
    {
        $ps = Product::allProducts(self::$db);
        $this->assertEquals(1, count($ps));
        $p = $ps[0];
        $this->assertInstanceOf(Product::class, $p);
        $this->assertEquals('org.kde.UnitTest', $p->name);
        $this->assertEquals(2, count($p->schema));

        $entry1 = $p->schema[0];
        $entry2 = $p->schema[1];
        if ($entry1->name > $entry2->name) {
            $entry1 = $p->schema[1];
            $entry2 = $p->schema[0];
        }

        $this->assertInstanceOf(SchemaEntry::class, $entry1);
        $this->assertEquals($entry1->name, 'entry1');
        $this->assertEquals($entry1->type, 'scalar');
        $this->assertEquals(2, count($entry1->elements));
        $elem = $entry1->elements[1];
        $this->assertInstanceOf(SchemaEntryElement::class, $elem);
        $this->assertEquals('element12', $elem->name);
        $this->assertEquals('bool', $elem->type);

        $this->assertInstanceOf(SchemaEntry::class, $entry2);
        $this->assertEquals($entry2->name, 'entry2');
        $this->assertEquals($entry2->type, 'list');

        $this->assertCount(2, $p->aggregation);
    }

    public function testProductByName()
    {
        $p = Product::productByName(self::$db, 'I don\'t exist');
        $this->assertNull($p);

        $p = Product::productByName(self::$db, 'org.kde.UnitTest');
        $this->assertInstanceOf(Product::class, $p);
        $this->assertEquals('org.kde.UnitTest', $p->name);
        $this->assertEquals(2, count($p->schema));

        $entry1 = $p->schema[0];
        $entry2 = $p->schema[1];
        if ($entry1->name > $entry2->name) {
            $entry1 = $p->schema[1];
            $entry2 = $p->schema[0];
        }

        $this->assertInstanceOf(SchemaEntry::class, $entry1);
        $this->assertEquals($entry1->name, 'entry1');
        $this->assertEquals($entry1->type, 'scalar');
        $elem = $entry1->elements[1];
        $this->assertInstanceOf(SchemaEntryElement::class, $elem);
        $this->assertEquals('element12', $elem->name);
        $this->assertEquals('bool', $elem->type);

        $this->assertInstanceOf(SchemaEntry::class, $entry2);
        $this->assertEquals($entry2->name, 'entry2');
        $this->assertEquals($entry2->type, 'list');
    }

    public function testProductInsert()
    {
        $json = '{
            "name": "org.kde.NewProduct",
            "schema": [
                {
                    "name": "entryA",
                    "type": "scalar",
                    "elements" : [
                        { "name": "elementA1", "type": "int" },
                        { "name": "elementA2", "type": "bool" }
                    ]
                },
                {
                    "name": "entryB",
                    "type": "map",
                    "elements" : [
                        { "name": "elementB1", "type": "number" },
                        { "name": "elementB2", "type": "string" }
                    ]
                }
            ],
            "aggregation": [
                {
                    "type": "category",
                    "name": "n1",
                    "elements": [
                        { "type": "value", "schemaEntry": "entryA", "schemaEntryElement": "elementA1" }
                    ]
                },
                {
                    "type": "ratio_set",
                    "name": "n2",
                    "elements": [
                        { "type": "value", "schemaEntry": "entryB", "schemaEntryElement": "elementB1" }
                    ]
                }
            ]
        }';

        $p = Product::fromJson($json);
        $p->insert(self::$db);

        $p = Product::productByName(self::$db, 'org.kde.NewProduct');
        $p->aggregation = Aggregation::aggregationsForProduct(self::$db, $p);
        $this->assertJsonStringEqualsJsonString($json, json_encode($p));
    }

    public function testProductUpdate()
    {
        $jsonOld = '{
            "name": "org.kde.ProductToUpdate",
            "schema": [
                {
                    "name": "entryA",
                    "type": "scalar",
                    "elements" : [
                        { "name": "elementA1", "type": "int" },
                        { "name": "elementA2", "type": "bool" }
                    ]
                },
                {
                    "name": "entryB",
                    "type": "list",
                    "elements" : [
                        { "name": "elementB1", "type": "number" },
                        { "name": "elementB2", "type": "string" }
                    ]
                }
            ],
            "aggregation": [
                {
                    "type": "ratio_set",
                    "name": "n2",
                    "elements": [
                        { "type": "value", "schemaEntry": "entryB", "schemaEntryElement": "elementB1" }
                    ]
                }
            ]
        }';

        $p = Product::fromJson($jsonOld);
        $p->insert(self::$db);

        $p = Product::productByName(self::$db, 'org.kde.ProductToUpdate');
        $p->aggregation = Aggregation::aggregationsForProduct(self::$db, $p);
        $this->assertJsonStringEqualsJsonString($jsonOld, json_encode($p));

        $jsonNew = '{
            "name": "org.kde.ProductToUpdate",
            "schema": [
                {
                    "name": "entryA",
                    "type": "scalar",
                    "elements" : [
                        { "name": "elementA1", "type": "int" }
                    ]
                },
                {
                    "name": "entryC",
                    "type": "map",
                    "elements" : [
                        { "name": "elementC1", "type": "number" },
                        { "name": "elementC2", "type": "number" }
                    ]
                }
            ],
            "aggregation": [
                {
                    "type": "category",
                    "name": "n1",
                    "elements": [
                        { "type": "value", "schemaEntry": "entryA", "schemaEntryElement": "elementA1" }
                    ]
                },
                {
                    "type": "xy",
                    "name": "n3",
                    "elements": [
                        { "type": "value", "schemaEntry": "entryC", "schemaEntryElement": "elementC1" },
                        { "type": "value", "schemaEntry": "entryC", "schemaEntryElement": "elementC2" }
                    ]
                }
            ]
        }';
        $newP = Product::fromJson($jsonNew);
        $p->update(self::$db, $newP);

        $p = Product::productByName(self::$db, 'org.kde.ProductToUpdate');
        $p->aggregation = Aggregation::aggregationsForProduct(self::$db, $p);
        $this->assertJsonStringEqualsJsonString($jsonNew, json_encode($p));
    }

    public function testProductDelete()
    {
        $json = '{
            "name": "org.kde.DeletedProduct",
            "schema": [
                {
                    "name": "entry1",
                    "type": "list",
                    "elements" : [
                        { "name": "element3", "type": "number" }
                    ]
                }
            ],
            "aggregation": [
                {
                    "type": "numeric",
                    "name": "n4",
                    "elements": [
                        { "type": "size", "schemaEntry": "entry1" }
                    ]
                }
            ]
        }';

        $p = Product::fromJson($json);
        $p->insert(self::$db);
        $p = Product::productByName(self::$db, 'org.kde.DeletedProduct');
        $p->aggregation = Aggregation::aggregationsForProduct(self::$db, $p);
        $this->assertJsonStringEqualsJsonString($json, json_encode($p));

        $p->delete(self::$db);
        $p = Product::productByName(self::$db, 'org.kde.DeletedProduct');
        $this->assertNull($p);
    }

    public static function invalidInput_data()
    {
        return [
            'empty' => [ '{}' ],
            'empty name' => [ '{ "name": "", "schema": [] }' ],
            'invalid name' => [ '{ "name": "123", "schema": [] }' ],
            'non-string name' => [ '{ "name": 123, "schema": [] }' ]
        ];
    }

    /**
     * @dataProvider invalidInput_data
     */
    public function testInvalidInput($json)
    {
        $this->expectException(RESTException::class);
        $this->expectExceptionCode(400);
        $p = Product::fromJson($json);
    }
}

