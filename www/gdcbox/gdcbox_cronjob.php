#!/usr/bin/php
<?php
    /*
    //  GDCBox Cron Script
    //  (C) Ondics,2012
    */

    require_once("platforms.inc");

    function output($output, $stop=false) {
        echo date("Y-m-d H:i:s")." PID=".getmypid()." ".$output.". ";
        if ($stop) die("aborting!\n");
        echo "\n";
    }

/*
    // workaround for simultaneously started processes (avoid database locking)
    // wait some time than start script
    $waitingtime=rand(100000,2000000);
    //echo "startup waiting $waitingtime ms. \n";
    usleep($waitingtime);
*/
    
    // datenbankzugriff herstellen
    if (! ($pdo=new PDO('sqlite:'.$env["database"])) )
        output("error: database access",true);
    
    require_once($env["classinc"]);
    
    // get device_id from command line (as specifioed in crontab)
    if (empty($argc) || $argc<2)
        output(" error: device_id missing",true);
    $device_id=$argv[1];

    // load device-app
    $query = $pdo->prepare("SELECT gd.appfile,d.active,d.gdc_send ".
                           "FROM devices d, generic_devices gd ".
                           "WHERE d.generic_device_name=gd.name ".
                           "AND d.id=".$device_id);
    $query->execute();
    $row = $query->fetch();
    if (!$row)
        output("error: device_id [".$device_id."] incorrect",true);
    
    // do we have to send values later on?
    $gdc_send=($row[2]=="no")?false:true;
    
    // if app is not active, then stop!
    if ($row[1]=="no")
        output("error: device (id=".$device_id.") is not set active",true);
        
    // load dynamic device app code
    $appfile=$row[0];
    require_once($env["apppath"]."/".$row[0]);
 
    // instantiate new device object
    $classname=substr($appfile,0,strpos($appfile,"."));
    $device=new $classname();
    $device->loadDeviceFromDB($device_id);
    if (!$device->isLoaded())
        output("error: device [".$classname."] not found in db",true);
    
    $ipaddress=$device->device_config_values['IP-Address']['value'];
    // do some logging...
    output("Device ".$device->device_values['name']." (id=".$device_id.", ipaddr=".$ipaddress.")");
    
    // check if ip-address ist set 
    if (strncmp($ipaddress,"0.0.",4)===0 || strlen(trim($ipaddress))==0 )
        output("error: ip-address unconfigured",true);

    // now request values from device
    $error=$device->getValuesFromDevice();
    
    $values_to_print=implode("|",$device->values);
    output("values=$values_to_print");

    if ($error!="") output("error: ".$error,true);  // abort on error!
    
    // save values to db for later (api) access
    $error=$device->saveValuesToDB();
    if ($error!="") output("error: ".$error,true);  // abort on error!
    
    // prepare GDC-Url 
    $gdc_url=$env["gdc_baseurl"]."?sid=".$device->device_values['gdc_sid'];
    // add values to gdc url 
    for ($i=0; $i<$device->device_config_values['NumValues']['value']; $i++) {
        if ($device->values[$i]!="")
            $gdc_url.="&value".$i."=".$device->values[$i];
    }
    // add timestamp to gdc url
    $gdc_url.="&timestamp=".date("Y-m-d")."%20".date("H:m");

    
    // finally send values to GDC (if send-flag is set!)
    // if app is not active, then stop!
    if ($gdc_send) {
        output("GDC-Request: ".$gdc_url);
        if (strlen($device->device_values['gdc_sid'])!=40)
            output("Warning: GDC-SID is missing / wrong. nothing sent");
        else {
            // http-request to gdc should be here!
            output("Error: gdc-send not yet implemented",true);
        }
    } else {
        //output("GDC-Request would be: ".$gdc_url);
        //output("Warning: do not send to GDC (as configured)");
    }

    // db-connectivity schließen
    unset($pdo);

?>
