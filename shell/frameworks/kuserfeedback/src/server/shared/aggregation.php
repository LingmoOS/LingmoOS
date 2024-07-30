<?php
/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

require_once('datastore.php');
require_once('product.php');

/** Represents a product aggregation setting. */
class Aggregation
{
    public $type;
    public $name = '';
    public $elements = array();

    /** Load aggregation settings for @p $product from storage. */
    static public function aggregationsForProduct(DataStore $db, Product $product)
    {
        $sql = 'SELECT col_type, col_name, col_elements FROM tbl_aggregation WHERE col_product_id = :productId ORDER BY col_id';
        $stmt = $db->prepare($sql);
        $stmt->bindValue(':productId', $product->id(), PDO::PARAM_INT);
        $db->execute($stmt);

        $aggrs = array();
        foreach ($stmt as $row) {
            $a = new Aggregation;
            $a->type = strval($row['col_type']);
            $a->name = strval($row['col_name']);
            $a->elements = json_decode(strval($row['col_elements']));
            array_push($aggrs, $a);
        }
        return $aggrs;
    }

    /** Update aggregation settings for @p $product to @p $aggregations. */
    static public function update(DataStore $db, Product $product, $aggregations)
    {
        Aggregation::delete($db, $product);

        $sql = 'INSERT INTO tbl_aggregation (col_product_id, col_type, col_name, col_elements) VALUES (:productId, :type, :name, :elements)';
        $stmt = $db->prepare($sql);
        $stmt->bindValue(':productId', $product->id(), PDO::PARAM_INT);
        foreach ($aggregations as $a) {
            $stmt->bindValue(':type', $a->type, PDO::PARAM_STR);
            $stmt->bindValue(':name', $a->name, PDO::PARAM_STR);
            $stmt->bindValue(':elements', json_encode($a->elements), PDO::PARAM_STR);
            $db->execute($stmt);
        }
    }

    /** Delete all aggregation settings for @p $product. */
    static public function delete(DataStore $db, Product $product)
    {
        $sql = 'DELETE FROM tbl_aggregation WHERE col_product_id = :productId';
        $stmt = $db->prepare($sql);
        $stmt->bindValue(':productId', $product->id(), PDO::PARAM_INT);
        $db->execute($stmt);
    }

    /** Convert a JSON object into an array of Aggregation instances. */
    static public function fromJson($jsonArray)
    {
        if (!is_array($jsonArray))
            throw new RESTException('Wrong aggregation list format.', 400);

        $aggrs = array();
        foreach ($jsonArray as $jsonObj) {
            if (!is_object($jsonObj))
                throw new RESTException('Wrong aggregation format.', 400);
            if (!property_exists($jsonObj, 'type') || !property_exists($jsonObj, 'name'))
                throw new RESTException('Incomplete aggregation object.', 400);

            $a = new Aggregation;
            $a->type = strval($jsonObj->type);
            $a->name = strval($jsonObj->name);
            if (property_exists($jsonObj, 'elements'))
                $a->elements = $jsonObj->elements;
            array_push($aggrs, $a);
        }
        return $aggrs;
    }
}

?>
