#!/bin/sh


sudo cp remoteSender.service /lib/systemd/system/
sudo cp rfcomm.service /lib/systemd/system/

sudo systemctl daemon-reload
sudo systemctl enable remoteSender
sudo systemctl enable rfcomm

