<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') exit;

$input = file_get_contents('php://input');
$data  = json_decode($input, true);

if (!$data || !isset($data['timestamp']) || !isset($data['event'])) {
  http_response_code(400);
  echo json_encode(['ok'=>false, 'msg'=>'invalid payload']);
  exit;
}

$pdo = require __DIR__ . '/bootstrap.php';

$stmt = $pdo->prepare("
  INSERT INTO alerts (timestamp, distance_cm, threshold_cm, event, source)
  VALUES (:timestamp, :distance_cm, :threshold_cm, :event, :source)
");
$stmt->execute([
  ':timestamp'    => $data['timestamp'],
  ':distance_cm'  => $data['distance_cm'] ?? null,
  ':threshold_cm' => $data['threshold_cm'] ?? null,
  ':event'        => $data['event'],
  ':source'       => $data['source'] ?? null
]);

echo json_encode(['ok'=>true, 'id'=>$pdo->lastInsertId()]);
