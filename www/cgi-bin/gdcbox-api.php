<!--#!/usr/bin/php-cgi-->
<?php
    /*
    //  GDCBox API
    //  (C) Ondics,2012
    */
    require_once("../gdcbox/platforms.inc");

    // datenbankzugriff herstellen
    if (! ($pdo=new PDO('sqlite:'.$env["database"])) ) {
        echo "<html><body>Fehler: DB-Zugriff</body></html>";
        exit;
    } 

    //////////////////////////////////////////////
    // declare some functions for global use
    //
    function getValueFromURLSave($key) {
        return isset($_GET[$key])?htmlentities($_GET[$key],ENT_QUOTES):''; 
    }

    require_once($env["classinc"]);
    
    $error="";  // save errors here  
    $result=""; // result-array for api-calls
    
    //////////////////////////////////////////
    //
    // main program starts here 
    //
    //////////////////////////////////////////
    // get api-function
    $cmd=getValueFromURLSave('cmd');
    switch ($cmd) {
    case 'help':
        echo "<html><head><title>GDCBox-API Help</title></head><body>";
        echo "<h1>GDCBox-API Help</h1>";
        echo "<p>API-Call: ".$env["api_url"]."?cmd=&lt;command&gt;{&key=value}...</p>";
        echo "<p>Valid API-Commands are:</p>";
        echo "<ul><li>cmd=getvalues&device_id=&lt;id&gt;</li>";
        echo "<li>cmd=setvalue&device_id=&lt;id&gt;&num=&lt;number of value&gt;&value=&lt;new value&gt;</li>";
//        echo "<li>cmd=setvalue_with_update&device_id=&lt;id&gt;&num=&lt;number of value&gt;&value=&lt;new value&gt;</li>";
//        echo "<li>cmd=lastlog</li>";
        echo "<li>cmd=help</li></ul>";
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
        $result=array("timestamp"=>$timestamp,"values"=>$values);
        break;
    
    case 'setvalue':
        $device_id=getValueFromURLSave('device_id');
        if (!is_numeric($device_id)) {$error="device_id [$device_id] is not a number";break; }
        
        // we need the concrete device object to set values!

        // load device-app
        $query = $pdo->prepare("SELECT gd.appfile,d.active,d.gdc_send ".
                           "FROM devices d, generic_devices gd ".
                           "WHERE d.generic_device_name=gd.name ".
                           "AND d.id=".$device_id);
        $query->execute();
        $row = $query->fetch();
        if (!$row) {$error="device_id [".$device_id."] incorrect";break; }
        // load dynamic device app code
        $appfile=$row[0];
        require_once($env["apppath"]."/".$row[0]);
 
        // instantiate new device object
        $classname=substr($appfile,0,strpos($appfile,"."));
        $device=new $classname();
        $device->loadDeviceFromDB($device_id);
        if (!$device->isLoaded())
            {$error="device [".$classname."] not found in db";break; }

        // read parameters from url 
        $num=getValueFromURLSave('num');
        $value=getValueFromURLSave('value');

        // is device an actor?
        if (!isset($device->device_config_values['Actor']['value']) ||
            $device->device_config_values['Actor']['value']!="yes")
            {$error="error: device is no actor";break; }

        // and set new value!        
        $error.=$device->setActorValue($num,$value);
        
        unset($device);
        break;        

    // eigenschaften und typ des devices ausgeben (für buttons etc.)
    case 'getdeviceinfo':
        
        $device_id=getValueFromURLSave('device_id');
        if (!is_numeric($device_id)) {$error="device_id [$device_id] is not a number";break; }
        
        // we need the concrete device object to get info!
        
        // load device-app
        $query = $pdo->prepare("SELECT gd.appfile,d.active,d.gdc_send ".
                           "FROM devices d, generic_devices gd ".
                           "WHERE d.generic_device_name=gd.name ".
                           "AND d.id=".$device_id);
        $query->execute();
        $row = $query->fetch();
        if (!$row) {$error="device_id [".$device_id."] incorrect";break; }
        
        // if app is not active, then stop!
        if ($row[1]=="no") {$error="device (id=".$device_id.") is not set active";break; }

        // load dynamic device app code
        $appfile=$row[0];
        require_once($env["apppath"]."/".$row[0]);
 
        // instantiate new device object
        $classname=substr($appfile,0,strpos($appfile,"."));
        $device=new $classname();
        $device->loadDeviceFromDB($device_id);
        if (!$device->isLoaded())
            {$error="device [".$classname."] not found in db";break; }
        
        $result= array("device_values"=>$device->device_values,
                       "device_config_values"=>$device->device_config_values );
        
        unset($device);
        break;        


    default:
        $error='api command not valid.';
    }

    if ($error != "") {
        $logline=$_SERVER['REMOTE_ADDR']." ".date("Y-m-d H:i:s")." ".$error."\n";
        file_put_contents($env["api_logfile"],$logline,FILE_APPEND);
        echo json_encode(array("success"=>"error","err_msg"=>$error));
        /*
        echo "<html><head><title>GDC-Box API: Error</title></head><body>";
        echo '<p>Error:'.$error.'</p><p>try <a href="'.$env["api_url"].'?cmd=help">help</a></p>';
        echo '</body></html>';
        */
    } else
        echo json_encode(array(array("success"=>"OK"),$result));

    // db-connectivity schließen
    unset($pdo);
    
?>
