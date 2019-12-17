# TumbleBot - Tumbledryer monitoring system

This is a homemade IoT device built entirely using a ESP-32, LDR and SW-420 vibration sensor. 

This device was made using the following devices and components:
  - ESP-32 (main device program runs on) - [Amazon link](https://www.amazon.co.uk/MakerHawk-Development-0-96inch-Display-Compatible/dp/B076P8GRWV/ref=sr_1_2?keywords=esp-32&qid=1576590700&sr=8-2)
  - LDR (light dependent resistor for checking lights on tumble dryer) - [Amazon link](https://www.amazon.co.uk/Quality-GL5516-Dependent-Resistor-Photoresistor/dp/B00NXW9WZ6/ref=sr_1_1?keywords=ldr&qid=1576590718&sr=8-1)
  - SW-420 (vibration sensor for checking vibration from tumble dryer - [Amazon link](https://www.amazon.co.uk/ARCELI-SW-420-Vibration-Sensor-Arduino-Green/dp/B07BP5568L/ref=sr_1_1?keywords=sw420&qid=1576590731&sr=8-1)
  - MFRC522 (card scanner for uni cards/key fobs) - [Amazon link](https://www.amazon.co.uk/IZOKEE-RFID-RC522-13-56MHz-Arduino-Raspberry/dp/B076HTH56Q/ref=sr_1_3?keywords=mfrc522&qid=1576590742&sr=8-3)
  
Software used for IoT system:
  - Firebase Real Time Database for fetching and storing data - [Firebase docs](https://firebase.google.com/docs/database)
  - ReactJS for front-end UI - [ReactJS docs](https://reactjs.org/)
  - IFTTT for triggering events with Telegram - [IFTTT](https://ifttt.com/)
  - Notify Me for sending notifications to Amazon Echo devices - [Notify Me](https://www.thomptronics.com/about/notify-me)
