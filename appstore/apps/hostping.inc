<?php
//require_once('classes.inc');

class hostping extends Device {

    function setDefaultValues() {
        parent::setDefaultValues();        
        $this->generic_device_specs['version']='0.0.1';
        $this->generic_device_specs['name']='Host Ping';
        $this->generic_device_specs['name_long']='Host Ping';
        $this->generic_device_specs['description']='Prüfung, ob Host mittels Ping erreichbar ist.';
        $this->generic_device_specs['appfile']='hostping.inc';
        $this->device_defaults['description']='Prüfung, ob Host mittels Ping erreichbar ist.';
        $this->device_config_defaults['IP-Address']='';
        $this->device_config_defaults['NumberRetries']='5';
        $this->device_config_defaults['NumValues']=4;
        $this->device_config_defaults['0_Description']='RTT min in ms';
        $this->device_config_defaults['0_Formula']='$value';
        $this->device_config_defaults['1_Description']='RTT avg (average) in ms';
        $this->device_config_defaults['1_Formula']='$value';
        $this->device_config_defaults['2_Description']='RTT max in ms';
        $this->device_config_defaults['2_Formula']='$value';
        $this->device_config_defaults['3_Description']='RTT mdev (moving standard deviation) in ms';
        $this->device_config_defaults['3_Formula']='$value';
        parent::checkDefaultValues();        
    }

    function getValuesFromDevice() {
        $this->error="";
        if (!parent::isLoaded()) {$this->error.="error: device not loaded";return; }
        $command = "ping -q -c ".$this->device_config_values['NumberRetries']." ".
                    $this->device_config_values['IP-Address'];
        $lastline= exec($command);
        $ping_all_times=explode(" ",$lastline);
        $ping_times=explode("/",$ping_all_times[3]);
        for ($i=0; $i<$this->device_config_values['NumValues']; $i++)
            $this->error .= parent::getValuesFromDevicePostprocessingValue($i,$ping_times[$i]);
        return $this->error; 
    }
}

?>
