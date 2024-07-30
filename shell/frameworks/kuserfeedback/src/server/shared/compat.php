<?php
/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

/** Comaptibility functions for PHP 5.4 */

if (PHP_VERSION_ID < 50500) {

if (!function_exists('boolval')) {
    function boolval( $my_value ) {
        return (bool)$my_value;
    }
}

}

?>
