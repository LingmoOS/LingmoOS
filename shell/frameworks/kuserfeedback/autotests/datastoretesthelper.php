<?php
/*
    SPDX-FileCopyrightText: 2017-2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: MIT
*/

require_once('../src/server/shared/datastore.php');

// default database configuration for sqlite, can be overridden by localconfig.php
$USERFEEDBACK_DB_DRIVER = 'sqlite';
$USERFEEDBACK_DB_NAME =  ':memory:';

class DatastoreTestHelper
{
    public static function setup()
    {
        // database schema setup
        $db = new Datastore();
        $db->checkSchema();

        // add dummy test data (from previous use of DBUnit, might make sense to move this to the corresponding tests relying on this instead)
        $db->pdoHandle()->exec('INSERT INTO tbl_product (col_id, col_name) VALUES (2, "org.kde.UnitTest")');

        $db->pdoHandle()->exec('INSERT INTO tbl_schema (col_id, col_product_id, col_name, col_type) VALUES (31, 2, "entry1", "scalar")');
        $db->pdoHandle()->exec('INSERT INTO tbl_schema (col_id, col_product_id, col_name, col_type) VALUES (32, 2, "entry2", "list")');

        $db->pdoHandle()->exec('INSERT INTO tbl_schema_element (col_id, col_schema_id, col_name, col_type) VALUES (42, 31, "element11", "string")');
        $db->pdoHandle()->exec('INSERT INTO tbl_schema_element (col_id, col_schema_id, col_name, col_type) VALUES (43, 31, "element12", "bool")');
        $db->pdoHandle()->exec('INSERT INTO tbl_schema_element (col_id, col_schema_id, col_name, col_type) VALUES (44, 32, "element21", "int")');
        $db->pdoHandle()->exec('INSERT INTO tbl_schema_element (col_id, col_schema_id, col_name, col_type) VALUES (45, 32, "element22", "number")');

        $db->pdoHandle()->exec('INSERT INTO tbl_survey (col_id, col_uuid, col_product_id, col_name, col_url, col_active, col_target) VALUES (101, "{962bbd80-7120-4f18-a4c0-5800fa323868}", 2, "myActiveSurvey", "http://survey.example/active", 1, "usageTime.value >= 3600")');
        $db->pdoHandle()->exec('INSERT INTO tbl_survey (col_id, col_uuid, col_product_id, col_name, col_url, col_active, col_target) VALUES (102, "{bdfa82c5-238f-404b-a441-07ca3d3eff7f}", 2, "myInactiveSurvey", "http://survey.example/inactive", 0, "screen[0].dpi < 200")');

        $stmt =
        $db->pdoHandle()->exec("INSERT INTO tbl_aggregation (col_id, col_name, col_product_id, col_type, col_elements) VALUES (201, 'Category: entry1.elem11', 2, 'category', '[ { \"type\": \"value\", \"schemaEntry\": \"entry1\", \"schemaEntryElement\": \"element11\" } ]')");
        $db->pdoHandle()->exec("INSERT INTO tbl_aggregation (col_id, col_name, col_product_id, col_type, col_elements) VALUES (202, 'entry2 size distribution', 2, 'numeric', '[ { \"type\": \"size\", \"schemaEntry\": \"entry2\" } ]')");

        return $db;
    }
}

?>
