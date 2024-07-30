<?php
/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

require_once('config.php');
require_once('restexception.php');
require_once('utils.php');

/** Handles all database operations. */
class DataStore {

private $db;

function __construct()
{
    try {
        $conf = new Config;
        $this->db = new PDO($conf->dsn(), $conf->username(), $conf->password());
    } catch (PDOException $e) {
        throw new RESTException($e->getMessage(), 500);
    }
}

/** Returns the type of the database driver. */
public function driver()
{
    return $this->db->getAttribute(PDO::ATTR_DRIVER_NAME);
}

/** Access to the PDO handle. */
public function pdoHandle()
{
    return $this->db;
}

/** Prepare a query. */
public function prepare($queryString)
{
    try {
        $res = $this->db->prepare($queryString);
        $this->checkError($res);
    } catch (PDOException $e) {
        throw new RESTException($e->getMessage(), 500);
    }
    return $res;
}

/** Execute a prepared query and check the result for errors. */
public function execute(PDOStatement $stmt)
{
    try {
        if (!$stmt->execute()) {
            $err = $stmt->errorInfo();
            $msg = "SQL execution error: SQLSTATE: " . $err[0] . "\nDriver error code: " . $err[1] . "\nDriver error message: " . $err[2];
            throw new RESTException($msg, 500);
        }
    } catch (PDOException $e) {
        throw new RESTException($e->getMessage(), 500);
    }
}

/** Check database result for errors, and if so, bail out. */
private function checkError($res)
{
    if ($res === FALSE) {
        $err = $this->db->errorInfo();
        $msg = "SQLSTATE: " . $err[0] . "\nDriver error code: " . $err[1] . "\nDriver error message: " . $err[2];
        throw new RESTException($msg, 500);
    }
}

/** Begin transaction. */
public function beginTransaction()
{
    $this->db->beginTransaction();
}

/** Commit transaction. */
public function commit()
{
    $this->db->commit();
}

/** Abort an ongoing transaction. */
public function rollback()
{
    $this->db->rollback();
}

/** Verify the database schema, and fix if needed. */
public function checkSchema()
{
    $this->beginTransaction();
    $currentVersion = $this->schemaVersion();
    $schemaFile = __DIR__ . '/schema.json';
    $schemaDefs = json_decode(file_get_contents($schemaFile), true);
    $targetVersion = count($schemaDefs['schema']);

    $res = array();
    $res['previousSchemaVersion'] = $currentVersion;
    $res['currentSchemaVersion'] = $targetVersion;

    if ($currentVersion == $targetVersion)
        return $res;
    if ($currentVersion > $targetVersion || $currentVersion < 0)
        throw new RESTException('Current schema version is invalid: ' . $currentVersion . '.', 500);

    # apply database updates
    error_log('Current schema version: ' . $currentVersion . ' should be: ' . $targetVersion);
    for ($i = $currentVersion; $i < $targetVersion; $i++) {
        error_log("Applying update $i...");
        $this->applySchemaChange($schemaDefs['schema'][$i]);
    }
    $this->commit();
    return $res;
}

/** Returns the current schema version. */
private function schemaVersion()
{
    try {
        $res = $this->db->query('SELECT col_version FROM tbl_version');
        if ($res === FALSE) {
            // restart transaction, so this fail doesn't block the following commands
            $this->rollback();
            $this->beginTransaction();
            return 0;
        }
        foreach ($res as $row)
            return intval($row['col_version']);
    } catch (PDOException $e) {
        // table does not exist yet
    }
    return 0;
}

/** Applies a list of schema setup commands. */
private function applySchemaChange($schemaDef)
{
    if (array_key_exists($this->driver(), $schemaDef))
        $cmds = $schemaDef[$this->driver()];
    else
        $cmds = $schemaDef['sql'];

    foreach($cmds as $cmd) {
        $res = $this->db->exec($cmd);
        $this->checkError($res);
    }
    $res = $this->db->exec('UPDATE tbl_version SET col_version = ' . $schemaDef['version']);
    $this->checkError($res);
}

}

?>
