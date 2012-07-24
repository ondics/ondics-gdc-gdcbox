<?php
//require_once('classes.inc');

class testdevice_mini extends Device {

    function setDefaultValues() {
        parent::setDefaultValues();        
        $this->generic_device_specs['version']='1.0.0';
        $this->generic_device_specs['name']='Testdevice Mini';
        $this->generic_device_specs['name_long']='Minimales Testdevice ohne Network-Access';
        $this->generic_device_specs['description']='Nur zum Testen';
        $this->generic_device_specs['appfile']='testdevice_mini.inc';
        $this->device_defaults['description']='Testdevice ohne Network-Access. Liefert als Messwert immer 1';
        $this->device_config_defaults['IP-Address']='localhost';
        $this->device_config_defaults['NumValues']=1;
        $this->device_config_defaults['0_Description']='Returns always 1';
        $this->device_config_defaults['0_Formula']='$value';
        parent::checkDefaultValues();        
    }

    function getValuesFromDevice() {
        $this->error="";
        if (!parent::isLoaded()) {$this->error.="error: device not loaded";return; }
        $value=1;
        $this->error .= parent::getValuesFromDevicePostprocessingValue(0,$value);
        return $this->error; 
    }
}

?>
