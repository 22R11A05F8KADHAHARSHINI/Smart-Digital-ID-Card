const char webpage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<title>GPS Geofence Tracker</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="stylesheet" href="https://unpkg.com/leaflet@1.9.4/dist/leaflet.css"/>
<script src="https://unpkg.com/leaflet@1.9.4/dist/leaflet.js"></script>
<style>
    body { font-family: Arial, sans-serif; margin: 0; padding: 0; background-color: #f4f4f4; }
    #map { height: 400px; width: 100%; border-radius: 8px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }
    .container { padding: 20px; max-width: 900px; margin: auto; }
    .status-panel { display: flex; flex-wrap: wrap; justify-content: space-between; margin-bottom: 20px; }
    .status-box { background: #ffffff; padding: 10px; border-radius: 8px; flex-grow: 1; margin: 5px; text-align: center; box-shadow: 0 2px 4px rgba(0,0,0,0.05); }
    .status-label { font-size: 0.8em; color: #555; }
    .status-value { font-size: 1.2em; font-weight: bold; color: #333; margin-top: 5px; }
    .status-green { color: #28a745; }
    .status-red { color: #dc3545; }
    .status-yellow { color: #ffc107; }
    .status-blue { color: #007bff; }
</style>
</head>
<body>

<div class="container">
    <h2>Geofence Tracker (ESP8266)</h2>

    <div class="status-panel">
        <div class="status-box">
            <div class="status-label">Device Status</div>
            <div class="status-value status-green" id="deviceStatus">ONLINE</div>
        </div>
        <div class="status-box">
            <div class="status-label">Geofence Status</div>
            <div class="status-value" id="fenceStatus">...</div>
        </div>
        <div class="status-box">
            <div class="status-label">Satellites</div>
            <div class="status-value" id="satsValue">...</div>
        </div>
        <div class="status-box">
            <div class="status-label">Last Update</div>
            <div class="status-value" id="dateValue">...</div>
        </div>
    </div>

    <div id="map"></div>

    <div class="status-panel" style="margin-top: 20px;">
        <div class="status-box">
            <div class="status-label">Latitude</div>
            <div class="status-value" id="latValue">...</div>
        </div>
        <div class="status-box">
            <div class="status-label">Longitude</div>
            <div class="status-value" id="lonValue">...</div>
        </div>
        <div class="status-box">
            <div class="status-label">Angle (>180=Inside)</div>
            <div class="status-value" id="angleValue">...</div>
        </div>
    </div>
</div>

<script>
    // Initial coordinates (Accurate center of Geethanjali College)
    var initialLat = 17.520550; 
    var initialLon = 78.631027; 
    
    // Initialize Map and set view
    var map = L.map('map').setView([initialLat, initialLon], 16);

    // --- SATELLITE TILE LAYER ---
    var satelliteLayer = L.tileLayer(
        'https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}', {
        maxZoom: 19,
        attribution: 'Tiles © Esri — Source: Esri, i-cubed, USDA, USGS, AEX, GeoEye, Getmapping, Aerogrid, IGN, IGP, swisstopo, and the GIS User Community'
    }).addTo(map);

    // Define the geofence polygon (must match the coordinates in Final_mini.ino)
    var geofenceCoords = [
        [17.521800, 78.632200],
        [17.521500, 78.630000],
        [17.520500, 78.629500],
        [17.519000, 78.630000],
        [17.518800, 78.631000],
        [17.519200, 78.631800],
        [17.520000, 78.632500],
        [17.521000, 78.632800],
        [17.521500, 78.632500],
        [17.521800, 78.632000]
    ];
    
    // Draw the geofence polygon on the map
    var polygon = L.polygon(geofenceCoords, {color: 'blue', fillColor: '#00f', fillOpacity: 0.2, weight: 3}).addTo(map);
    
    // Create a marker for the device's current location
    var deviceMarker = L.circleMarker([initialLat, initialLon], {
        color: 'red',
        fillColor: '#f03',
        fillOpacity: 0.8,
        radius: 8
    }).addTo(map);

    function fetchGpsData() {
        var xhr = new XMLHttpRequest();
        xhr.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
                var data = this.responseText.split("#"); 
                // data = [SATS, DATE_TIME, LAT, LONG, FENCE_STATUS, ANGLE, DEVICE_STATUS_STRING (always ONLINE)]
                
                var sats = data[0];
                var dateTime = data[1];
                var lat = parseFloat(data[2]);
                var lon = parseFloat(data[3]);
                var fenceStatus = data[4];
                var angle = data[5];
                var deviceStatusStr = data[6];

                // 1. Update Map Marker
                if (lat != 0.0 && lon != 0.0) {
                    var newLatLng = new L.LatLng(lat, lon);
                    deviceMarker.setLatLng(newLatLng);
                }

                // 2. Update Status Boxes
                document.getElementById('satsValue').innerHTML = sats;
                document.getElementById('dateValue').innerHTML = dateTime;
                document.getElementById('latValue').innerHTML = lat.toFixed(6);
                document.getElementById('lonValue').innerHTML = lon.toFixed(6);
                document.getElementById('angleValue').innerHTML = angle.substring(0, angle.indexOf('.') + 4);

                // 3. Update Device Status (Always ONLINE and GREEN)
                var dsBox = document.getElementById('deviceStatus');
                dsBox.innerHTML = deviceStatusStr; 
                // *** FIX: Explicitly ensure the green class is reapplied on update ***
                dsBox.className = 'status-value status-green';
                
                // 4. Update Geofence Status 
                var fsBox = document.getElementById('fenceStatus');
                fsBox.innerHTML = fenceStatus.toUpperCase();
                fsBox.className = 'status-value ' + (fenceStatus === 'inside' ? 'status-green' : 'status-yellow');

            }
        };
        xhr.open("GET", "/values", true);
        xhr.send();
    }

    // Fetch data every 2 seconds
    setInterval(fetchGpsData, 2000);
    
    // Initial fetch on page load
    fetchGpsData();
</script>

</body>
</html>
)=====";