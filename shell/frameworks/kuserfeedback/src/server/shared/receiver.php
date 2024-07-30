<?php
/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

require_once('datastore.php');
require_once('product.php');
require_once('restexception.php');
require_once('utils.php');
require_once('sample.php');
require_once('survey.php');

/** Command handler for the data receiver. */
class Receiver
{

/** Dummy command for probing for redirects. */
function get_submit($productName)
{
}

/** Data submission command. */
function post_submit($productName)
{
    // load JSON data sent by the client
    $rawPostData = file_get_contents('php://input');

    // look up product
    $db = new DataStore();
    $db->beginTransaction();
    $product = Product::productByName($db, $productName);
    if (is_null($product))
        throw new RESTException('Unknown product.', 404);

    Sample::insert($db, $rawPostData, $product);

    // read survey from db
    $responseData = array();
    $surveys = Survey::activeSurveysForProduct($db, $product);
    $responseData['surveys'] = $surveys;
    $db->commit();

    header('Content-Type: text/json');
    echo(json_encode($responseData));
}

}

?>
