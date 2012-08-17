<?php

class Device {
    
    // default values, set in subclasses
    protected $generic_device_specs = array();
    protected $device_defaults = array();
    protected $device_config_defaults = array();
    // parameters from db
    public $device_values = array(); 
    public $device_config_values = array(); 
    // current status
    private $loaded = false;        // device_values loaded from db?
    public $error = "";             // status of last operation
    public $values = array();       // last values read from device

    function isLoaded() {
        return $this->loaded;
    }
   
    function setDefaultValues() {
        $this->generic_device_specs = array(
            'version'=>'0.0.0',
            'name'=>'',
            'name_long'=>'',
            'description'=>'',
            'url'=>'',
            'appfile'=>''
        );  
        $this->device_defaults = array(
            'name'=>'',
            'description'=>'',
            'location'=>'',
            'interval_min'=>'5',
            'gdc_sid'=>'===***unset***===',
            'generic_device_name'=>'',
            'active'=>'no',
            'gdc_send'=>'no'
        );
        $this->device_config_defaults = array(
            'IP-Address'=>'',
//            'Port'=>'',
            'NumValues'=>'1',
            '0_Description'=>'',
            '0_Formula'=>'$value'
        );
    }
    
    function checkDefaultValues() {
        // check generice_device values
        if (!isset($this->generic_device_specs['version'])) die("init error: version missing");
        if (!isset($this->generic_device_specs['name']))    die("init error: name unset");
        if (!isset($this->generic_device_specs['appfile'])) die("init error: appfile unset");

        // check device_default values
        $this->device_defaults['name']=$this->generic_device_specs['name'];
        $this->device_defaults['generic_device_name']=$this->generic_device_specs['name'];
        $this->device_defaults['gdc_sid']="***===unset===***";
    }    
    
    function createNewGenericDeviceInDB() {
        global $pdo;
        echo "<p>Device::createNewGenericDeviceInDB()</p>";
        $values="";
        $fieldnames="";
        foreach ( $this->generic_device_specs as $field => $value) {
            $fieldnames.=($fieldnames==""?"":", ").$field;
            //$values.=($values==""?"":", ").(is_int($value)?$value:("'".$value."'"));
            $values.=($values==""?"":", ")."'".$value."'";
        }
        $fieldnames.=",date_installed";
        $values.=", '".date("Y-m-d H:i:s")."'";
        $sql="INSERT INTO generic_devices (".$fieldnames.") VALUES (".$values.") ";
        echo "<p>sql=".$sql."</p>";
        $pdo->exec($sql);
    }
    
    function removeGenericDeviceFromDB($generic_device_name) {
        global $apppath,$pdo;
        
        // first: delete local files (read filename from db)
        $query = $pdo->prepare("SELECT appfile FROM generic_devices ".
                                "WHERE name='".$generic_device_name."'");
        $query->execute();
        if ($row = $query->fetch()) {
            echo "<p>removing file ".$apppath."/".$row['appfile']."</p>";
            unlink($apppath."/".$row['appfile']);
        } else {
            echo "<p>error: Device::removeGenericDeviceFromDB!";
            return false;
        }
        // second: remove all devices_values from these devices
        echo "<p>removing values for all devices to be removed</p>";
        $pdo->exec("DELETE FROM device_values WHERE device_id IN ".
                   "( SELECT id FROM devices WHERE generic_device_name='".$generic_device_name."')");
        // third: remove all devices
        echo "<p>removing configs for all devices to be removed</p>";
        $pdo->exec("DELETE FROM device_configs WHERE device_id IN ".
                   "( SELECT id FROM devices WHERE generic_device_name='".$generic_device_name."')");
        // fourth: remove generic_device
        echo "<p>removing devices</p>";
        $pdo->exec("DELETE FROM devices WHERE generic_device_name='".$generic_device_name."'");
        // fifth: remove generic_device
        echo "<p>removing generic_device</p>";
        $pdo->exec("DELETE FROM generic_devices WHERE name='".$generic_device_name."'");
        
        return true;
    }

