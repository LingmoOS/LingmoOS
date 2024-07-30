<?php
/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

include_once(__DIR__ . '/../shared/restdispatcher.php');
include_once(__DIR__ . '/../shared/admin.php');

$handler = new Admin();
RESTDispatcher::dispatch($handler);
?>
