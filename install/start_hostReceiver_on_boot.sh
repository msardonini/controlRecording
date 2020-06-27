#!/bin/sh


sudo cp hostReceiver.service /etc/systemd/system/
# sudo cp bluetoothServer.service /etc/systemd/system/

sudo systemctl daemon-reload
sudo systemctl enable hostReceiver
# sudo systemctl enable bluetoothServer

