import time
import json
import datetime
import threading
import logging
from collections import deque

# Flask for web server
from flask import Flask, render_template, jsonify, request, send_from_directory

# MQTT for ESP8266 communication
import paho.mqtt.client as mqtt

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler("plant_care.log"),
        logging.StreamHandler()
    ]
)
logger = logging.getLogger(__name__)

# Flask app configuration
app = Flask(__name__, static_folder='static')

# Global data storage
sensor_data = {
    "moisture": 50,
    "temperature": 22.5,
    "humidity": 55,
    "waterLevel": 75,
    "pumpStatus": "OFF",
    "tempAction": "Optimal",
    "humidityAction": "Optimal",
    "waterLevelStatus": "OK",
    "lastUpdate": None
}

# Store hourly data for charts and statistics
# Using deque with max length to limit memory usage
hourly_data = deque(maxlen=72)  # Store up to 72 hours (3 days)

# MQTT Configuration
MQTT_BROKER = "localhost"  # Use ESP8266 IP if MQTT broker is on ESP
MQTT_PORT = 1883
MQTT_USERNAME = "pi"  # Set as needed
MQTT_PASSWORD = "raspberry"  # Set as needed
MQTT_CLIENT_ID = "raspi_plant_care"
MQTT_TOPIC_SENSOR = "plant_care/sensors"  # Topic where ESP8266 publishes data
MQTT_TOPIC_CONTROL = "plant_care/control"  # Topic where we send commands to ESP8266

# Connect to MQTT broker
mqtt_client = None

# Time windows for aggregation
STATS_INTERVAL = 3600  # 1 hour in seconds
last_stats_time = 0

def on_mqtt_connect(client, userdata, flags, rc):
    """Callback for when MQTT connection is established"""
    if rc == 0:
        logger.info("Connected to MQTT broker")
        # Subscribe to sensor data topic
        client.subscribe(MQTT_TOPIC_SENSOR)
    else:
        logger.error(f"Failed to connect to MQTT broker with code: {rc}")

def on_mqtt_message(client, userdata, msg):
    """Callback for when a message is received from MQTT"""
    try:
        payload = json.loads(msg.payload.decode('utf-8'))
        logger.debug(f"Received MQTT message: {payload}")
        
        # Update our global sensor data
        if msg.topic == MQTT_TOPIC_SENSOR:
            update_sensor_data(payload)
    except Exception as e:
        logger.error(f"Error processing MQTT message: {e}")

def update_sensor_data(data):
    """Update sensor data and calculate derived values"""
    global sensor_data, last_stats_time
    
    # Update timestamp
    sensor_data["lastUpdate"] = datetime.datetime.now().isoformat()
    
    # Update raw sensor values
    if "moisture" in data:
        sensor_data["moisture"] = data["moisture"]
    if "temperature" in data:
        sensor_data["temperature"] = data["temperature"]
    if "humidity" in data:
        sensor_data["humidity"] = data["humidity"]
    if "waterLevel" in data:
        sensor_data["waterLevel"] = data["waterLevel"]
    if "pumpStatus" in data:
        sensor_data["pumpStatus"] = data["pumpStatus"]
    
    # Calculate derived values
    # Temperature status
    if sensor_data["temperature"] < 20:
        sensor_data["tempAction"] = "Low"
    elif sensor_data["temperature"] > 30:
        sensor_data["tempAction"] = "High"
    else:
        sensor_data["tempAction"] = "Optimal"
    
    # Humidity status
    if sensor_data["humidity"] < 40:
        sensor_data["humidityAction"] = "Low"
    elif sensor_data["humidity"] > 70:
        sensor_data["humidityAction"] = "High"
    else:
        sensor_data["humidityAction"] = "Optimal"
    
    # Water level status
    if sensor_data["waterLevel"] < 20:
        sensor_data["waterLevelStatus"] = "Critical"
    elif sensor_data["waterLevel"] < 40:
        sensor_data["waterLevelStatus"] = "Low"
    else:
        sensor_data["waterLevelStatus"] = "OK"
    
    # Check if it's time to save hourly stats
    current_time = time.time()
    if current_time - last_stats_time >= STATS_INTERVAL:
        save_hourly_stats()
        last_stats_time = current_time

def save_hourly_stats():
    """Save current sensor readings as hourly statistics"""
    try:
        now = datetime.datetime.now()
        hourly_entry = {
            "timestamp": now.isoformat(),
            "fullDate": now.date().isoformat(),
            "hourLabel": now.strftime("%H:00"),
            "avgMoisture": sensor_data["moisture"],
            "avgTemp": sensor_data["temperature"],
            "avgHumidity": sensor_data["humidity"],
            "avgWater": sensor_data["waterLevel"],
            "pumpActivations": 1 if sensor_data["pumpStatus"] == "ON" else 0,
            "moistureTrend": "stable",  # Will be calculated when serving multiple data points
            "tempTrend": "stable",
            "humidityTrend": "stable",
            "waterTrend": "stable"
        }
        
        # Add to our hourly data storage
        hourly_data.append(hourly_entry)
        logger.info(f"Saved hourly stats: {hourly_entry['hourLabel']}")
        
        # Save to disk as backup
        save_data_to_disk()
    except Exception as e:
        logger.error(f"Error saving hourly stats: {e}")

