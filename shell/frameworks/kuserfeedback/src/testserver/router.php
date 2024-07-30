<?php
/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

/** NOTE: This file is for local autotests only, do not deploy! */

// default database configuration for sqlite, can be overridden by localconfig.php
$USERFEEDBACK_DB_DRIVER = 'sqlite';
$USERFEEDBACK_DB_NAME =  __DIR__ . '/../server/data/db.sqlite';

$path = explode('/', $_SERVER['REQUEST_URI']);

switch ($path[1]) {
    case 'admin':
        $_SERVER['PHP_SELF'] = '/admin/index.php';
        include '../server/admin/index.php';
        return;
    case 'analytics':
        $_SERVER['PHP_SELF'] = '/analytics/index.php';
        include '../server/analytics/index.php';
        return;
    case 'receiver':
        $_SERVER['PHP_SELF'] = '/receiver/index.php';
        include '../server/receiver/index.php';
        return;

    // redirection test cases
    case 'absRedirect':
        http_response_code(307);
        $url = str_replace('/absRedirect/', '/', $_SERVER['REQUEST_URI']);
        header('Location: http://'. $_SERVER['SERVER_NAME'] . ':' . $_SERVER['SERVER_PORT'] . $url);
        return;
    case 'relRedirect':
        http_response_code(307);
        $url = str_replace('/relRedirect/', '/../', $_SERVER['REQUEST_URI']);
        header('Location: ' . $url);
        return;
    case 'circleRedirect':
        http_response_code(307);
        header('Location: http://'. $_SERVER['SERVER_NAME'] . ':' . $_SERVER['SERVER_PORT'] . $_SERVER['REQUEST_URI']);
        return;
}

http_response_code(404);
header('Content-Type: text/plain');
print('Invalid request URI: ' . $_SERVER['REQUEST_URI']);

?>
