import paho.mqtt.client as mqtt


client = mqtt.Client()

print(client.connect("54.201.49.72", 1883))

client.publish("test", "hello")

client.disconnect()