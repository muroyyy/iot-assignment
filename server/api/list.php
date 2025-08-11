<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');

$pdo = require __DIR__ . '/bootstrap.php';

$limit = isset($_GET['limit']) ? max(1, min(200, (int)$_GET['limit'])) : 200;

$q = $pdo->prepare("SELECT timestamp, distance_cm, threshold_cm, event, source
                    FROM alerts ORDER BY id DESC LIMIT :limit");
$q->bindValue(':limit', $limit, PDO::PARAM_INT);
$q->execute();

$rows = $q->fetchAll(PDO::FETCH_ASSOC);

// return oldest→newest for nicer charts/tables
echo json_encode(array_reverse($rows));
