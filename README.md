#GDCBox V2

GDCBox helps you to
- collect data from your network and connected hardware,
- store the latest values in a local database,
- make data accesible via an api and
- transmit data to the GDC Global Data Cloud Storage

The GDCBox V2 was completely redesigned and rewritten
in PHP. GDCBox mainly consists of tree parts:
1. the GDCBox user interface
2. the cronjobs collecting data
3. the api
4. and the AppStore to dynamiccaly expand your GDCBox

GDCBox provides hardware specific Apps, e.g. for Raspberry
PI. get more information about [GDCBox on Raspberry PI at http://pi-io.com](http://pi-io.com).

GDCBox can easily expanded by building your own Apps.



(C) Ondics GmbH, 2012

##TODOs
- more Apps (Web-Grab-Apps!)
- complete App for WS500 
- control display devices and actors
- version vontrol of apps
- improve AppStore navigation


##AppStore
Der AppStore liegt unter ./www/appstore und ist ein
REST/JSON-Server, der die Befehle 'applist' (Liste mit
verfügbaren Apps) und 'download' (Eine App downloaden)
kennt.

Eine App ist der Code zum Anschluss eines Devices an
die GDCBox. Bisher werden nur lesende Devices unterstützt
(z.B. noch keine Laufschrift-Anzeigen oder Aktoren).
Eine App besteht nur aus der Datei <app>.inc und
liegt im Verzeichnis ./www/appstore/apps

##GDCBox
Die GDCBox modules are
- the Web-UI (./www/cgi-bin/gdcbox.php)
- the cronjob-script (./www/gdcbox/gdcbox_cronjob.php).
- the api (./www/cgi-bin/gdcbox-api.php)
GDCBox uses a local SQLite (Version 3)

##Develop new Apps
Die App-Architektur ist objektorientiert aufgebaut.
Damit kann sich das Erstellen einer neuen App auf
das Wesentliche beschränken. Ist eigentich ganz leicht!
Eine neue App wird am Besten wie folgt erstellt:
- Kopieren einer bestehenden App aus ./appstore/apps
- Anpassen der neuen App
Fertig!

