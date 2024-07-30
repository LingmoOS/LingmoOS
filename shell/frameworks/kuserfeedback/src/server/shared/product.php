<?php
/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

require_once('aggregation.php');
require_once('schemaentry.php');
require_once('utils.php');

/** Represents a product and its schema. */
class Product
{
    public $name;
    public $schema = array();
    public $aggregation = array();

    private $productId = -1;

    /** Name of the primary data table for this product, ie.
     *  the one containing all scalar data.
     */
    public function dataTableName()
    {
        $tableName = 'pd_' . Utils::normalizeString($this->name);
        return strtolower($tableName);
    }

    /** Returns the numeric database id of this product for use in queries. */
    public function id()
    {
        return $this->productId;
    }

    /** Retrieve all products from storage. */
    public static function allProducts(Datastore $db)
    {
        $products = array();
        $stmt = $db->prepare('SELECT col_id, col_name FROM tbl_product');
        $db->execute($stmt);
        foreach ($stmt as $row) {
            $p = new Product();
            $p->productId = intval($row['col_id']);
            $p->name = strval($row['col_name']);
            $p->schema = SchemaEntry::loadSchema($db, $p);
            $p->aggregation = Aggregation::aggregationsForProduct($db, $p);
            array_push($products, $p);
        }
        return $products;
    }

    /** Retrieve a specific product by name from storage. */
    public static function productByName(Datastore $db, $name)
    {
        if (!is_string($name) || strlen($name) <= 0)
            throw new RESTException('Invalid product name.', 400);

        $stmt = $db->prepare('SELECT col_id, col_name FROM tbl_product WHERE col_name = :name');
        $stmt->bindValue(':name', strval($name), PDO::PARAM_STR);
        $db->execute($stmt);
        foreach ($stmt as $row) {
            $p = new Product();
            $p->productId = intval($row['col_id']);
            $p->name = strval($row['col_name']);
            $p->schema = SchemaEntry::loadSchema($db, $p);
            return $p;
        }
        return null;
    }

    /** Insert new product into database. */
    public function insert(Datastore $db)
    {
        // create product entry
        $stmt = $db->prepare('INSERT INTO tbl_product (col_name) VALUES (:name)');
        $stmt->bindValue(':name', $this->name, PDO::PARAM_STR);
        $db->execute($stmt);
        $this->productId = $db->pdoHandle()->lastInsertId();

        // create data tables;
        $stmt = $db->prepare('CREATE TABLE ' . $this->dataTableName() . ' (' .
            Utils::primaryKeyColumnDeclaration($db->driver(), 'col_id') .
            ', col_timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP)
        ');
        $db->execute($stmt);

        // create schema entries
        foreach ($this->schema as $entry)
            $entry->insert($db, $this->productId);

        // store aggregation settings
        Aggregation::update($db, $this, $this->aggregation);
    }

    /** Update an existing product in the database to match @p $newProduct. */
    public function update(Datastore $db, Product $newProduct)
    {
        $oldSchema = array();
        foreach ($this->schema as $oldEntry)
            $oldSchema[$oldEntry->name] = $oldEntry;

        foreach ($newProduct->schema as $newEntry) {
            if (array_key_exists($newEntry->name, $oldSchema)) {
                // update
                $oldEntry = $oldSchema[$newEntry->name];
                $oldEntry->update($db, $newEntry);
            } else {
                // insert
                $newEntry->insert($db, $this->productId);
            }
            unset($oldSchema[$newEntry->name]);
        }

        // delete whatever is left
        foreach($oldSchema as $entry)
            $entry->delete($db, $this->productId);

        // store aggregation settings
        Aggregation::update($db, $this, $newProduct->aggregation);
    }

    /** Delete an existing product in the database. */
    public function delete(Datastore $db)
    {
        // delete aggregation settings
        Aggregation::delete($db, $this);

        // delete schema entries
        foreach ($this->schema as $entry)
            $entry->delete($db, $this->productId);

        // delete data tables
        $stmt = $db->prepare('DROP TABLE ' . $this->dataTableName() . ($db->driver() == 'sqlite' ? '' : ' CASCADE'));
        $db->execute($stmt);

        // delete product
        $stmt = $db->prepare('DELETE FROM tbl_product WHERE col_id = :id');
        $stmt->bindValue(':id', $this->productId, PDO::PARAM_INT);
        $db->execute($stmt);
    }

    /** Create one Product instance based on JSON input and verifies it is valid. */
    public static function fromJson($jsonString)
    {
        $jsonObj = json_decode($jsonString);
        if (!property_exists($jsonObj, 'name'))
            throw new RESTException('No product name specified.', 400);

        $p = new Product();
        $p->name = $jsonObj->name;
        $p->schema = SchemaEntry::fromJson($jsonObj->schema, $p);
        if (property_exists($jsonObj, 'aggregation'))
            $p->aggregation = Aggregation::fromJson($jsonObj->aggregation);

        // verify
        if (strlen($p->name) <= 0 || !is_string($p->name))
            throw new RESTException('Empty product name.', 400);
        if (!ctype_alpha($p->name[0]))
            throw new RESTException("Invalid product name, must start with a letter.", 400);

        return $p;
    }
}

?>
