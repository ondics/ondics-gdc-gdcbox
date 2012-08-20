<?php



class rasppi_1wire_actor extends Device {
    
    function __construct() {
        $this->system_info_available = true;
    }

    function setDefaultValues() {
        parent::setDefaultValues();
        $this->generic_device_specs['version']='0.0.2';
        $this->generic_device_specs['name']='Raspberry PI 1-Wire Actor';
        $this->generic_device_specs['name_long']='1-Wire-Actor connected to Raspberry PI.';
        $this->generic_device_specs['description']='Hot. Very hot!';
        $this->generic_device_specs['appfile']='rasppi_1wire_actor.inc';
        $this->device_defaults['description']='1-Wire-Actor connected to Raspberry PI.';
        //$this->device_config_defaults['L2P-Driver-Path']='/home/clauss/git-repos/ondics-lufft-opus20/';
        $this->device_config_defaults['IP-Address']=array('value'=>'localhost','attribs'=>'');
        $this->device_config_defaults['NumValues']=array('value'=>'1','attribs'=>'visible:no');
        $this->device_config_defaults['0_1-Wire Adress']=array('value'=>'0000-000-000','attribs'=>'');  
        $this->device_config_defaults['0_Description']=array('value'=>'Sensor-Description','attribs'=>'');
        $this->device_config_defaults['0_Formula']=array('value'=>'round($value,3,PHP_ROUND_HALF_UP)','attribs'=>'');
        // attention: if SysInfo is set to yes, function getSystemInfoInHTML has to be implememted
        $this->device_config_defaults['SysInfo']=array('value'=>'yes','attribs'=>'visible:no');
        // attention: if Actor is set to yes, function setOutputValue has to be implememted
        $this->device_config_defaults['Actor']=array('value'=>'yes','attribs'=>'actor_low:0|actor_high:1|visible:no');
        
        parent::checkDefaultValues();
    }        
    
    function getValuesFromDevice() {
        $this->error="";
        if (!parent::isLoaded()) {$this->error.="error: device not loaded";return; }

        $this->error="not implemented yet";
        return $this->error;       
    }
    
    static function getSystemInfoInHTML() {
        $html="<p>not yet implemented, sorry!</p>";
        return $html;
  }

    function setActorValue() {
        $this->error="";
        if (!parent::isLoaded()) {$this->error.="error: device not loaded";return; }

        $this->error="not implemented yet";
        return $this->error;       
    }
    
}

?>
