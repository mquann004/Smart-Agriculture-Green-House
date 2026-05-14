#ifndef PAGE_HISTORY_H
#define PAGE_HISTORY_H

// ===== TRANG LỊCH SỬ DỮ LIỆU =====
const char PAGE_HISTORY[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><title>Lịch sử dữ liệu</title>
<meta name='viewport' content='width=device-width, initial-scale=1'>
<style>
    body{font-family:system-ui, sans-serif;margin:0;padding:20px;background:#f8f9fa;}
    .container{max-width:1000px;margin:0 auto;background:white;padding:20px;border-radius:12px;box-shadow:0 4px 12px rgba(0,0,0,0.08);}
    h1{color:#28a745;text-align:center;}
    table{width:100%;border-collapse:collapse;margin-top:20px;}
    th,td{border:1px solid #ddd;padding:12px;text-align:left;}
    th{background:#28a745;color:white;text-align:center;}
    tr:nth-child(even){background-color:#f2f2f2;}
    button{padding:10px 20px;border:none;border-radius:5px;cursor:pointer;background:#2196F3;color:white;font-size:1em;}
</style>
</head><body>
<div class='container'><h1>Lịch sử dữ liệu cảm biến</h1>
<button onclick="window.location.href='/'">Quay lại Dashboard</button>
<table><thead><tr><th>Thời gian</th><th>Nhiệt độ (&deg;C)</th><th>Độ ẩm (%)</th><th>Độ ẩm đất</th><th>Ánh sáng (lx)</th><th>CO2 (ppm)</th></tr></thead>
<tbody id='historyTable'></tbody></table>
<script>
    function updateHistory(){fetch('/api/history').then(r=>r.json()).then(data=>{
        let tbody=document.getElementById('historyTable');tbody.innerHTML='';
        data.forEach(d=>{
            let row=tbody.insertRow();
            let time=new Date(d.timestamp).toLocaleString('vi-VN');
            row.innerHTML=`<td>${time}</td><td>${d.temperature.toFixed(1)}</td><td>${d.humidity.toFixed(1)}</td><td>${d.soilMoisture}</td><td>${d.lightLevel.toFixed(1)}</td><td>${d.co2Level}</td>`;
        });
    });}
    updateHistory(); setInterval(updateHistory, 5000);
</script>
</body></html>
)rawliteral";

#endif
