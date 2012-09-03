#!/bin/bash

SYSTEM=`uname -a | awk '/raspberry/ {print "raspberry";}
                        /linux srv1/ {print "linux srv1";}
                        /openwrt/ {print "openwrt";}'`
echo "system=$SYSTEM"

if [ "X$SYSTEM" = "X" ]; then
	echo "system unknown";
	exit 1;
fi

if [ "X$SYSTEM" = "Xraspberry" ]; then
(
echo "installing raspberry..."
cd /var/www/gdcbox/ondics-gdc-gdcbox/www/gdcbox;
chmod a+w gdcbox-db.sqlite;
chmod a+w ./;
chmod a+w apps;
chmod a+w ../appstore/apps;
sed -i 's/srv1.ondics.de/localhost/g' platforms.inc
echo "don't forget to make some symlinks from your web-root"
)
fi

if [ "X$SYSTEM" = "Xlinux srv1" ]; then
	echo "system srv1 mussma noch manchen!";
        exit 1;
fi

if [ "X$SYSTEM" = "Xopenwrt" ]; then
(
echo "installing openwrt..."
)
fi

