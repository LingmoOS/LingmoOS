<?php
/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

include_once(__DIR__ . '/../shared/restdispatcher.php');
include_once(__DIR__ . '/../shared/analytics.php');

$handler = new Analytics();
RESTDispatcher::dispatch($handler);
?>
