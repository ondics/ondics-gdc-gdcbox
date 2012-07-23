GDCBox V2
=========

Die GDCBox V2 wurde komplett neu in PHP implementiert.
Sie besteht aus 2 Teilen, der eigentlichen GDCBox
und dem AppStore.

TODOs
-----
- Bilder für Apps (.inc -> .zip)
- weitere Apps (Web-Grab-Apps!)
- WS500 konkret einbinden
- Installation/Test/Distro für Raspberry PI
- Oberfläche auf JQuery(mobile) umstellen
- Automatic Restart Cronjobs nach Änderungen
- Anzeige-Geräte einbauen (Aktoren)


AppStore
--------
Der AppStore liegt unter ./appstore und ist nur ein
JSON-Server, der die Befehle 'applist' (Liste mit
verfügbaren Apps) und 'download' (Eine App downloaden)
kennt.

Ein App ist der Code zum Anschluss eines Devices an
die GDCBox. Bisher werden nur lesende Devices unterstützt
(z.B. noch keine Laufschrift-Anzeigen).
Eine App besteht nur aus der Datei <app>.inc und
liegt im Verzeichnis ./appstore/apps

GDCBox
------
Die GDCBox besteht aus 2 Teilen: dem Web-UI (./www/cgi-bin/gdcbox.php)
und dem cronjob-programm (./www/gdcbox/gdcbox_cronjob.php).
Beide greifen auf eine lokale SQLite-DB zu.

Die auf der GDCBox installierten Apps liegen im Verzeichnis
./www/gdcbox/apps.

Neue Apps erstellen
-------------------
Die App-Architektur ist objektorientiert aufgebaut.
Damit kann sich das Erstellen einer neuen App auf
das Wesentliche beschränken. Ist eigentlch ganz leicht!
Eine neue App wird am Besten wie folgt erstellt:
- Kopieren einer bestehenden App aus ./appstore/apps
- Anpassen der neuen App
- Eintragen der App in der AppStore (in ./appstore/appstore.php)
Fertig!

