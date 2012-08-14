<!--#!/usr/bin/php-cgi-->
<?php
    /*
    //  GDCBox API
    //  (C) Ondics,2012
    */

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
    $myurl=$baseurl."/cgi-bin/gdcbox-api.php";
    
    // appstore access
    $appstore_url="http://srv1.ondics.de/gdcbox/appstore/appstore.php";
    $appstore_user="appstore";
    $appstore_pass="appstore";
    $apppath=$basepath."/gdcbox/apps";
    
    // logging
    $api_logfile="/tmp/gdcbox_api.log";
    

    // datenbankzugriff herstellen
    if (! ($pdo=new PDO('sqlite:'.$database)) ) {
        echo "<html><body>Fehler: DB-Zugriff</body></html>";
        exit;
    } 


    //////////////////////////////////////////////
    // declare some functions for global use
    //
    function getValueFromURLSave($key) {
        return isset($_GET[$key])?htmlentities($_GET[$key],ENT_QUOTES):''; 
    }

    require_once($classinc);

    // api-function
    $cmd=getValueFromURLSave('cmd');

    // save errors here
    $error="";
    
    //////////////////////////////////////////
    //
    // main program starts here 
    //
    //////////////////////////////////////////
     
    switch ($cmd) {
    case 'help':
        echo "<html><head><title>GDCBox-API Help</title></head><body>";
        echo "<h1>GDCBox-API Help</h1>";
        echo "<p>API-Call: $myurl?cmd=&lt;command&gt;{&key=value}...</p>";
        echo "<p>Valid API-Commands are:</p>";
        echo "<ul><li>cmd=getvalues&device_id=&lt;id&gt;</li>";
        echo "<li>cmd=lastlog</li>";
        echo "<li>cmd=...</li></ul>";
        echo "<p>(C) Ondics GmbH, 2012</p>";
        echo "</body></html>";
        break;

    case 'getvalues':
        $device_id=getValueFromURLSave('device_id');
        if (!is_numeric($device_id)) {$error="device_id [$device_id] is not a number";break; }
        // now request values from device
        $values=array();
        $timestamp="";
        $error=Device::loadValuesFromDB($device_id,$values,$timestamp);
        if ($error!="") break;
        echo json_encode(array("timestamp"=>$timestamp,"values"=>$values));
        
        break;

/*
    // eigenschaften und typ des devices ausgeben (für buttons etc.)
    case 'getdevice':
        $device_id=getValueFromURLSave('device_id');
        $query = $pdo->prepare("SELECT gd.appfile,d.active,d.gdc_send ".
                               "FROM devices d, generic_devices gd ".
                               "WHERE d.generic_device_name=gd.name ".
                               "AND d.id=".$device_id);
        $query->execute();
        $row = $query->fetch();
        if (!$row) { $error="device_id [".$device_id."] incorrect";break; )
        // if app is not active, then stop!
        if ($row[1]=="no") { $error="device (id=".$device_id.") is not set active"; break; }
        // load dynamic device app code
        $appfile=$row[0];
        require_once($apppath."/".$row[0]);
 
        // instantiate new device object
        $classname=substr($appfile,0,strpos($appfile,"."));
        $device=new $classname();
        $device->loadDeviceFromDB($device_id);
        if (!$device->isLoaded()) { $error="device [".$classname."] not found in db"; break; }
*/
        


    default:
        $error='command not valid.';
    }

    if ($error != "") {
        $logline=$_SERVER['REMOTE_ADDR']." ".date("Y-m-d H:i:s")." ".$error."\n";
        file_put_contents($api_logfile,$logline,FILE_APPEND);
        echo "<html><head><title>GDC-Box API: Error</title></head><body>";
        echo '<p>Error:'.$error.'</p><p>try <a href="'.$myurl.'?cmd=help">help</a></p>';
        echo '</body></html>';
    } 

    // db-connectivity schließen
    unset($pdo);
    
?>
