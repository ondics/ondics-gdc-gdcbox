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
        $this->device_config_defaults['0_1-Wire Address']=array('value'=>'0000-000-000','attribs'=>'');  
        $this->device_config_defaults['0_Description']=array('value'=>'Sensor-Description','attribs'=>'');
        $this->device_config_defaults['0_Formula']=array('value'=>'round($value,3,PHP_ROUND_HALF_UP)','attribs'=>'');
        // attention: if SysInfo is set to yes, function getSystemInfoInHTML has to be implememted
        $this->device_config_defaults['SysInfo']=array('value'=>'yes','attribs'=>'visible:no');
        // attention: if Actor is set to yes, function setOutputValue has to be implememted
        $this->device_config_defaults['Actor']=array('value'=>'yes','attribs'=>'visible:no');
        $this->device_config_defaults['0_ActorRangeLow']=array('value'=>'0','attribs'=>'visible:no');
        $this->device_config_defaults['0_ActorRangeHigh']=array('value'=>'1','attribs'=>'visible:no');
        $this->device_config_defaults['0_ActorRangeType']=array('value'=>'int','attribs'=>'visible:no');
        
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

    function setActorValue($num,$value) {
        $this->error="";
        if (!parent::isLoaded()) {$this->error.="error: device not loaded";return; }

        $maxnum=$this->device_config_values['NumValues']['value'];
        if ($num<0 || $num>$maxnum-1)
            {$this->error.="num [$num] is not in range 0-".($maxnum-1);return $this->error; }
        
        $value_low=$this->device_config_values[$num.'_ActorRangeLow']['value'];
        $value_high=$this->device_config_values[$num.'_ActorRangeHigh']['value'];
        if ($value<$value_low || $value>$value_high)
            {$this->error.="value [$value] is not in range ".$value_low."-".$value_high;return $this->error; }

        // now do adjust the value
        $value_type=$this->device_config_values[$num.'_ActorRangeType']['value'];
        $value_corrected=0;
        switch ($value_type) {
            case 'int': $value_corrected = round($value,0,PHP_ROUND_HALF_UP); break;
            case 'float': $value_corrected = (float) $value; break;
            case 'bool': $value_corrected = (bool) $value; break;
            default: {$this->error.=" unknown type [$value_type].";return $this->error;}
        }
        
        // now write to actor
        $this->error="now setting 1-Wire Address ".
             $this->device_config_values[$num.'_1-Wire Address']['value'].
             " to value ".$value_corrected;
                
        $this->error.="  (not implemented yet)";


/*        
        // now update all values from sensor
        if ($error="") {
            $this->error=$device->getValuesFromDevice();
            if ($this->error=="") {
                // save values to db for later (api) access
                $device->saveValuesToDB();
            } else
                $this->error="actor failed to read values after setting";
        }
*/        
        return $this->error;       
    }
    
}

?>
