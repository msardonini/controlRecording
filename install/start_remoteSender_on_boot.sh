#!/bin/sh


cp remoteSender.service /lib/systemd/system/

systemctl daemon-reload
systemctl enable remoteSender

