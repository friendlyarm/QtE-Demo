#!/bin/bash

killall QtE-Demo

. /usr/bin/setqt5env
export QWS_DISPLAY=Linuxfb:/dev/fb0:mmWidth200:mmHeight125:0
cd /opt/QtE-Demo
./QtE-Demo -qws
