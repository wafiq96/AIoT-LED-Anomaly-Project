# AIoT-LED-Anomaly-Project
An end-to-end AIoT system for predictive maintenance of LED lighting using on-device anomaly detection. Sensor data (e.g., current, voltage, temperature, light intensity) is captured on the STM32 B-L4S5I-IOT01A and processed in real time by a lightweight autoencoder running directly on the edge — flagging early signs of degradation or failure without relying on cloud inference.
Detected anomalies and telemetry are published over MQTT to HiveMQ Cloud, with Node-RED used for real-time visualization, alerting, and dashboard integration — enabling continuous, low-latency monitoring of lighting infrastructure health.
Key Features

Edge Inference: Autoencoder deployed on STM32 (TinyML) for real-time, on-device anomaly scoring
Predictive Maintenance: Detects early degradation patterns before hard failure
Cloud Connectivity: Secure telemetry publishing via HiveMQ Cloud (MQTT)
Visualization & Alerts: Node-RED dashboard for live monitoring and notifications
Low Latency, Low Bandwidth: On-device inference minimizes data sent to the cloud

Tech Stack
STM32 B-L4S5I-IOT01A · Edge AI / TinyML · Autoencoder · MQTT · HiveMQ Cloud · Node-RED
