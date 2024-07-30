<?php
/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

require_once('compat.php');
require_once('datastore.php');
require_once('product.php');
require_once('restexception.php');
require_once('schemaentry.php');
require_once('schemaentryelement.php');

/** Represents a data sample received from a user. */
class Sample
{
    /** JSON for all data of the given product. */
    public static function echoDataAsJson(DataStore $db, Product $product)
    {
        echo '[';
        $first = false;
        $i = 0;

        // query scalar table
        $scalarSql = 'SELECT col_id, col_timestamp ';
        foreach ($product->schema as $entry) {
            if (!$entry->isScalar())
                continue;
            foreach ($entry->elements as $elem)
                $scalarSql .= ', ' . $elem->dataColumnName();
        }
        $scalarSql .= ' FROM ' . $product->dataTableName() . ' ORDER BY col_timestamp ASC';
        $scalarStmt = $db->prepare($scalarSql);
        $db->execute($scalarStmt);
        foreach ($scalarStmt as $scalarRow) {
            $rowData = array();
            $rowData['id'] = intval($scalarRow['col_id']);
            $rowData['timestamp'] = $scalarRow['col_timestamp'];
            foreach ($product->schema as $entry) {
                if ($entry->isScalar()) {
                    $entryData = null;
                    foreach ($entry->elements as $elem) {
                        $value = self::valueFromDb($elem, $scalarRow[$elem->dataColumnName()]);
                        if (!is_null($value))
                            $entryData[$elem->name] = $value;
                    }
                    if (!is_null($entryData))
                        $rowData[$entry->name] = $entryData;
                } else {
                    $sql = 'SELECT col_sample_id';
                    if ($entry->type == SchemaEntry::MAP_TYPE)
                        $sql .= ', col_key';
                    foreach ($entry->elements as $elem)
                        $sql .= ', ' . $elem->dataColumnName();
                    $sql .= ' FROM ' . $entry->dataTableName() . '  WHERE col_sample_id = ' . $rowData['id'] . ' ORDER BY col_id ASC ';
                    $stmt = $db->prepare($sql);
                    $db->execute($stmt);
                    foreach ($stmt as $row) {
                        $entryData = null;
                        foreach ($entry->elements as $elem) {
                            $value = self::valueFromDb($elem, $row[$elem->dataColumnName()]);
                            if (!is_null($value))
                                $entryData[$elem->name] = $value;
                        }
                        if (!array_key_exists($entry->name, $rowData))
                            $rowData[$entry->name] = array();
                        if ($entry->type == SchemaEntry::MAP_TYPE)
                            $rowData[$entry->name][$row['col_key']] = $entryData;
                        else
                            array_push($rowData[$entry->name], $entryData);
                    }
                }
            }

            if ($i != 0)
                echo ',';
            echo(json_encode($rowData));
            $i++;
        }
        echo ']';
    }

    /** Insert a received sample for @p product into the data store. */
    public static function insert(DataStore $db, $jsonData, Product $product)
    {
        $jsonObj = json_decode($jsonData);
        if (!is_object($jsonObj))
            throw new RESTException('Invalid data sample format.', 400);
        if (empty(get_object_vars($jsonObj)))
            return; // happens when application is just querying for surveys, or on entirely invalid input
        if (property_exists($jsonObj, 'id') || property_exists($jsonObj, 'timestamp'))
            throw new RESTException('Invalid data sample.', 400);

        $sampleId = self::insertScalar($db, $jsonObj, $product);

        foreach ($product->schema as $entry) {
            if ($entry->isScalar())
                continue;
            self::insertNonScalar($db, $jsonObj, $entry, $sampleId);
        }
    }

    /** Insert a array of samples previously exported for @p product.
     *  Unlike Sample::insert, this will preserve ids and timestamps.
     */
    public static function import(DataStore $db, $jsonData, Product $product)
    {
        $jsonArray = json_decode($jsonData);
        if (!is_array($jsonArray))
            throw new RESTException('Invalid data sample format.', 400);

        foreach ($jsonArray as $sampleObj) {
            if (property_exists($sampleObj, 'id') || !property_exists($sampleObj, 'timestamp'))
                throw new RESTException('Invalid data sample.', 400);

            $sampleId = self::insertScalar($db, $sampleObj, $product);
            foreach ($product->schema as $entry) {
                if ($entry->isScalar())
                    continue;
                self::insertNonScalar($db, $sampleObj, $entry, $sampleId);
            }
        }
    }

