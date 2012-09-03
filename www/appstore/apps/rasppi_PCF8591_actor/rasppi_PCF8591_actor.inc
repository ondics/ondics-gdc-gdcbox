<?php

class rasppi_PCF8591_actor extends Device {
    
    function __construct() {
        $this->system_info_available = true;
    }

    function setDefaultValues() {
        parent::setDefaultValues();
        $this->generic_device_specs['version']='0.0.1';
        $this->generic_device_specs['name']='PCF8591 Analog Output';
        $this->generic_device_specs['name_long']='PCF8591 Set Value to Analog Output';
        $this->generic_device_specs['description']='Analog Output';
        $this->generic_device_specs['appfile']='rasppi_PCF8591_actor';
        $this->device_defaults['description']='PCF8591';
        $this->device_config_defaults['IP-Address']=array('value'=>'localhost','attribs'=>'');
        $this->device_config_defaults['NumValues']=array('value'=>'1','attribs'=>'visible:no');
        $this->device_config_defaults['0_PCF8591_Adress']=array('value'=>'00','attribs'=>'');  
        $this->device_config_defaults['0_Description']=array('value'=>'Sensor-Description','attribs'=>'');
        $this->device_config_defaults['0_Formula']=array('value'=>'round($value,3,PHP_ROUND_HALF_UP)','attribs'=>'visible:no');
        // attention: if SysInfo is set to yes, function getSystemInfoInHTML has to be implememted
        $this->device_config_defaults['SysInfo']=array('value'=>'yes','attribs'=>'visible:no');
        // attention: if Actor is set to yes, function setOutputValue has to be implememted
        $this->device_config_defaults['Actor']=array('value'=>'yes','attribs'=>'visible:no');
        $this->device_config_defaults['0_ActorRangeLow']=array('value'=>'0','attribs'=>'visible:no');
        $this->device_config_defaults['0_ActorRangeHigh']=array('value'=>'255','attribs'=>'visible:no');
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
        
        $aryDelete = array(1 => " ", 2 => "0123456789abcdef", 3 => "\n", 4 => "-");

        $aryReplace = array(1 => "");
        $pattern = "([0-9]+:)";

        $strI2cAll = shell_exec('/usr/sbin/i2cdetect -y 0');
        $strI2cAll = str_replace($aryDelete,$aryReplace,$strI2cAll);
        $strI2cAll = preg_replace($pattern,"",$strI2cAll);

        $länge = strlen($strI2cAll) / 2;
        
        $html = "<p>Available I2C - Devices on your Raspberry PI: <br>";
        
        for($i = 0; $i < $länge; $i++){
    
            $html = $html . "Device " . $i . " : 0x" . substr($strI2cAll,0,2) . "<br>";
            $strI2cAll = substr($strI2cAll,2,strlen($strI2cAll) -2);
    
        }
        
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
        
        
        /*
        // now write to actor
        $this->error="now setting GPIO ".
             $this->device_config_values[$num.'_GPIO_Address']['value'].
             " to value ".$value_corrected;
                
        $this->error.="  (not implemented yet)";
        */
        
        $pcf_adress = $this->device_config_values[$num . '_PCF8591_Adress']['value'];
        shell_exec('/usr/sbin/i2cset -y 0 ' . $pcf_adress . ' ' . '0x40' . ' ' . $value);
        

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
