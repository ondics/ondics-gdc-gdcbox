<?php

    require_once("../gdcbox/platforms.inc");

    // we need classinc for device instantiation
    require_once($env["classinc"]);
    
    // build array $applist containing all apps (from app directory):
    // how? we extract 'name', 'version', 'platforms' and 'appfile' from each class
    // 'platforms' has format "platform1,platform2,..." or "ANY" if platform independend
    // supported platforms for gdcbox are defined in platforms.inc
    $applist=array();
    foreach(glob('./apps/*.inc') as $file) {
        //echo "<p>file=[$file]</p>";
        //require_once($file);
        $file_content=file_get_contents($file);
        //if (!preg_match("/^\s*class\s*(?'classname'\S*)\s*Device/",$file_content,$matches))
        if (!preg_match("/(class[\s]*)([\w]*)([\s]*extends[\s]*)/",$file_content,$matches)) 
            { echo "<p>no class found</p>"; continue; }
        require_once($file);
        //echo "<p>matches:<br>";var_dump($matches);echo "</p>";
        $classname=$matches[2]; // just classname is relevant!
        $device=new $classname();
        $device->setDefaultValues();
        $applist[]=array('name'     => $device->generic_device_specs['name'],
                         'version'  => $device->generic_device_specs['version'],
                         'platforms'=> $device->generic_device_specs['platforms'],
                         'file'     => $device->generic_device_specs['appfile']);
        unset($device);
    }
    
    //
    // appstore-main processing
    //
    $action=isset($_GET['action'])?htmlentities($_GET['action'],ENT_QUOTES):'';
    $machine_os=isset($_GET['machine_os'])?htmlentities($_GET['machine_os'],ENT_QUOTES):'';
    
    switch ($action) {
        // applist returns json-encoded list with all apps
        // available for specified platform
        // request: action=applist&machine_os=<platform>
        // <platform> is optional and may be raspberry, ubuntu, openwrt,... 
        case 'applist':
            $applist_platform=array();
            foreach($applist as $app) {
                $platforms=explode(",",$app["platforms"]);
                if ($platforms[0]=="ANY" || in_array($machine_os,$platforms))
                    $applist_platform[]=$app;
            }
            echo json_encode($applist_platform);
            break;
        
        // download returns requested app as binary file
        // currently, only one file (*.inc) is transfered
        case 'download':
            $appfile=$_GET["appfile"];
            // prüfen, ob app auch in applist ist.
            $i=count($applist);
            while (--$i>=0)
                if ($applist[$i]['file'] == $appfile) break;
            if ($i<0) die("<p>error: app ".$appfile." nicht gefunden<p>");
            // jetzt download starten            
            header("Content-Length: ".filesize("./apps/".$appfile));
            header('Content-Type: application/x-download');
            header('Content-Disposition: attachment; filename="'.$appfile.'"');
            header('Content-Transfer-Encoding: binary');
            $fp=fopen("./apps/".$appfile,"rb");
            while($fp && !feof($fp) && (connection_status()==0)) {
                print(fread($fp, filesize("./apps/".$appfile)));
                flush();
            }
            fclose($fp);
            break;
        
        default:
            die("<p>error in appstore: unknown command</p>");
    }

?>
