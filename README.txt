GDCBox V2
=========

Die GDCBox V2 wurde komplett neu in PHP implementiert.
Sie besteht aus 2 Teilen, der eigentlichen GDCBox
und dem AppStore.

(C) Ondics GmbH, 2012

TODOs
-----
- weitere Apps (Web-Grab-Apps!)
- WS500 konkret einbinden
- Anzeige-Geräte einbauen (Aktoren)
- Versionsnummern


AppStore
--------
Der AppStore liegt unter ./www/appstore und ist nur ein
JSON-Server, der die Befehle 'applist' (Liste mit
verfügbaren Apps) und 'download' (Eine App downloaden)
kennt.

Ein App ist der Code zum Anschluss eines Devices an
die GDCBox. Bisher werden nur lesende Devices unterstützt
(z.B. noch keine Laufschrift-Anzeigen oder Aktoren).
Eine App besteht nur aus der Datei <app>.inc und
liegt im Verzeichnis ./www/appstore/apps

GDCBox
------
Die GDCBox besteht im wesentlichen aus 3 Teilen:
- dem Web-UI (./www/cgi-bin/gdcbox.php)
- dem cronjob-programm (./www/gdcbox/gdcbox_cronjob.php).
- der api (./www/cgi-bin/gdcbox-api.php)
Alle greifen auf eine lokale SQLite-DB (Version 3) zu.

Die auf der GDCBox installierten Apps liegen im Verzeichnis
./www/gdcbox/apps.

Neue Apps erstellen
-------------------
Die App-Architektur ist objektorientiert aufgebaut.
Damit kann sich das Erstellen einer neuen App auf
das Wesentliche beschränken. Ist eigentich ganz leicht!
Eine neue App wird am Besten wie folgt erstellt:
- Kopieren einer bestehenden App aus ./appstore/apps
- Anpassen der neuen App
Fertig!

