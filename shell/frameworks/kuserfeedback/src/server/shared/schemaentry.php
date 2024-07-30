<?php
/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

require_once('schemaentryelement.php');
require_once('utils.php');

/** Represents a product schema entry. */
class SchemaEntry
{
    public $name;
    public $type;
    public $elements = array();

    private $entryId = -1;
    private $m_product = null;

    const SCALAR_TPYE = 'scalar';
    const LIST_TYPE = 'list';
    const MAP_TYPE = 'map';

    public function __construct(Product $product)
    {
        $this->m_product = &$product;
    }

    /** Checks if this is a schema entry. */
    public function isValid()
    {
        if ($this->type != self::SCALAR_TPYE && $this->type != self::LIST_TYPE && $this->type != self::MAP_TYPE)
            return false;
        if (!Utils::isValidIdentifier($this->name))
            return false;
        return true;
    }

    /** Checks if this is a scalar type, ie. samples go into the primary data table. */
    public function isScalar()
    {
        return $this->type === self::SCALAR_TPYE;
    }

    /** Returns the product this entry belongs to. */
    public function product()
    {
        return $this->m_product;
    }

    /** Load product schema from storage. */
    static public function loadSchema(Datastore $db, Product &$product)
    {
        $stmt = $db->prepare('SELECT
                tbl_schema.col_id, tbl_schema.col_name, tbl_schema.col_type, tbl_schema_element.col_name, tbl_schema_element.col_type
            FROM tbl_schema_element JOIN tbl_schema ON (tbl_schema.col_id = tbl_schema_element.col_schema_id)
            WHERE tbl_schema.col_product_id = :productId
            ORDER BY tbl_schema.col_id
        ');
        $stmt->bindValue(':productId', $product->id(), PDO::PARAM_INT);
        $db->execute($stmt);
        $schema = array();
        $entry = new SchemaEntry($product);
        foreach ($stmt as $row) {
            if ($entry->entryId != $row[0]) {
                if ($entry->isValid()) {
                    array_push($schema, $entry);
                    $entry = new SchemaEntry($product);
                }
                $entry->entryId = $row[0];
                $entry->name = $row[1];
                $entry->type = $row[2];
            }
            $elem = new SchemaEntryElement($entry);
            $elem->name = $row[3];
            $elem->type = $row[4];
            array_push($entry->elements, $elem);
        }
        if ($entry->isValid())
            array_push($schema, $entry);

        return $schema;
    }

    /** Insert a new schema entry into storage. */
    public function insert(Datastore $db, $productId)
    {
        $stmt = $db->prepare('INSERT INTO
            tbl_schema (col_product_id, col_name, col_type)
            VALUES (:productId, :name, :type)
        ');
        $stmt->bindValue(':productId', $productId, PDO::PARAM_INT);
        $stmt->bindValue(':name', $this->name, PDO::PARAM_STR);
        $stmt->bindValue(':type', $this->type, PDO::PARAM_STR);
        $db->execute($stmt);
        $this->entryId = $db->pdoHandle()->lastInsertId();

        // add secondary data tables for non-scalars
        switch ($this->type) {
            case self::LIST_TYPE:
                $this->createListDataTable($db);
                break;
            case self::MAP_TYPE:
                $this->createMapDataTable($db);
                break;
        }

        // add elements (which will create data table columns, so this needs to be last)
        foreach ($this->elements as $elem)
            $elem->insert($db, $this->entryId);
    }

    /** Update this schema entry in storage. */
    public function update(Datastore $db, SchemaEntry $newEntry)
    {
        // TODO reject type changes

        // update elements
        $oldElements = array();
        foreach ($this->elements as $oldElem)
            $oldElements[$oldElem->name] = $oldElem;

        foreach ($newEntry->elements as $newElem) {
            if (array_key_exists($newElem->name, $oldElements)) {
                // update
                // TODO this would require type conversion of the data!?
            } else {
                // insert
                $newElem->insert($db, $this->entryId);
            }
            unset($oldElements[$newElem->name]);
        }

        // delete whatever is left
        foreach($oldElements as $elem) {
            $elem->dropDataColumn($db);
            $elem->delete($db, $this->entryId);
        }
    }

    /** Delete this schema entry from storage. */
    public function delete(Datastore $db, $productId)
    {
        // delete data
        if ($this->isScalar()) {
            foreach ($this->elements as $elem)
                $elem->dropDataColumn($db);
        } else {
            $this->dropDataTable($db);
        }

        // delete elements
        $stmt = $db->prepare('DELETE FROM tbl_schema_element WHERE col_schema_id = :id');
        $stmt->bindValue(':id', $this->entryId, PDO::PARAM_INT);
        $db->execute($stmt);

        // delete entry
        $stmt = $db->prepare('DELETE FROM tbl_schema WHERE col_id = :id');
        $stmt->bindValue(':id', $this->entryId, PDO::PARAM_INT);
        $db->execute($stmt);
    }

    /** Convert a JSON object into an array of SchemaEntry instances. */
    static public function fromJson($jsonArray, Product &$product)
    {
        $entries = array();
        foreach ($jsonArray as $jsonObj) {
            if (!property_exists($jsonObj, 'name') || !property_exists($jsonObj, 'type'))
                throw new RESTException('Incomplete schema entry object.', 400);
            $e = new SchemaEntry($product);
            $e->name = strval($jsonObj->name);
            $e->type = strval($jsonObj->type);
            if (property_exists($jsonObj, 'elements'))
                $e->elements = SchemaEntryElement::fromJson($jsonObj->elements, $e);
            if (!$e->isValid())
                throw new RESTException('Invalid schema entry.', 400);
            array_push($entries, $e);
        }
        return $entries;
    }

    /** Data table name for secondary data tables. */
    public function dataTableName()
    {
        $tableName = 'pd2_' . Utils::normalizeString($this->product()->name)
            . '__' . Utils::normalizeString($this->name);
        return strtolower($tableName);
    }


    /** Create secondary data tables for list types. */
    private function createListDataTable(Datastore $db)
    {
        $stmt = $db->prepare('CREATE TABLE ' . $this->dataTableName(). ' ('
            . Utils::primaryKeyColumnDeclaration($db->driver(), 'col_id') . ', '
            . 'col_sample_id INTEGER REFERENCES ' . $this->product()->dataTableName() . '(col_id) ON DELETE CASCADE)'
        );
        $db->execute($stmt);
    }

    /** Create secondary data tables for map types. */
    private function createMapDataTable(Datastore $db)
    {
        $stmt = $db->prepare('CREATE TABLE ' . $this->dataTableName(). ' ('
            . Utils::primaryKeyColumnDeclaration($db->driver(), 'col_id') . ', '
            . 'col_sample_id INTEGER REFERENCES ' . $this->product()->dataTableName() . '(col_id) ON DELETE CASCADE, '
            . 'col_key ' . Utils::sqlStringType($db->driver()) . ' NOT NULL)'
        );
        $db->execute($stmt);
    }

    /** Drop secondary data tables for non-scalar types. */
    private function dropDataTable(Datastore $db)
    {
        $stmt = $db->prepare('DROP TABLE ' . $this->dataTableName());
        $db->execute($stmt);
    }
}

?>