def save_data_to_disk():
    """Save current data to disk as backup"""
    try:
        with open('sensor_data.json', 'w') as f:
            json.dump(sensor_data, f)
        
        with open('hourly_data.json', 'w') as f:
            json.dump(list(hourly_data), f)
            
        logger.debug("Data saved to disk")
    except Exception as e:
        logger.error(f"Error saving data to disk: {e}")

def load_data_from_disk():
    """Load saved data from disk"""
    global sensor_data, hourly_data
    
    try:
        try:
            with open('sensor_data.json', 'r') as f:
                sensor_data.update(json.load(f))
            logger.info("Loaded sensor data from disk")
        except (FileNotFoundError, json.JSONDecodeError):
            logger.warning("No valid sensor data found on disk")
        
        try:
            with open('hourly_data.json', 'r') as f:
                loaded_data = json.load(f)
                hourly_data = deque(loaded_data, maxlen=72)
            logger.info(f"Loaded {len(hourly_data)} hourly data points from disk")
        except (FileNotFoundError, json.JSONDecodeError):
            logger.warning("No valid hourly data found on disk")
    except Exception as e:
        logger.error(f"Error loading data from disk: {e}")

def setup_mqtt():
    """Initialize MQTT client and connection"""
    global mqtt_client
    
    try:
        # Create MQTT client
        mqtt_client = mqtt.Client(client_id=MQTT_CLIENT_ID)
        mqtt_client.username_pw_set(username=MQTT_USERNAME, password=MQTT_PASSWORD)
        
        # Set callbacks
        mqtt_client.on_connect = on_mqtt_connect
        mqtt_client.on_message = on_mqtt_message
        
        # Connect to broker
        logger.info(f"Connecting to MQTT broker at {MQTT_BROKER}:{MQTT_PORT}")
        mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
        
        # Start MQTT loop in background thread
        mqtt_client.loop_start()
    except Exception as e:
        logger.error(f"Error setting up MQTT: {e}")
        # Continue execution - we'll retry connection later

def control_pump(state):
    """Send command to ESP8266 to control the water pump"""
    if mqtt_client and mqtt_client.is_connected():
        try:
            # Create command payload
            command = {
                "device": "pump",
                "action": state
            }
            
            # Publish command to MQTT topic
            mqtt_client.publish(MQTT_TOPIC_CONTROL, json.dumps(command))
            logger.info(f"Sent pump control command: {state}")
            
            # Update our local state immediately for UI responsiveness
            # (ESP8266 will send back actual state via sensor topic)
            sensor_data["pumpStatus"] = state
            
            return True
        except Exception as e:
            logger.error(f"Error sending pump control command: {e}")
            return False
    else:
        logger.error("MQTT client not connected, cannot send command")
        return False

# Flask routes
@app.route('/')
def index():
    """Serve the main HTML page"""
    return send_from_directory(app.static_folder, 'index.html')

@app.route('/api/sensor-data')
def get_sensor_data():
    """API endpoint to get current sensor data"""
    return jsonify(sensor_data)

@app.route('/api/stats-data')
def get_stats_data():
    """API endpoint to get hourly/daily statistics"""
    # Return the last 24 hours of data by default
    hours = request.args.get('hours', 24, type=int)
    
    # Limit to available data
    data_to_return = list(hourly_data)[-hours:] if len(hourly_data) > 0 else []
    
    # Calculate trends for consecutive entries
    for i in range(1, len(data_to_return)):
        prev = data_to_return[i-1]
        curr = data_to_return[i]
        
        # Moisture trend
        if curr["avgMoisture"] > prev["avgMoisture"]:
            curr["moistureTrend"] = "up"
        elif curr["avgMoisture"] < prev["avgMoisture"]:
            curr["moistureTrend"] = "down"
        
        # Temperature trend
        if curr["avgTemp"] > prev["avgTemp"]:
            curr["tempTrend"] = "up"
        elif curr["avgTemp"] < prev["avgTemp"]:
            curr["tempTrend"] = "down"
        
        # Humidity trend
        if curr["avgHumidity"] > prev["avgHumidity"]:
            curr["humidityTrend"] = "up"
        elif curr["avgHumidity"] < prev["avgHumidity"]:
            curr["humidityTrend"] = "down"
        
        # Water level trend
        if curr["avgWater"] > prev["avgWater"]:
            curr["waterTrend"] = "up"
        elif curr["avgWater"] < prev["avgWater"]:
            curr["waterTrend"] = "down"
    
    return jsonify(data_to_return)

