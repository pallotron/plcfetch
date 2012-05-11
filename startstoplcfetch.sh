#!/bin/sh
kill -9 $(ps ax | grep plcfetch | grep -v .sh | grep -v grep | awk '{print $1}')
sleep 2
/rw/root/startplcfetch.sh /rw/root/plcfetch -N 1>/dev/null 2>&1 &
