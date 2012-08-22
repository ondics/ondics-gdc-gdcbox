<?php

class testdevice extends Device {

    function setDefaultValues() {
        parent::setDefaultValues();        
        $this->generic_device_specs['version']='0.0.1';
        $this->generic_device_specs['name']='Testdevice';
        $this->generic_device_specs['name_long']='Testdevice';
        $this->generic_device_specs['description']='sollte nur zum Testen installiert werden';
        $this->generic_device_specs['url']='http://srv1.ondics.de/appstore/devicehelp.html';
        $this->generic_device_specs['appfile']='testdevice.inc';
        $this->generic_device_specs['platforms']='ANY';//raspberry-pi';
        $this->device_defaults['description']='Testdevice ohne Network-Access. Liefert als Messwert immer 1';
        $this->device_defaults['location']='auf der GDCBox';
        $this->device_config_defaults['IP-Address']=array('value'=>'localhost','attribs'=>'');
        $this->device_config_defaults['NumValues']=array('value'=>1,'attribs'=>'visible:no');
        $this->device_config_defaults['0_Description']=array('value'=>'Returns always 123','attribs'=>'');
        $this->device_config_defaults['0_Formula']=array('value'=>'$value','attribs'=>'');
        parent::checkDefaultValues();        
    }

    function getValuesFromDevice() {
        $this->error="";
        if (!parent::isLoaded()) {$this->error.="error: device not loaded";return; }
        $value=123;
        $this->error .= parent::getValuesFromDevicePostprocessingValue(0,$value);
        return $this->error; 
    }
}

?>
