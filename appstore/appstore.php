<?php

    $apppath="/home/clauss/git-repos/ondics-gdc-gdcbox/appstore/apps";

    // hier sind alle app gelistet. die zugehörige klasse ist <file> ohne endung.
    // später können anstelle .inc auch .zip dazukommen für mehrere dateien (incl.
    // bilder etc.)
    $applist = array (
        array(  'name'=>'Lufft WS600',
                'ver'=>'0.0.1',
                'platforms'=>'ANY',
                'file'=>'lufft_ws600.inc' ),
        array(  'name'=>'Lufft L2P',
                'ver'=>'0.1.0',
                'platforms'=>'ANY',
                'file'=>'lufft_l2p.inc' ),        
        array(  'name'=>'AVR Net IO',
                'ver'=>'1.0.0',
                'platforms'=>'ANY',
                'file'=>'avrnetio.inc' ),        
        array(  'name'=>'Host Ping',
                'ver'=>'0.0.1', 
                'platforms'=>'ANY',
                'file'=>'hostping.inc' ),
        array(  'name'=>'Raspberry PI I2C',
                'ver'=>'0.0.1', 
                'platforms'=>'raspberry-pi',
                'file'=>'rasppi-i2c.inc' ),
        array(  'name'=>'Raspberry PI 1-Wire Sensor',
                'ver'=>'0.0.2', 
//                'platforms'=>'raspberry-pi',
                'platforms'=>'ANY',
                'file'=>'rasppi_1wire_sensor.inc' ),        
        array(  'name'=>'Testdevice',
                'ver'=>'0.0.1',
                'platforms'=>'ANY',
                'file'=>'testdevice.inc' ),
        array(  'name'=>'Testdevice Mini',
                'ver'=>'1.0.0',
                'platforms'=>'ANY',
                'file'=>'testdevice_mini.inc' ),
    );

    $action=isset($_GET['action'])?htmlentities($_GET['action'],ENT_QUOTES):'';
    $machine_os=isset($_GET['machine_os'])?htmlentities($_GET['machine_os'],ENT_QUOTES):'';
    
    switch ($action) {
        case 'applist':
            $applist_platform=array();
            foreach($applist as $app) {
                $platforms=explode(",",$app["platforms"]);
                if ($platforms[0]=="ANY" || in_array($machine_os,$platforms))
                    $applist_platform[]=$app;
            }
            echo json_encode($applist_platform);
            break;
        case 'download':
            $appfile=$_GET["appfile"];
            // prüfen, ob app auch in applist ist.
            $i=count($applist);
            while (--$i>=0)
                if ($applist[$i]['file'] == $appfile) break;
            if ($i<0) die("<p>error: app ".$appfile." nicht gefunden<p>");
            // jetzt download starten            
            header("Content-Length: ".filesize($apppath."/".$appfile));
            header('Content-Type: application/x-download');
            header('Content-Disposition: attachment; filename="'.$appfile.'"');
            header('Content-Transfer-Encoding: binary');
            $fp=fopen($apppath."/".$appfile,"rb");
            while($fp && !feof($fp) && (connection_status()==0)) {
                print(fread($fp, filesize($apppath."/".$appfile)));
                flush();
            }
            fclose($fp);
            break;
        default:
            die("<p>error in appstore: unknown command</p>");
    }

?>
