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
        $this->device_config_defaults['IP-Address']   =array('value'=>'localhost','attribs'=>'');
        $this->device_config_defaults['NumberRetries']=array('value'=>'5','attribs'=>'');
        $this->device_config_defaults['NumValues']    =array('value'=>'4','attribs'=>'visible:no');
        $this->device_config_defaults['0_Description']=array('value'=>'RTT min in ms','attribs'=>'');
        $this->device_config_defaults['0_Formula']    =array('value'=>'$value','attribs'=>'');
        $this->device_config_defaults['1_Description']=array('value'=>'RTT avg (average) in ms','attribs'=>'');
        $this->device_config_defaults['1_Formula']    =array('value'=>'$value','attribs'=>'');
        $this->device_config_defaults['2_Description']=array('value'=>'RTT max in ms','attribs'=>'');
        $this->device_config_defaults['2_Formula']    =array('value'=>'$value','attribs'=>'');
        $this->device_config_defaults['3_Description']=array('value'=>'RTT mdev (moving standard deviation) in ms','attribs'=>'');
        $this->device_config_defaults['3_Formula']    =array('value'=>'$value','attribs'=>'');
        parent::checkDefaultValues();        
    }

    function getValuesFromDevice() {
        $this->error="";
        if (!parent::isLoaded()) {$this->error.="error: device not loaded";return; }
        $command = "ping -q -c ".$this->device_config_values['NumberRetries']['value']." ".
                    $this->device_config_values['IP-Address']['value'];
        $lastline= exec($command);
        $ping_all_times=explode(" ",$lastline);
        $ping_times=explode("/",$ping_all_times[3]);
        for ($i=0; $i<$this->device_config_values['NumValues']['value']; $i++)
            $this->error .= parent::getValuesFromDevicePostprocessingValue($i,$ping_times[$i]);
        return $this->error; 
    }
}

?>