    /** Insert scalar data for @p product.
     *  @return The sample id for use in non-scalar data tables.
     */
    private static function insertScalar(DataStore $db, $jsonObj, Product $product)
    {
        $columns = array();
        $binds = array();
        $values = array();

        if (property_exists($jsonObj, 'timestamp')) {
            array_push($columns, 'col_timestamp');
            array_push($binds, ':timestamp');
            $values[':timestamp'] = array($jsonObj->timestamp, PDO::PARAM_STR);
        }

        foreach ($product->schema as $entry) {
            if (!$entry->isScalar())
                continue;
            if (!property_exists($jsonObj, $entry->name) || !is_object($jsonObj->{$entry->name}))
                continue;
            foreach ($entry->elements as $elem) {
                if (!property_exists($jsonObj->{$entry->name}, $elem->name))
                    continue;

                $v = $jsonObj->{$entry->name}->{$elem->name};
                if (!self::isCorrectType($elem, $v))
                    continue;

                $bind = ':' . $elem->dataColumnName();
                array_push($columns, $elem->dataColumnName());
                array_push($binds, $bind);
                $values[$bind] = array($v, self::pdoParamType($elem));
            }
        }

        $sql = 'INSERT INTO ' . $product->dataTableName();
        if (count($columns) > 0) {
            $sql .= ' (' . implode(', ', $columns) . ') VALUES (';
            $sql .= implode(', ', $binds) . ')';
        } else {
            if ($db->driver() == 'sqlite')
                $sql .= ' DEFAULT VALUES';
            else
                $sql .= ' VALUES ()';
        }
        $stmt = $db->prepare($sql);
        foreach ($values as $key => $data) {
            $stmt->bindValue($key, $data[0], $data[1]);
        }
        $db->execute($stmt);

        return $db->pdoHandle()->lastInsertId();
    }

    /** Insert non-scalar elements in seconadary data tables. */
    private static function insertNonScalar(DataStore $db, $jsonObj, SchemaEntry $schemaEntry, $sampleId)
    {
        if (!property_exists($jsonObj, $schemaEntry->name))
            return;
        $data = $jsonObj->{$schemaEntry->name};
        switch ($schemaEntry->type) {
            case SchemaEntry::LIST_TYPE:
                if (!is_array($data))
                    return;
                break;
            case SchemaEntry::MAP_TYPE:
                if (!is_object($data))
                    return;
                break;
            default:
                Utils::httpError(500, "Unknown non-scalar schema entry type.");
        }

        $columns = array('col_sample_id');
        $binds = array(':sampleId');
        if ($schemaEntry->type == SchemaEntry::MAP_TYPE) {
            array_push($columns, 'col_key');
            array_push($binds, ':key');
        }
        foreach ($schemaEntry->elements as $elem) {
            array_push($columns, $elem->dataColumnName());
            array_push($binds, ':' . $elem->dataColumnName());
        }
        $sql = 'INSERT INTO ' . $schemaEntry->dataTableName()
             . ' (' . implode(', ', $columns) . ') VALUES ('
             . implode(', ', $binds) . ')';
        $stmt = $db->prepare($sql);

        foreach ($data as $key => $entry) {
            if (!is_object($entry))
                continue;
            $stmt->bindValue(':sampleId', $sampleId, PDO::PARAM_INT);
            if ($schemaEntry->type == SchemaEntry::MAP_TYPE) {
                if (!is_string($key) || strlen($key) == 0)
                    continue;
                $stmt->bindValue(':key', $key, PDO::PARAM_STR);
            }
            foreach ($schemaEntry->elements as $elem) {
                $stmt->bindValue(':' . $elem->dataColumnName(), null, self::pdoParamType($elem));
                if (!property_exists($entry, $elem->name))
                    continue;
                $v = $entry->{$elem->name};
                if (!self::isCorrectType($elem, $v))
                    continue;
                $stmt->bindValue(':' . $elem->dataColumnName(), $v, self::pdoParamType($elem));
            }
            $db->execute($stmt);
        }
    }

    /** Fix database output that lost the correct type for a schema entry element. */
    private static function valueFromDb(SchemaEntryElement $elem, $dbValue)
    {
        if (is_null($dbValue))
            return null;
        switch ($elem->type) {
            case SchemaEntryElement::STRING_TYPE:
                return strval($dbValue);
            case SchemaEntryElement::INT_TYPE:
                return intval($dbValue);
            case SchemaEntryElement::NUMBER_TYPE:
                return floatval($dbValue);
            case SchemaEntryElement::BOOL_TYPE:
                return boolval($dbValue);
        }
        Utils::httpError(500, "Invalid schema entry element type.");
    }

    /** Check if the given input value @p $v matches the expected type for element @p $elem. */
    private static function isCorrectType(SchemaEntryElement $elem, $v)
    {
        switch ($elem->type) {
            case SchemaEntryElement::STRING_TYPE:
                if (!is_string($v))
                    return false;
                break;
            case SchemaEntryElement::INT_TYPE:
                if (!is_int($v))
                    return false;
                break;
            case SchemaEntryElement::NUMBER_TYPE:
                if (!is_float($v))
                    return false;
                break;
            case SchemaEntryElement::BOOL_TYPE:
                if (!is_bool($v))
                    return false;
                break;
        }
        return true;
    }

    /** Determine PDO column type for the given element. */
    private static function pdoParamType(SchemaEntryElement $elem)
    {
        switch ($elem->type) {
            case SchemaEntryElement::STRING_TYPE:
                return PDO::PARAM_STR;
            case SchemaEntryElement::INT_TYPE:
                return PDO::PARAM_INT;
            case SchemaEntryElement::NUMBER_TYPE:
                return PDO::PARAM_STR; // yes, really...
            case SchemaEntryElement::BOOL_TYPE:
                return PDO::PARAM_BOOL;
        }
        throw new RESTException('Unsupported element type.', 500);
    }
}

?>
