# Challenge 1: WiFi Signal Strength Mapper

## Description
WiFi scanner that publishes network data via MQTT and creates a live bar chart visualization in Python.

## Setup

### ESP32
1. Copy `esp32/env.example` to `esp32/.env`
2. Fill in your WiFi credentials and MQTT topic prefix
3. Upload to ESP32 using PlatformIO

### Python
```bash
cd python
uv run visualizer.py
```

## Features
- Scans WiFi networks every 5 seconds
- Filters networks weaker than -80 dBm
- Publishes top 10 networks to MQTT
- Real-time bar chart with color coding:
  - Green: RSSI > -60 (Excellent)
  - Yellow/Orange: -60 to -75 (Good)
  - Red: < -75 (Weak)
  - Blue: Connected network
- Saves all data to `wifi_data.csv`

## Video Demo
[YouTube Link Here]
https://youtu.be/6xLdGoHvBP0



