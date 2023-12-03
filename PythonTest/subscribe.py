import paho.mqtt.client as mqtt

client = mqtt.Client()

client.connect("35.91.221.3", 1883)

client.subscribe("particle1")

def on_message(client, userdata, message):
  print("Received message: " + message.topic + " " + message.payload.decode())

client.on_message = on_message

# Run the loop
client.loop_forever()

