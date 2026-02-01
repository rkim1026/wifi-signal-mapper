import paho.mqtt.client as mqtt
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import json
import csv
from datetime import datetime

MQTT_BROKER = "broker.emqx.io"
MQTT_TOPIC = "wifiscanner"
CSV_FILE = "wifi_data.csv"

networks = []  # Changed to list since ESP32 sends array
connected_ssid = ""
connected_rssi = 0

fig, ax = plt.subplots()

with open(CSV_FILE, "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["timestamp", "connected_ssid", "connected_rssi", "network_count", "networks_json"])

def get_color(rssi):
    if rssi > -60:
        return 'green'
    elif rssi > -75:
        return 'orange'
    return 'red'

def on_connect(client, userdata, flags, rc):
    print(f"Connected to MQTT broker {rc}")
    client.subscribe(MQTT_TOPIC + "/scan")

def on_message(client, userdata, msg):
    global networks, connected_ssid, connected_rssi
    data = json.loads(msg.payload.decode())
    
    # Set connected SSID and RSSI
    connected_ssid = data.get("connected_ssid", "")
    connected_rssi = data.get("connected_rssi", 0)
    
    # Get networks array (list of objects)
    networks = data.get("networks", [])
    
    # Write to CSV
    with open(CSV_FILE, "a", newline="") as file:
        writer = csv.writer(file)
        timestamp = datetime.now().isoformat()
        network_count = len(networks)
        networks_json = json.dumps(networks)
        writer.writerow([timestamp, connected_ssid, connected_rssi, network_count, networks_json])
    
    # Print top 3 strongest networks
    sorted_nets = sorted(networks, key=lambda x: x['rssi'], reverse=True)
    for i in range(min(3, len(sorted_nets))):
        ssid = sorted_nets[i]['ssid']
        rssi = sorted_nets[i]['rssi']
        print(f"Top {i+1}: SSID: {ssid}, RSSI: {rssi} dBm")

def animate(frame):
    ax.clear()
    if not networks:
        ax.text(0.5, 0.5, "Waiting for data...", ha='center', va='center', fontsize=14)
        return
    
    # Sort networks by RSSI
    sorted_nets = sorted(networks, key=lambda x: x['rssi'], reverse=True)
    ssids = [n['ssid'] for n in sorted_nets]
    rssis = [n['rssi'] for n in sorted_nets]
    colors = [get_color(r) for r in rssis]
    
    # Highlight connected network in blue
    for i, ssid in enumerate(ssids):
        if ssid == connected_ssid:
            colors[i] = 'blue'
    
    bars = ax.barh(ssids, rssis, color=colors)
    ax.set_xlabel('RSSI (dBm)')
    ax.set_title(f'WiFi Networks | Connected: {connected_ssid} ({connected_rssi} dBm)')
    ax.set_xlim(-100, -20)
    ax.axvline(x=-60, color='green', linestyle='--', alpha=0.5, label='Excellent')
    ax.axvline(x=-75, color='orange', linestyle='--', alpha=0.5, label='Good')
    
    for bar, rssi in zip(bars, rssis):
        ax.text(rssi + 1, bar.get_y() + bar.get_height()/2, f'{rssi}', va='center', fontsize=8)

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect(MQTT_BROKER, 1883, 60)
client.loop_start()

ani = animation.FuncAnimation(fig, animate, interval=1000)
plt.tight_layout()
plt.show()

client.loop_stop()