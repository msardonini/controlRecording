#!/bin/sh


sudo cp remoteSender.service /etc/systemd/system/
# sudo cp rfcomm.service /etc/systemd/system/

sudo systemctl daemon-reload
sudo systemctl enable remoteSender
# sudo systemctl enable rfcomm

