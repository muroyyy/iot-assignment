const API = 'http://localhost/code/server/api/list.php';
const pollMs = 5000;

async function fetchAlerts() {
  try {
    const r = await fetch(API + '?t=' + Date.now()); // bust cache
    if (!r.ok) throw new Error('HTTP ' + r.status);
    return await r.json();
  } catch (e) {
    console.error(e);
    return [];
  }
}

function render(data) {
  const tbody = document.querySelector('#tbl tbody');
  tbody.innerHTML = '';

  // latest first
  const rows = [...data].reverse().slice(0, 20);
  for (const a of rows) {
    const tr = document.createElement('tr');
    tr.innerHTML = `
      <td>${new Date(a.timestamp).toLocaleString()}</td>
      <td>${a.source || '-'}</td>
      <td>${a.event}</td>
      <td>${a.distance_cm?.toFixed?.(1) ?? '-'}</td>
      <td>${a.threshold_cm ?? '-'}</td>`;
    tbody.appendChild(tr);
  }

  // stats
  document.getElementById('total').textContent = data.length;
  const last = rows[0];
  document.getElementById('lastDistance').textContent =
    last?.distance_cm ? last.distance_cm.toFixed(1) : '–';
  document.getElementById('lastEvent').textContent = last?.event ?? '–';

  const chip = document.getElementById('chip');
  if (last?.event === 'intrusion') {
    chip.textContent = 'Intrusion';
    chip.className = 'chip alert';
  } else {
    chip.textContent = 'Idle';
    chip.className = 'chip ok';
  }
}

async function tick() {
  const alerts = await fetchAlerts();
  render(alerts);
}
tick();
setInterval(tick, pollMs);