@app.route('/api/control', methods=['POST'])
def control_device():
    """API endpoint to control devices (e.g., water pump)"""
    try:
        action = request.json
        if not action or "device" not in action or "action" not in action:
            return jsonify({"success": False, "error": "Invalid request format"}), 400
        
        device = action["device"]
        state = action["action"]
        
        if device == "pump":
            if state in ["ON", "OFF"]:
                if control_pump(state):
                    return jsonify({"success": True, "message": f"Pump turned {state}"})
                else:
                    return jsonify({"success": False, "error": "Failed to control pump"}), 500
            else:
                return jsonify({"success": False, "error": "Invalid pump state"}), 400
        else:
            return jsonify({"success": False, "error": "Unknown device"}), 400
    except Exception as e:
        logger.error(f"Error in control API: {e}")
        return jsonify({"success": False, "error": str(e)}), 500

@app.route('/api/placeholder/<int:width>/<int:height>')
def placeholder_image(width, height):
    """Return a placeholder SVG image to satisfy frontend placeholder requests"""
    svg = f'''
    <svg width="{width}" height="{height}" xmlns="http://www.w3.org/2000/svg">
        <rect width="100%" height="100%" fill="#e0e0e0"/>
        <text x="50%" y="50%" font-family="Arial" font-size="14" fill="#333333" 
              text-anchor="middle" dominant-baseline="middle">
            {width} × {height}
        </text>
    </svg>
    '''
    return svg, 200, {'Content-Type': 'image/svg+xml'}

# Function to simulate sensor data for testing without ESP8266
def simulate_sensor_data():
    """Generate simulated sensor data at regular intervals for testing"""
    while True:
        try:
            # Generate random changes to sensor values
            moisture = max(0, min(100, sensor_data["moisture"] + (((time.time() % 60) / 30 - 1) * 2)))
            temperature = max(15, min(35, sensor_data["temperature"] + ((time.time() % 120) / 60 - 1) * 0.2))
            humidity = max(30, min(85, sensor_data["humidity"] + ((time.time() % 180) / 90 - 1) * 2))
            water_level = max(0, min(100, sensor_data["waterLevel"] - 0.02))  # Slowly decreasing
            
            # Update pump status based on moisture level (automatic control simulation)
            pump_status = "ON" if moisture < 40 and sensor_data["pumpStatus"] != "ON" else \
                         "OFF" if moisture > 65 and sensor_data["pumpStatus"] == "ON" else \
                         sensor_data["pumpStatus"]
            
            # Apply pump effect on moisture if pump is on
            if pump_status == "ON":
                moisture = min(100, moisture + 0.5)
            
            # Create simulated data packet
            simulated_data = {
                "moisture": moisture,
                "temperature": temperature,
                "humidity": humidity,
                "waterLevel": water_level,
                "pumpStatus": pump_status
            }
            
            # Update sensor data as if it came from MQTT
            update_sensor_data(simulated_data)
            
            # Sleep for next update
            time.sleep(2)
        except Exception as e:
            logger.error(f"Error in simulation thread: {e}")
            time.sleep(5)  # Longer sleep on error

def copy_html_to_static():
    """Copy the HTML file to the static folder for serving"""
    import os
    import shutil
    
    # Create static folder if it doesn't exist
    if not os.path.exists(app.static_folder):
        os.makedirs(app.static_folder)
    
    # Copy HTML file to static folder
    try:
        with open('index.html', 'r') as src, open(os.path.join(app.static_folder, 'index.html'), 'w') as dst:
            dst.write(src.read())
        logger.info("Copied index.html to static folder")
    except Exception as e:
        logger.error(f"Error copying HTML file: {e}")

def main():
    """Main function to start the server"""
    # Load saved data
    load_data_from_disk()
    
    # Set up MQTT client
    setup_mqtt()
    
    # Copy HTML file to static folder
    copy_html_to_static()
    
    # Start simulation thread for testing (comment out when using real ESP8266)
    # simulation_thread = threading.Thread(target=simulate_sensor_data, daemon=True)
    # simulation_thread.start()
    # logger.info("Started simulation thread")
    
    # Start auto-save thread
    def auto_save():
        while True:
            time.sleep(300)  # Save every 5 minutes
            save_data_to_disk()
    
    save_thread = threading.Thread(target=auto_save, daemon=True)
    save_thread.start()
    
    # Start Flask server
    logger.info("Starting Flask server")
    app.run(host='0.0.0.0', port=80, debug=False)

if __name__ == "__main__":
    main()