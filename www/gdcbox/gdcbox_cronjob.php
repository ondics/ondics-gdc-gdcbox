#!/usr/bin/php
<?php
    /*
    //  GDCBox Cron Script
    //  (C) Ondics,2012
    */

    $gdcbox_version="0.1";
    $testmode=FALSE; // einblenden von "Test" in Fußzeile und "logout" bei Info.
    $testmsg="";    // wird eingeblendet im footer

    // basepath and baseurl depend on machine (default is gdcbox);
    $basepath="/www-wifunbox";  
    $baseurl="";
    $machine="srv1";
    if ($machine=="srv1") {
        // ...for use on srv1.ondics.de
        $basepath="/home/clauss/git-repos/ondics-gdc-gdcbox/www";
        $baseurl="/gdcbox";
    }
    // now the real paths
    $database=$basepath.'/gdcbox/gdcbox-db.sqlite';
    $classinc=$basepath.'/gdcbox/classes.inc';
    $myurl=$baseurl."/cgi-bin/gdcbox.php";
    
    // appstore access
    $appstore_url="http://srv1.ondics.de/gdcbox/appstore/appstore.php";
    $appstore_user="appstore";
    $appstore_pass="appstore";
    $apppath=$basepath."/gdcbox/apps";
        
    // cronjob-script
    $cronjob_script='gdcbox_cronjob.php';
    $cronjob_script_path=$basepath.'/gdcbox/'.$cronjob_script;
    
    // gdc access
    $gdc_baseurl="http://gdc.ondics.de/gdc-da.php";
    
    function output($output, $stop=false) {
        echo date("Y-m-d H:i:s")." PID=".getmypid()." ".$output.". ";
        if ($stop) die("aborting!\n");
        echo "\n";
    }
    
    // datenbankzugriff herstellen
    if (! ($pdo=new PDO('sqlite:'.$database)) )
        output("error: database access",true);

    require_once($classinc);
    
    // get device_id from command line (as specifioed in crontab)
    if (empty($argc) || $argc<2)
        output(" error: device_id missing",true);
    $device_id=$argv[1];

    // load device-app
    $query = $pdo->prepare("SELECT gd.appfile FROM devices d, generic_devices gd ".
                           "WHERE d.generic_device_name=gd.name ".
                           "AND d.id=".$device_id);
    $query->execute();
    $row = $query->fetch();
    if (!$row)
        output("error: device_id [".$device_id."] incorrect",true);

    // load dynamic device app code
    $appfile=$row[0];
    require_once($apppath."/".$row[0]);
 
    // instantiate new device object
    $classname=substr($appfile,0,strpos($appfile,"."));
    $device=new $classname();
    $device->loadDeviceFromDB($device_id);
    if (!$device->isLoaded())
        output("error: device [".$classname."] not found",true);
    
    // prepare GDC-Url 
    $gdc_url=$gdc_baseurl."?sid=".$device->device_values['gdc_sid'];

    // do some logging...
    output("Device ".$device->device_values['name'].
           " (id=".$device_id.", ".$device->device_config_values['IP-Address'].")");

    // check if ip-address ist set 
    if (strncmp($device->device_config_values['IP-Address'],"0.0.",4)===0)
        output("error: ip-address unconfigured",true);

    // now request values from device
    $error=$device->getValuesFromDevice();
    if ($error!="") output("error: ".$error,true);
        
    // generate gdc url from values
    for ($i=0; $i<$device->device_config_values['NumValues']; $i++) {
        if ($device->values[$i]!="")
            $gdc_url.="&value".$i."=".$device->values[$i];
    }
    
    // finally send values to GDC
    //echo "gdc_url=[$gdc_url]\n";
    output("GDC-Request: ".$gdc_url);
    output("****** GDC-Request fehlt noch!");

    // db-connectivity schließen
    unset($pdo);

?>
