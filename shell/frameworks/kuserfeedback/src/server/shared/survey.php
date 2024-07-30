<?php
/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

require_once('compat.php');
require_once('datastore.php');
require_once('restexception.php');

/** Represents a survey. */
class Survey
{
    public $uuid = '';
    public $name = '';
    public $url = '';
    public $active = false;
    public $target = null;

    private $id = -1;
    /** Returns an array of all surveys for @p productName. */
    public static function surveysForProduct(Datastore $db, $productName)
    {
        $stmt = $db->prepare('
            SELECT tbl_survey.col_id, tbl_survey.col_uuid, tbl_survey.col_name, tbl_survey.col_url, tbl_survey.col_active, tbl_survey.col_target
            FROM tbl_survey JOIN tbl_product ON (tbl_survey.col_product_id = tbl_product.col_id)
            WHERE tbl_product.col_name = :productName
        ');
        $stmt->bindValue(':productName', $productName, PDO::PARAM_STR);
        $db->execute($stmt);

        $surveys = array();
        foreach ($stmt as $row) {
            $s = new Survey();
            $s->id = intval($row['col_id']);
            $s->uuid = strval($row['col_uuid']);
            $s->name = strval($row['col_name']);
            $s->url = strval($row['col_url']);
            $s->active = boolval($row['col_active']);
            $s->target = strval($row['col_target']);
            array_push($surveys, $s);
        }

        return $surveys;
    }

    /** Returns an array of all active surveys for @p product. */
    public static function activeSurveysForProduct(Datastore $db, Product $product)
    {
        $stmt = $db->prepare('SELECT col_id, col_uuid, col_name, col_url, col_target FROM tbl_survey WHERE col_product_id = :productId AND col_active = :active');
        $stmt->bindValue(':productId', $product->id(), PDO::PARAM_INT);
        $stmt->bindValue(':active', true, PDO::PARAM_BOOL);
        $db->execute($stmt);

        $surveys = array();
        foreach ($stmt as $row) {
            $s = new Survey();
            $s->id = intval($row['col_id']);
            $s->uuid = strval($row['col_uuid']);
            $s->name = strval($row['col_name']);
            $s->url = strval($row['col_url']);
            $s->active = true;
            $s->target = strval($row['col_target']);
            array_push($surveys, $s);
        }

        return $surveys;
    }

    /** Insert a new survey into storage for product @p product. */
    public function insert(Datastore $db, Product $product)
    {
        $stmt = $db->prepare('
            INSERT INTO tbl_survey (col_product_id, col_uuid, col_name, col_url, col_active, col_target)
            VALUES (:productId, :uuid, :name, :url, :active, :target)
        ');
        $stmt->bindValue(':productId', $product->id(), PDO::PARAM_INT);
        $stmt->bindValue(':uuid', $this->uuid, PDO::PARAM_STR);
        $stmt->bindValue(':name', $this->name, PDO::PARAM_STR);
        $stmt->bindValue(':url', $this->url, PDO::PARAM_STR);
        $stmt->bindValue(':active', $this->active, PDO::PARAM_BOOL);
        $stmt->bindValue(':target', $this->target, PDO::PARAM_STR);
        $db->execute($stmt);
        $this->id = $db->pdoHandle()->lastInsertId();
    }

    /** Update an existing survey in storage. */
    public function update(Datastore $db)
    {
        $stmt = $db->prepare('
            UPDATE tbl_survey SET
                col_name = :name,
                col_url = :url,
                col_active = :active,
                col_target = :target
            WHERE col_uuid = :surveyUuid
        ');
        $stmt->bindValue(':name', $this->name, PDO::PARAM_STR);
        $stmt->bindValue(':url', $this->url, PDO::PARAM_STR);
        $stmt->bindValue(':active', $this->active, PDO::PARAM_BOOL);
        $stmt->bindValue(':target', $this->target, PDO::PARAM_STR);
        $stmt->bindValue(':surveyUuid', $this->uuid, PDO::PARAM_STR);
        $db->execute($stmt);
    }

    /** Delete this existing survey from storage. */
    public function delete(Datastore $db)
    {
        $stmt = $db->prepare('DELETE FROM tbl_survey WHERE col_uuid = :surveyUuid');
        $stmt->bindValue(':surveyUuid', $this->uuid, PDO::PARAM_STR);
        $db->execute($stmt);
    }

    /** Create one Survey instance based on JSON input and verifies it is valid. */
    public static function fromJson($jsonString)
    {
        $jsonObj = json_decode($jsonString);
        if (!property_exists($jsonObj, 'uuid') || !property_exists($jsonObj, 'name') || !property_exists($jsonObj, 'url') || !property_exists($jsonObj, 'active'))
            throw new RESTException('Incomplete survey object.', 400);

        $s = new Survey();
        $s->uuid = strval($jsonObj->uuid);
        $s->name = $jsonObj->name;
        $s->url = strval($jsonObj->url);
        $s->active = boolval($jsonObj->active);
        if (property_exists($jsonObj, 'target'))
            $s->target = strval($jsonObj->target);
        if (property_exists($jsonObj, 'id'))
            $s->id = $jsonObj->id;

        // verify
        if (strlen($s->uuid) <= 0)
            throw new RESTException('Empty survey UUID.', 400);
        if (strlen($s->name) <= 0 || !is_string($s->name))
            throw new RESTException('Empty product name.', 400);
        if (!is_numeric($s->id))
            throw new RESTException('Invalid survey id.', 400);

        return $s;
    }
}

?>
