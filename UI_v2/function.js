 // Global variables
 let receivedRealData = false;
 let mqttClient;
 let moistureChart, tempChart, humidityChart;
 const maxDataPoints = 20; // Number of data points to keep in memory
 let sensorData = {
     moisture: [],
     temperature: [],
     humidity: [],
     waterLevel: [],
     timestamps: []
 };

 // Initialize when DOM is loaded
 document.addEventListener('DOMContentLoaded', function() {
     initCharts();
     connectToMQTT();
     showPage('dashboard');
     // Simulate initial data update (can be removed in production with real MQTT data)
     simulateData();
     // Set interval for weekly stats update to every hour
     setInterval(updateWeeklyStats, 3600000); // 3600000 milliseconds = 1 hour
 });

 // Initialize Chart.js charts
 function initCharts() {
     // Moisture Chart
     const moistureCtx = document.getElementById('moistureChart').getContext('2d');
     moistureChart = new Chart(moistureCtx, {
         type: 'line',
         data: {
             labels: [],
             datasets: [{
                 label: 'Soil Moisture (%)',
                 data: [],
                 borderColor: '#10b981',
                 backgroundColor: 'rgba(16, 185, 129, 0.1)',
                 tension: 0.3,
                 fill: true
             }]
         },
         options: getChartOptions('Moisture (%)')
     });

     // Temperature Chart
     const tempCtx = document.getElementById('tempChart').getContext('2d');
     tempChart = new Chart(tempCtx, {
         type: 'line',
         data: {
             labels: [],
             datasets: [{
                 label: 'Temperature (°C)',
                 data: [],
                 borderColor: '#FF9800',
                 backgroundColor: 'rgba(255, 152, 0, 0.1)',
                 tension: 0.3,
                 fill: true
             }]
         },
         options: getChartOptions('Temperature (°C)')
     });

     // Humidity Chart
     const humidityCtx = document.getElementById('humidityChart').getContext('2d');
     humidityChart = new Chart(humidityCtx, {
         type: 'line',
         data: {
             labels: [],
             datasets: [{
                 label: 'Humidity (%)',
                 data: [],
                 borderColor: '#2196F3',
                 backgroundColor: 'rgba(33, 150, 243, 0.1)',
                 tension: 0.3,
                 fill: true
             }]
         },
         options: getChartOptions('Humidity (%)')
     });
 }

 // Common chart options
 function getChartOptions(title) {
     return {
         responsive: true,
         maintainAspectRatio: false,
         plugins: {
             legend: {
                 position: 'top',
             },
             title: {
                 display: true,
                 text: title
             }
         },
         scales: {
             x: {
                 grid: {
                     display: false
                 }
             },
             y: {
                 grid: {
                     color: 'rgba(0, 0, 0, 0.05)'
                 }
             }
         }
     };
 }

 // Connect to MQTT broker
 function connectToMQTT() {
     // Use the same MQTT broker settings as in your ESP8266 code
     const options = {
        host: 'broker.hivemq.com', // Should match ESP8266
        port: 8000, // WebSocket port, ESP8266 uses 1883 for direct TCP
        path: '/mqtt',
        clientId: 'plantcare-web-' + Math.random().toString(16).substr(2, 8),
        protocol: 'wss'
    };

    mqttClient = mqtt.connect(options);

    mqttClient.on('connect', function() {
        console.log('Connected to MQTT');
        document.getElementById('connection-status').textContent = 'Connected to MQTT';
        // Subscribe to the same topics as published by ESP8266
        mqttClient.subscribe('plantcare/sensors'); 
        mqttClient.subscribe('plantcare/actuators/status');
    });

     mqttClient.on('message', function(topic, message) {
         const data = JSON.parse(message.toString());
         console.log('Received:', data);

         // Update UI with new data
         updateSensorData(data);
         updateCharts(data);

         // Check for alerts
         checkAlerts(data);

         // Auto-control pump if moisture is low
         if (data.moisture !== undefined && data.moisture < 40) {
             controlPump('ON');
         }
     });

     mqttClient.on('error', function(err) {
         console.error('MQTT Error:', err);
         document.getElementById('connection-status').textContent = 'MQTT Error: ' + err.message;
     });

     mqttClient.on('close', function() {
         console.log('MQTT Connection closed');
         document.getElementById('connection-status').textContent = 'Disconnected from MQTT';
     });
 }

 // Update sensor data arrays
 function updateSensorData(data) {
     const now = new Date();
     const timestamp = now.toLocaleTimeString();

     // Add new data (limit to maxDataPoints)
     if (data.moisture !== undefined) {
         sensorData.moisture.push(data.moisture);
         if (sensorData.moisture.length > maxDataPoints) sensorData.moisture.shift();
     }

     if (data.temperature !== undefined) {
         sensorData.temperature.push(data.temperature);
         if (sensorData.temperature.length > maxDataPoints) sensorData.temperature.shift();
     }

     if (data.humidity !== undefined) {
         sensorData.humidity.push(data.humidity);
         if (sensorData.humidity.length > maxDataPoints) sensorData.humidity.shift();
     }

     if (data.waterLevel !== undefined) {
         sensorData.waterLevel.push(data.waterLevel);
         if (sensorData.waterLevel.length > maxDataPoints) sensorData.waterLevel.shift();
     }

     sensorData.timestamps.push(timestamp);
     if (sensorData.timestamps.length > maxDataPoints) sensorData.timestamps.shift();

     // Update UI
     updateUI(data);
 }

 // Update charts with new data
 function updateCharts(data) {
     // Update moisture chart
     moistureChart.data.labels = sensorData.timestamps;
     moistureChart.data.datasets[0].data = sensorData.moisture;
     moistureChart.update();

     // Update temperature chart
     tempChart.data.labels = sensorData.timestamps;
     tempChart.data.datasets[0].data = sensorData.temperature;
     tempChart.update();

     // Update humidity chart
     humidityChart.data.labels = sensorData.timestamps;
     humidityChart.data.datasets[0].data = sensorData.humidity;
     humidityChart.update();
 }

 // Update the weekly stats table
 function updateWeeklyStats() {
     // In a real implementation, you would get this from a database or API
     // For demo purposes, we'll simulate some weekly data
     const days = ['Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday', 'Sunday'];
     const moistureTableBody = document.getElementById('moisture-table-body');
     const tempTableBody = document.getElementById('temp-table-body');
     const humidityTableBody = document.getElementById('humidity-table-body');

     moistureTableBody.innerHTML = '';
     tempTableBody.innerHTML = '';
     humidityTableBody.innerHTML = '';

     days.forEach(day => {
         // Moisture Table
         const moistureRow = document.createElement('tr');
         const moistureDayCell = document.createElement('td');
         moistureDayCell.textContent = day;
         moistureRow.appendChild(moistureDayCell);
         const avgMoistureCell = document.createElement('td');
         const avgMoistureValue = Math.round(40 + Math.random() * 40); // Random between 40-80
         avgMoistureCell.textContent = avgMoistureValue + '%';
         moistureRow.appendChild(avgMoistureCell);
         const pumpCell = document.createElement('td');
         const pumpCount = avgMoistureValue < 50 ? Math.floor(1 + Math.random() * 3) : 0;
         pumpCell.textContent = pumpCount;
         moistureRow.appendChild(pumpCell);
         const moistureTrendCell = document.createElement('td');
         const moistureTrendSpan = document.createElement('span');
         moistureTrendSpan.className = 'trend-indicator';
         if (Math.random() > 0.5) {
             moistureTrendSpan.classList.add('trend-up');
             moistureTrendSpan.textContent = '↑ ' + Math.floor(1 + Math.random() * 20) + '%';
         } else {
             moistureTrendSpan.classList.add('trend-down');
             moistureTrendSpan.textContent = '↓ ' + Math.floor(1 + Math.random() * 15) + '%';
         }
         moistureTrendCell.appendChild(moistureTrendSpan);
         moistureRow.appendChild(moistureTrendCell);
         moistureTableBody.appendChild(moistureRow);

         // Temperature Table
         const tempRow = document.createElement('tr');
         const tempDayCell = document.createElement('td');
         tempDayCell.textContent = day;
         tempRow.appendChild(tempDayCell);
         const avgTempCell = document.createElement('td');
         const avgTempValue = Math.round(20 + Math.random() * 15); // Random between 20-35
         avgTempCell.textContent = avgTempValue + '°C';
         tempRow.appendChild(avgTempCell);
         const tempTrendCell = document.createElement('td');
         const tempTrendSpan = document.createElement('span');
         tempTrendSpan.className = 'trend-indicator';
         if (Math.random() > 0.5) {
             tempTrendSpan.classList.add('trend-up');
             tempTrendSpan.textContent = '↑ ' + Math.floor(1 + Math.random() * 5) + '°C';
         } else {
             tempTrendSpan.classList.add('trend-down');
             tempTrendSpan.textContent = '↓ ' + Math.floor(1 + Math.random() * 5) + '°C';
         }
         tempTrendCell.appendChild(tempTrendSpan);
         tempRow.appendChild(tempTrendCell);
         tempTableBody.appendChild(tempRow);

         // Humidity Table
         const humidityRow = document.createElement('tr');
         const humidityDayCell = document.createElement('td');
         humidityDayCell.textContent = day;
         humidityRow.appendChild(humidityDayCell);
         const avgHumidityCell = document.createElement('td');
         const avgHumidityValue = Math.round(40 + Math.random() * 30); // Random between 40-70
         avgHumidityCell.textContent = avgHumidityValue + '%';
         humidityRow.appendChild(avgHumidityCell);
         const humidityTrendCell = document.createElement('td');
         const humidityTrendSpan = document.createElement('span');
         humidityTrendSpan.className = 'trend-indicator';
         if (Math.random() > 0.5) {
             humidityTrendSpan.classList.add('trend-up');
             humidityTrendSpan.textContent = '↑ ' + Math.floor(1 + Math.random() * 10) + '%';
         } else {
             humidityTrendSpan.classList.add('trend-down');
             humidityTrendSpan.textContent = '↓ ' + Math.floor(1 + Math.random() * 10) + '%';
         }
         humidityTrendCell.appendChild(humidityTrendSpan);
         humidityRow.appendChild(humidityTrendCell);
         humidityTableBody.appendChild(humidityRow);
     });

     // Update weekly averages
     const avgTemp = sensorData.temperature.length > 0 ? Math.round(sensorData.temperature.reduce((a, b) => a + b, 0) / sensorData.temperature.length) : '--';
     const avgHumidity = sensorData.humidity.length > 0 ? Math.round(sensorData.humidity.reduce((a, b) => a + b, 0) / sensorData.humidity.length) : '--';
     document.getElementById('weekly-temp-avg').textContent = avgTemp + '°C';
     document.getElementById('weekly-humidity-avg').textContent = avgHumidity + '%';
 }

 // Control the water pump
 function controlPump(state) {
     if (!mqttClient || !mqttClient.connected) {
         alert('Not connected to MQTT broker');
         return;
     }

     const message = JSON.stringify({ pump: state });
     mqttClient.publish('plantcare/actuators', message);

     // Update UI immediately
     document.getElementById('pump-status').textContent = 'Pump Status: ' + state;
     document.getElementById('moisture-msg').textContent =
         'Pump has been turned ' + state + '. Moisture level: ' +
         (document.getElementById('moisture-text').textContent || '--');
 }

 // Check for alert conditions
 function checkAlerts(data) {
     // Moisture alerts
     const moistureAlert = document.getElementById('moisture-alert');
     if (data.moisture < 40) {
         moistureAlert.innerHTML = '<div class="alert alert-warning">Low moisture! Watering recommended.</div>';
     } else if (data.moisture > 80) {
         moistureAlert.innerHTML = '<div class="alert alert-warning">High moisture! Reduce watering.</div>';
     } else {
         moistureAlert.innerHTML = '<div class="alert alert-success">Moisture level is optimal.</div>';
     }

     // Temperature alerts
     const tempAlert = document.getElementById('temp-alert');
     if (data.temperature < 20) {
         tempAlert.innerHTML = '<div class="alert alert-warning">Low temperature! Consider warming.</div>';
     } else if (data.temperature > 30) {
         tempAlert.innerHTML = '<div class="alert alert-warning">High temperature! Cooling recommended.</div>';
     } else {
         tempAlert.innerHTML = '<div class="alert alert-success">Temperature is optimal.</div>';
     }

     // Humidity alerts
     const humidityAlert = document.getElementById('humidity-alert');
     if (data.humidity < 40) {
         humidityAlert.innerHTML = '<div class="alert alert-warning">Low humidity! Consider humidification.</div>';
     } else if (data.humidity > 70) {
         humidityAlert.innerHTML = '<div class="alert alert-warning">High humidity! Ventilation recommended.</div>';
     } else {
         humidityAlert.innerHTML = '<div class="alert alert-success">Humidity is optimal.</div>';
     }

     // Water level alerts
     const waterAlert = document.getElementById('water-alert');
     if (data.waterLevel < 20) {
         waterAlert.innerHTML = '<div class="alert alert-warning">Low water level! Refill required.</div>';
     } else if (data.waterLevel > 80) {
         waterAlert.innerHTML = '<div class="alert alert-warning">Water level is high.</div>';
     } else {
         waterAlert.innerHTML = '<div class="alert alert-success">Water level is adequate.</div>';
     }
 }

 // Update the UI with sensor data
 function updateUI(data) {
     
     // Add indicator for real vs. simulated data
     const dataSourceElement = document.getElementById('data-source');
     if (dataSourceElement) {
         dataSourceElement.textContent = data.simulated ? 'Source: Simulation' : 'Source: ESP8266 Sensor';
         dataSourceElement.className = data.simulated ? 'badge badge-warning' : 'badge badge-success';
     }

    // Moisture
     if (data.moisture !== undefined) {
         document.getElementById('moisture-text').textContent = data.moisture.toFixed(2) + '%';
         const moistureOffset = 377 - (data.moisture / 100) * 377;
         document.getElementById('moisture-circle').setAttribute('stroke-dashoffset', moistureOffset);

         const pumpStatus = data.moisture < 40 ? 'ON' : 'OFF';
         document.getElementById('pump-status').textContent = 'Pump Status: ' + pumpStatus;
         document.getElementById('moisture-msg').textContent =
             data.moisture < 40 ? 'Moisture is low. You should turn on the pump to water the plants.' :
             'Moisture is adequate. The pump can be turned off to conserve water.';
     }

     // Temperature
     if (data.temperature !== undefined) {
         document.getElementById('temp-value').textContent = data.temperature.toFixed(2);
         const tempAction = data.temperature < 20 ? 'Increase' : data.temperature > 30 ? 'Decrease' : 'Optimal';
         document.getElementById('temp-action').textContent = 'Action: ' + tempAction;
         document.getElementById('temp-msg').textContent =
             data.temperature < 20 ? 'Temperature is low for optimal plant growth. Consider increasing the temperature.' :
             data.temperature > 30 ? 'Temperature is high and may stress the plants. Consider cooling measures.' :
             'Temperature is in the optimal range for plant growth.';
     }

     // Humidity
     if (data.humidity !== undefined) {
         document.getElementById('humidity-value').textContent = data.humidity.toFixed(2);
         const humAction = data.humidity < 40 ? 'Increase' : data.humidity > 70 ? 'Decrease' : 'Optimal';
         document.getElementById('humidity-action').textContent = 'Action: ' + humAction;
         document.getElementById('humidity-msg').textContent =
             data.humidity < 40 ? 'Humidity is low. Increasing humidity will create better growing conditions.' :
             data.humidity > 70 ? 'Humidity is high and may promote mold growth. Consider reducing humidity.' :
             'Humidity is in the optimal range for plant health.';
     }

     // Water Level
     if (data.waterLevel !== undefined) {
         document.getElementById('water-level-text').textContent = data.waterLevel.toFixed(2) + '%';
         const waterOffset = 377 - (data.waterLevel / 100) * 377;
         document.getElementById('water-level-circle').setAttribute('stroke-dashoffset', waterOffset);

         const waterStatus = data.waterLevel < 20 ? 'Low' : data.waterLevel > 80 ? 'High' : 'Medium';
         document.getElementById('water-level-status').textContent = 'Level: ' + waterStatus;
         document.getElementById('water-level-msg').textContent =
             data.waterLevel < 20 ? 'Water level is critically low. Immediate refill is required to maintain the system.' :
             data.waterLevel > 80 ? 'Water level is high. No immediate action needed.' :
             'Water level is adequate. Monitor for changes.';
     }
 }

 // Page Navigation
 function showPage(pageId) {
     document.getElementById('dashboard').style.display = pageId === 'dashboard' ? 'grid' : 'none';
     document.getElementById('stats').style.display = pageId === 'stats' ? 'grid' : 'none';

     document.querySelectorAll('.nav button').forEach(btn => {
         btn.classList.toggle('active', btn.textContent.toLowerCase().includes(pageId));
     });
 }

 function showDialog(id) {
     document.getElementById(id).showModal();
 }

 function closeDialog(id) {
     document.getElementById(id).close();
 }

 // Simulate incoming data for demo purposes
 function simulateData() {
     // Only simulate if we haven't received real data yet
    if (!receivedRealData) {
        const now = new Date();
        const simulatedData = {
            moisture: Math.max(10, Math.min(90, 50 + (Math.random() * 30 - 15))),
            temperature: Math.max(15, Math.min(35, 25 + (Math.random() * 10 - 5))),
            humidity: Math.max(30, Math.min(80, 55 + (Math.random() * 20 - 10))),
            waterLevel: Math.max(10, Math.min(100, 60 + (Math.random() * 30 - 15))),
            timestamp: now.toISOString(),
            simulated: true  // Mark this data as simulated
        };

        updateSensorData(simulatedData);
        updateCharts(simulatedData);
        checkAlerts(simulatedData);

        console.log("Generated simulated data");
    }
}

