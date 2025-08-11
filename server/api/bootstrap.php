<?php
$dbPath = __DIR__ . '/../data/alerts.db';
$pdo = new PDO('sqlite:' . $dbPath);
$pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

$pdo->exec("
  CREATE TABLE IF NOT EXISTS alerts (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp TEXT NOT NULL,
    distance_cm REAL,
    threshold_cm REAL,
    event TEXT,
    source TEXT
  );
");
return $pdo;
