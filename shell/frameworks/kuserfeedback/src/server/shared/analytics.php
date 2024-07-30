<?php
/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

require_once('datastore.php');
require_once('product.php');
require_once('sample.php');
require_once('survey.php');
require_once('utils.php');

/** Command handler for the analytics interface. */
class Analytics
{

/** Schema check and update. */
public function get_check_schema()
{
    // check PHP version
    if (PHP_VERSION_ID < 50400)
        throw new RESTException('Minium required PHP version is 5.4, found ' . PHP_VERSION, 500);

    // check database layout
    $db = new DataStore();
    $res = $db->checkSchema();
    $res['protocolVersion'] = 2;

    header('Content-Type: text/json');
    echo(json_encode($res));
}

/** List all products. */
public function get_products()
{
    $db = new DataStore();
    $products = Product::allProducts($db);
    $json = json_encode($products);

    header('Content-Type: text/json');
    echo($json);
}

/** List data for a product. */
public function get_data($productName)
{
    $db = new DataStore();
    $product = Product::productByName($db, $productName);
    if (is_null($product))
        throw new RESTException('Product not found.', 404);

    header('Content-Type: text/json');
    Sample::echoDataAsJson($db, $product);
}

/** List all surveys for a product. */
public function get_surveys($productName)
{
    if ($productName == "")
        Utils::httpError(400, "No product id specified.");

    $db = new DataStore();
    $surveys = Survey::surveysForProduct($db, $productName);

    header('Content-Type: text/json');
    echo(json_encode($surveys));
}

}

?>
