<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');         // dev only
header('Access-Control-Allow-Methods: POST, OPTIONS');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') exit;

$input = file_get_contents('php://input');
$data  = json_decode($input, true);
if (!$data || !isset($data['timestamp'])) {
  http_response_code(400);
  echo json_encode(['ok'=>false, 'msg'=>'invalid payload']);
  exit;
}

$file = __DIR__ . '/../data/alerts.json';
if (!file_exists($file)) file_put_contents($file, "[]");
$log = json_decode(file_get_contents($file), true);
$log[] = $data;

// keep file small
$log = array_slice($log, -200);
file_put_contents($file, json_encode($log, JSON_PRETTY_PRINT));

echo json_encode(['ok'=>true]);