// Modify the MQTT message handler to set the flag when real data is received
mqttClient.on('message', function(topic, message) {
    try {
        const data = JSON.parse(message.toString());
        console.log('Received MQTT data:', data);
        
        // Mark that we've received real data
        receivedRealData = true;
        
        // Update UI with new data
        updateSensorData(data);
        updateCharts(data);
        
        // Check for alerts
        checkAlerts(data);
        
        // Auto-control pump if moisture is low
        if (data.moisture !== undefined && data.moisture < 40) {
            controlPump('ON');
        }
    } catch (e) {
        console.error('Error parsing MQTT message:', e);
    }
});

// Modify the DOM loaded event handler - keep the simulation for initial data
document.addEventListener('DOMContentLoaded', function() {
    initCharts();
    connectToMQTT();
    showPage('dashboard');
    
    // Keep the initial simulation
    simulateData();
    
    // Set interval for weekly stats update to every hour
    setInterval(updateWeeklyStats, 3600000); // 3600000 milliseconds = 1 hour
    
    // Keep the simulation interval, but it will only generate data
    // if no real data has been received
    setInterval(simulateData, 30000); // Simulate every 30 seconds if needed
});

     updateSensorData(simulatedData);
     updateCharts(simulatedData);
     checkAlerts(simulatedData);


 setInterval(simulateData, 1800000);