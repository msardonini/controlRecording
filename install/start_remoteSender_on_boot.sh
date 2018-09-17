#!/bin/sh


sudo cp remoteSender.service /lib/systemd/system/

sudo systemctl daemon-reload
sudo systemctl enable remoteSender

