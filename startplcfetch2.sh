#!/bin/sh
if [ "$1" = "" ]; then
	echo "devi fornire il path di plcfetch (es. /root/plcfetch)"
	exit -1
fi
PLCFETCH_PATH=$1
# if you want poll the configuration data from db every read from serial add -q
# if you want verbose mode add -v
# if you want to run process ad a daemon add -B
#$PLCFETCH_PATH  -v -D PLCFETCH -U plcfetch -P plcf3tch -I $(/sbin/ifconfig eth0 | grep "inet addr" | awk '{print $2}' | sed 's/addr\://g') 2>&1 > ~/plcfetch.log



$PLCFETCH_PATH  $2 -i /dev/ttyS1 -n 2 -q -v -D PLCFETCH -U plcfetch -P plcf3tch -I $(/sbin/ifconfig eth0 | grep "inet addr" | awk '{print $2}' | sed 's/addr\://g') 