    function createNewDeviceInDB() {
        global $pdo;
        echo "<p>Device::createNewDeviceInDB()</p>";
        // first create device
        $values="";
        $fieldnames="";
        foreach ( $this->device_defaults as $field => $value) {
            $fieldnames.=($fieldnames==""?"":", ").$field;
            //$values.=($values==""?"":", ").(is_int($value)?$value:("'".$value."'"));
            $values.=($values==""?"":", ")."'".$value."'";
        }
        $sql="INSERT INTO devices (".$fieldnames.") VALUES (".$values.")";
        $pdo->exec($sql);
        $device_id=$pdo->lastInsertId('id');
        // then create corresonding device_configs        
        foreach ( $this->device_config_defaults as $key => $value) {
            $sql="INSERT INTO device_configs (device_id,key,value) ".
                 "VALUES (".$device_id.",'".$key."','".$value."')";
            echo "<p>sql=".$sql."</p>";
            $pdo->exec($sql);
        }
        // then create one entry for all values of this device (format is
        // empty value "-" separated by "|")
        $valuesstring="";
        for($i=0; $i<$this->device_config_defaults['NumValues'];$i++)
            $valuesstring.=($valuesstring==""?"":"|")."-";
        $sql="INSERT INTO device_values (device_id,timestamp,valuesstring) ".
             "VALUES (".$device_id.",'".date("Y-m-d H:i:s")."','".$valuesstring."')";
        echo "<p>sql=".$sql."</p>";
        $pdo->exec($sql);        
        
        return $device_id;
    }

    function loadDeviceFromDB($device_id) {
        global $pdo;
        $this->loaded=false;
        // first get device-values...
        $query = $pdo->prepare("SELECT * FROM devices WHERE id=".$device_id);
        $query->execute();
        $row = $query->fetch(PDO::FETCH_ASSOC);
        if (!$row) return;
        foreach($row as $field => $value)
            $this->device_values[$field]=$value;
        // .. then get device-config-values...
        $query = $pdo->prepare("SELECT * FROM device_configs WHERE device_id=".$device_id);
        $query->execute();
        while( $row = $query->fetch(PDO::FETCH_ASSOC) )
            $this->device_config_values[$row['key']]=$row['value'];
        $this->loaded=true;
    }


    function saveDeviceToDB() {
        global $pdo;
        if (!$this->loaded) return;
        $sqlfields="";
        foreach ($this->device_values as $key => $value) {
            switch ($key) {
                case 'generic_device_name': break;
                case 'id': break;
                default: $sqlfields.=($sqlfields==""?"":", ").$key."='".$value."'";
            }
        }
        $pdo->exec("UPDATE devices SET ".$sqlfields." WHERE id=".$this->device_values['id']);
        foreach ($this->device_config_values as $key => $value) 
            if ($key!='NumValues')
                $pdo->exec("UPDATE device_configs SET value='".$value."' ".
                     "WHERE device_id=".$this->device_values['id']." AND key='".$key."'");
        
    }

    function removeDeviceFromDB($id) {
        global $pdo;
        // first: remove from device_configs
        echo "<p>removing device configs... </p>";
        $pdo->exec("DELETE FROM device_configs WHERE device_id=".$id);
        // second: remove device
        echo "<p>removing device</p>";
        $pdo->exec("DELETE FROM devices WHERE id=".$id);
        // third: remove values
        echo "<p>removing values</p>";
        $pdo->exec("DELETE FROM device_values WHERE device_id=".$id);
        
    }
    
    // for a single value do some postprocessing: the value
    // is calculated with "n_Formula"-field
    function getValuesFromDevicePostprocessingValue($numValue,$value) {
        $error="";
        $phpcode='$value=('.$this->device_config_values[$numValue."_Formula"].');';
        $eval_ok=eval($phpcode);
        if ($eval_ok===FALSE)
            $error=" eval failed for value [$$numValue].";
        $this->values[$numValue]=$value;
        return $error;
    }
    
    function saveValuesToDB() {
        global $pdo;        
        $valuesstring=implode("|",$this->values);
        $sql="UPDATE device_values SET timestamp='".date("Y-m-d H:i:s")."',".
                     "valuesstring='".$valuesstring."' ".
                     "WHERE device_id=".$this->device_values['id'];
        //echo "<p>sql=".$sql."</p>";
        $pdo->exec($sql);        
    }

    // function is static for performance reasons: we don't need
    // an instantiated object to get the values from db!
    static function loadValuesFromDB($device_id, &$values, &$timestamp) {
        global $pdo;
        $query = $pdo->prepare("SELECT timestamp,valuesstring FROM device_values ".
                               "WHERE device_id=".$device_id);
        $query->execute();
        $row = $query->fetch();
        if (!$row) { return "no values found in db for device $device_id";}
        $values=explode("|",$row['valuesstring']);
        $timestamp=$row['timestamp'];
  }

   

}

?>