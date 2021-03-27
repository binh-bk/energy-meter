#! /usr/bin/python3
# Binh Nguyen, Mar 2021
# listen to MQTT server for logger, 

import time
import json
import os
import paho.mqtt.client as mqtt

import file_log

# MQTT
os.environ['TZ'] = 'Asia/Ho_Chi_Minh'
time.tzset()

BROKER_IP = '192.168.1.21'
MQTT_PW = 'mqtt_pass'
MQTT_USER = 'mqtt_user'
MQTT_TOPIC = [('sensors/#',0),]

def takeTime():
    return time.strftime("%Y-%m-%d %H:%M:%S")


def on_connect(client, userdata, flags, rc):
    client.subscribe(MQTT_TOPIC)
    print("Connected with result code "+str(rc))
    return None


def on_message(client, userdata, msg):
    '''Main function
    INPUT: data client
    else data is saved to a file
    '''

    time_ = takeTime()
    payload = msg.payload.decode('UTF-8').lower()
    if msg.retain == 0:
        try:
            data = json.loads(payload)
            sensor = data['sensor']
            
            # if message does not have a timestamp
            if 'time' not in data.keys():
                data['time'] = takeTime()

            #  send to a log file
            file_log.to_log(data, json_=status)
                
        
        except Exception as e:
            print(f'Exception {e} - not json')          
    else:
        print("Retained MSG: ", payload)
    return None

def on_disconnect(client, userdata, rc):
    if rc !=0:
        print("Unexpected disconnection!")
    else:
        print("Disconnecting")
    return None

# Program starts here
sensors = set()
client = mqtt.Client(client_id='json-logger')
client.username_pw_set(username=MQTT_USER, password=MQTT_PW)
client.connect(BROKER_IP, 1883, 60)
client.on_connect = on_connect
client.on_message = on_message
client.on_disconnect = on_disconnect
time.sleep(0.1)
client.loop_forever()
