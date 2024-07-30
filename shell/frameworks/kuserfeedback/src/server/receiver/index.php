<?php
/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

require_once(__DIR__ . '/../shared/restdispatcher.php');
require_once(__DIR__ . '/../shared/receiver.php');

$handler = new Receiver();
RESTDispatcher::dispatch($handler);
?>
