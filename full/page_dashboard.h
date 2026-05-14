#ifndef PAGE_DASHBOARD_H
#define PAGE_DASHBOARD_H

// ===== TRANG DASHBOARD CHÍNH =====
const char PAGE_DASHBOARD[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
    <meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Smart Greenhouse</title>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.1.1/css/all.min.css">
    <style>
        :root{--primary-color:#28a745;--secondary-color:#f8f9fa;--card-bg:#ffffff;--text-color:#333;--border-radius:12px;--box-shadow:0 4px 12px rgba(0,0,0,0.08);}
        body{font-family:system-ui,-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,Oxygen,Ubuntu,Cantarell,'Open Sans','Helvetica Neue',sans-serif;background-color:var(--secondary-color);color:var(--text-color);margin:0;padding:20px;}
        .container{max-width:1200px;margin:auto;}
        header{text-align:center;margin-bottom:20px;color:var(--primary-color);}
        .grid{display:grid;grid-template-columns:2fr 1fr;gap:20px;}
        .card{background:var(--card-bg);border-radius:var(--border-radius);padding:20px;box-shadow:var(--box-shadow);margin-bottom:20px;}
        .card-title{display:flex;align-items:center;margin-top:0;font-size:1.2em;border-bottom:1px solid #eee;padding-bottom:10px;margin-bottom:15px;}
        .card-title i{margin-right:8px;color:var(--primary-color);}
        .sensor-grid{display:grid;grid-template-columns:1fr 1fr;gap:15px;}
        .sensor-item{background:var(--secondary-color);padding:15px;border-radius:8px;text-align:center;}
        .sensor-item .icon{font-size:1.8em;margin-bottom:5px;}
        .sensor-item .temp{color:#ff6347;} .sensor-item .humi{color:#1e90ff;} .sensor-item .soil{color:#8B4513;} .sensor-item .light{color:#FFD700;}
        .sensor-value{font-size:1.5em;font-weight:bold;margin-top:5px;}
        .sensor-alert{margin-top:15px;padding:10px;border-radius:8px;background-color:#fff3cd;color:#856404;text-align:center;display:none;}
        .control-grid, .device-grid{display:flex;flex-direction:column;gap:15px;}
        .control-item{display:flex;align-items:center;padding:10px;border-radius:8px;background:var(--secondary-color);}
        .device-item{display:flex;align-items:center;justify-content:space-between;}
        .device-info{display:flex;align-items:center;gap:10px;}
        .device-status{font-weight:bold;margin-left:auto;margin-right:15px;}
        button.ctrl{padding:8px 16px;border:none;border-radius:5px;cursor:pointer;font-weight:bold;color:white;transition:background-color 0.2s;}
        button.on{background-color:#28a745;} button.off{background-color:#f44336;}
        .toggle-switch{position:relative;display:inline-block;width:50px;height:28px;margin-left:auto;}
        .toggle-switch input{opacity:0;width:0;height:0;}
        .slider{position:absolute;cursor:pointer;top:0;left:0;right:0;bottom:0;background-color:#ccc;transition:.4s;border-radius:28px;}
        .slider:before{position:absolute;content:"";height:20px;width:20px;left:4px;bottom:4px;background-color:white;transition:.4s;border-radius:50%;}
        input:checked+.slider{background-color:var(--primary-color);} input:checked+.slider:before{transform:translateX(22px);}
        #mode-text{font-weight:bold;}
        form .form-grid{display:grid;grid-template-columns:1fr 1fr;gap:10px;}
        form label{display:flex;flex-direction:column;font-size:0.9em;color:#555;}
        form input{width:100%;padding:8px;box-sizing:border-box;border:1px solid #ddd;border-radius:5px;margin-top:4px;}
        form button{width:100%;padding:10px;margin-top:15px;background:var(--primary-color);color:white;border:none;border-radius:5px;cursor:pointer;font-size:1em;font-weight:bold;}
        @media (max-width:900px){.grid{grid-template-columns:1fr;}}
        @media (max-width:500px){.sensor-grid{grid-template-columns:1fr;}}
    </style>
</head>
<body>
    <div class="container">
        <header><h1><i class="fas fa-seedling"></i> Smart Greenhouse</h1></header>
        <main class="grid">
            <div class="main-column">
                <div class="card">
                    <h2 class="card-title"><i class="fas fa-tachometer-alt"></i> Cảm biến</h2>
                    <div class="sensor-grid">
                        <div class="sensor-item"><i class="fas fa-thermometer-half icon temp"></i><div>Nhiệt độ</div><div class="sensor-value" id="temp">-- &deg;C</div></div>
                        <div class="sensor-item"><i class="fas fa-tint icon humi"></i><div>Độ ẩm KK</div><div class="sensor-value" id="humi">-- %</div></div>
                        <div class="sensor-item"><i class="fas fa-water icon soil"></i><div>Độ ẩm đất</div><div class="sensor-value" id="soil">--</div></div>
                        <div class="sensor-item"><i class="fas fa-sun icon light"></i><div>Ánh sáng</div><div class="sensor-value" id="lightLux">-- lx</div></div>
                        <div class="sensor-item"><i class="fas fa-wind icon co2"></i><div>CO2</div><div class="sensor-value" id="co2">-- ppm</div></div>
                    </div>
                    <div class="sensor-alert" id="lightMsg"></div>
                </div>
                <div class="card">
                    <h2 class="card-title"><i class="fas fa-toggle-on"></i> Điều Khiển</h2>
                    <div class="control-item"><span id="mode-text">TỰ ĐỘNG</span><div class="toggle-switch"><input type="checkbox" id="mode-toggle"><label for="mode-toggle" class="slider"></label></div></div>
                </div>
                <div class="card">
                    <h2 class="card-title"><i class="fas fa-cogs"></i> Thiết bị</h2>
                    <div class="device-grid">
                        <div class="device-item"><div class="device-info"><i class="fas fa-fan icon"></i> Quạt</div> <span id="fan-status">TẮT</span> <div><button class="ctrl on" onclick="controlDevice('fan','on')">Bật</button><button class="ctrl off" onclick="controlDevice('fan','off')">Tắt</button></div></div>
                        <div class="device-item"><div class="device-info"><i class="fas fa-water icon"></i> Bơm</div> <span id="pump-status">TẮT</span> <div><button class="ctrl on" onclick="controlDevice('pump','on')">Bật</button><button class="ctrl off" onclick="controlDevice('pump','off')">Tắt</button></div></div>
                        <div class="device-item"><div class="device-info"><i class="far fa-lightbulb icon"></i> Đèn</div> <span id="light-status">TẮT</span> <div><button class="ctrl on" onclick="controlDevice('light','on')">Bật</button><button class="ctrl off" onclick="controlDevice('light','off')">Tắt</button></div></div>
                    </div>
                </div>
            </div>
            <div class="side-column">
                <div class="card">
                    <h2 class="card-title"><i class="fas fa-sliders"></i> Ngưỡng cảm biến</h2>
                    <form onsubmit="saveThresholds(event)">
                        <div class="form-grid">
                            <label>Nhiệt độ cao<input type="number" id="tempHigh" step="0.1"></label><label>Nhiệt độ thấp<input type="number" id="tempLow" step="0.1"></label>
                            <label>Đất khô<input type="number" id="soilDry"></label><label>Đất ướt<input type="number" id="soilWet"></label>
                            <label>Sáng yếu<input type="number" id="lightDark"></label><label>Sáng mạnh<input type="number" id="lightBright"></label>
                            <label>CO2 cao<input type="number" id="co2High"></label><label>CO2 thấp<input type="number" id="co2Low"></label>
                        </div>
                        <button type="submit">Lưu</button>
                    </form>
                </div>
                <div class="card">
                    <h2 class="card-title"><i class="fas fa-leaf"></i> Loại cây trồng</h2>
                    <input type="text" id="plantInput" placeholder="Nhập tên loại cây..." oninput="setPlant()" style="width:100%; padding: 8px; box-sizing: border-box;">
                </div>
                <div class="card">
                    <h2 class="card-title"><i class="fas fa-robot"></i> Lệnh từ AI</h2><div id="aiStatus">Chờ lệnh...</div>
                </div>
                <div class="card">
                    <h2 class="card-title"><i class="fas fa-history"></i> Lịch sử dữ liệu</h2><button onclick="window.location.href='/history'" style="width:100%; padding:10px;">Xem chi tiết</button>
                </div>
            </div>
        </main>
    </div>
    <script>
        function setDeviceStatus(id,state){const el=document.getElementById(id);if(state){el.textContent='BẬT';el.style.color='var(--primary-color)';}else{el.textContent='TẮT';el.style.color='#f44336';}}
        async function updateData(){const r=await fetch('/data');const d=await r.json();
            document.getElementById('temp').innerHTML=`${d.temperature.toFixed(1)} &deg;C`;
            document.getElementById('humi').textContent=`${d.humidity.toFixed(1)} %`;
            document.getElementById('soil').textContent=d.soilMoisture;
            document.getElementById('lightLux').textContent=`${d.lightLevel.toFixed(1)} lx`;
            document.getElementById('co2').textContent=`${d.co2Level} ppm`;
            const alertEl=document.getElementById('lightMsg');
            if(d.lightMessage&&d.lightMessage.length>0){alertEl.textContent=d.lightMessage;alertEl.style.display='block';}else{alertEl.style.display='none';}
            setDeviceStatus('fan-status',d.fan); setDeviceStatus('pump-status',d.pump); setDeviceStatus('light-status',d.light);
            document.getElementById('mode-toggle').checked=d.autoMode;
            document.getElementById('mode-text').textContent=d.autoMode?'TỰ ĐỘNG':'THỦ CÔNG';
            document.getElementById('aiStatus').textContent=d.aiCommand;
        }
        function setMode(){const isAuto=document.getElementById('mode-toggle').checked;fetch(`/mode?mode=${isAuto?'auto':'manual'}`).then(()=>setTimeout(updateData,200));}
        function controlDevice(device,action){fetch(`/device?device=${device}&action=${action}`).then(()=>setTimeout(updateData,200));}
        async function loadPlant(){const r=await fetch('/api/plant');const d=await r.json();document.getElementById('plantInput').value=d.plant;}
        function setPlant(){const plant=document.getElementById('plantInput').value.trim();if(plant){fetch('/api/plant',{method:'POST',body:JSON.stringify({plant:plant}),headers:{'Content-Type':'application/json'}});}}
        async function loadThresholds(){const r=await fetch('/api/thresholds');const t=await r.json();
            document.getElementById('tempHigh').value=t.tempHigh; document.getElementById('tempLow').value=t.tempLow;
            document.getElementById('soilDry').value=t.soilDry; document.getElementById('soilWet').value=t.soilWet;
            document.getElementById('lightDark').value=t.lightDark; document.getElementById('lightBright').value=t.lightBright;
            document.getElementById('co2High').value=t.co2High; document.getElementById('co2Low').value=t.co2Low;
        }
        async function saveThresholds(e){e.preventDefault();const btn=e.target.querySelector('button');btn.textContent='Đang lưu...';
            const t={tempHigh:parseFloat(document.getElementById('tempHigh').value),tempLow:parseFloat(document.getElementById('tempLow').value),soilDry:parseInt(document.getElementById('soilDry').value),soilWet:parseInt(document.getElementById('soilWet').value),lightDark:parseInt(document.getElementById('lightDark').value),lightBright:parseInt(document.getElementById('lightBright').value),co2High:parseInt(document.getElementById('co2High').value),co2Low:parseInt(document.getElementById('co2Low').value)};
            await fetch('/api/thresholds',{method:'POST',body:JSON.stringify(t),headers:{'Content-Type':'application/json'}});
            btn.textContent='Đã lưu!';setTimeout(()=>btn.textContent='Lưu',2000);
        }
        document.addEventListener('DOMContentLoaded',()=>{
            document.getElementById('mode-toggle').addEventListener('change',setMode);
            updateData();loadPlant();loadThresholds();setInterval(updateData,3000);
        });
    </script>
</body></html>
)rawliteral";

#endif
