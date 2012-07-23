<?php

class Device {
    //protected $device_name,$name,$name_long,$description,$version,$url,$date_installed;
    protected $generic_device_specs = array(); // filled by subclass
    protected $device_defaults = array(); // filled by subclass
    protected $device_config_defaults = array(); // filled by subclass
    // values from db
    public $device_values = array(); 
    public $device_config_values = array(); 
    
    private $loaded = false;

    function isLoaded() {
        return $this->loaded;
    }
    
    function createNewGenericDeviceInDB() {
        global $pdo;
        echo "<p>Device::createNewGenericDeviceInDB()</p>";
        $values="";
        $fieldnames="";
        foreach ( $this->generic_device_specs as $field => $value) {
            $fieldnames.=($fieldnames==""?"":", ").$field;
            $values.=($values==""?"":", ").(is_int($value)?$value:("'".$value."'"));
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
        // second: remove all devices
        echo "<p>removing configs for all devices to be removed</p>";
        $pdo->exec("DELETE FROM device_configs WHERE device_id IN ".
                   "( SELECT id FROM devices d WHERE generic_device_name='".$generic_device_name."')");
        // third: remove generic_device
        echo "<p>removing devices</p>";
        $pdo->exec("DELETE FROM devices WHERE generic_device_name='".$generic_device_name."'");
        // fourth: remove generic_device
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
            $values.=($values==""?"":", ").(is_int($value)?$value:("'".$value."'"));
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
        $pdo->exec("UPDATE devices  ".
                   "SET name='".$this->device_values['name']."',".
                   "description='".$this->device_values['description']."',".
                   "location='".$this->device_values['location']."',".
                   "interval_min=".$this->device_values['interval_min'].",".
                   "gdc_sid='".$this->device_values['gdc_sid']."' ".
                   "WHERE id=".$this->device_values['id']);
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
    }

}

?>