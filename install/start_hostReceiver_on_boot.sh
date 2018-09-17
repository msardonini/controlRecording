#!/bin/sh


sudo cp hostReceiver.service /lib/systemd/system/

sudo systemctl daemon-reload
sudo systemctl enable hostReceiver

