<?php

class rasppi_i2c_lm75_temperature extends Device {
    
    function __construct() {
        $this->system_info_available = true;
    }

    function setDefaultValues() {
        parent::setDefaultValues();
        $this->generic_device_specs['version']='0.0.1';
        $this->generic_device_specs['name']='Raspberry PI i2c LM75 Temperature';
        $this->generic_device_specs['name_long']='LM75-Sensor connected to Raspberry PI on I2C BUS.';
        $this->generic_device_specs['description']='LM75-App';
        $this->generic_device_specs['appfile']='rasppi_i2c_lm75_temperature';
        $this->device_defaults['description']='LM75-i2c-Sensor connected to Raspberry PI.';
        $this->device_config_defaults['IP-Address']=array('value'=>'localhost','attribs'=>'');
        $this->device_config_defaults['NumValues']=array('value'=>'1','attribs'=>'visible:no');
        $this->device_config_defaults['0_LM75_Adress']=array('value'=>'0x00','attribs'=>'');  
        $this->device_config_defaults['0_Description']=array('value'=>'Sensor-Description','attribs'=>'');
        $this->device_config_defaults['0_Formula']=array('value'=>'$value','attribs'=>'');
        // attention: if SysInfo is set to yes, function getSystemInfoInHTML has to be implememted
        $this->device_config_defaults['SysInfo']=array('value'=>'yes','attribs'=>'visible:no');
        // attention: if Actor is set to yes, function getSystemInfoInHTML has to be implememted
        $this->device_config_defaults['Actor']=array('value'=>'no','attribs'=>'rangelow:0|rangehigh:1');
        
        parent::checkDefaultValues();
    }        
    
    function getValuesFromDevice() {
        $this->error="";
        if (!parent::isLoaded()) {$this->error.="error: device not loaded";return; }
        
        //Hexwert wird per i2cget vom Device geholt.
        $var = shell_exec("/usr/sbin/i2cget -y 0 ". $this->device_config_values['0_LM75_Adress']['value'] ." 0x00 w");
        //Hexwert wird am "x" getrennt und geprüft ob der vordere Wert (erste zwei Zeichen) den Wert (dec) 255 hat.
        //Falls dieser 255 ist, bedeutet dies laut Datenblatt, dass sich der hintere Wert (die temperatur)
        //Um 0,5 erhöht.
        $var = explode("x",$var);
        $varsub_vorn = substr($var[1],0,2);
        $varsub_hinten = substr($var[1],2,4);
        $schritt = hexdec($varsub_vorn);
        $temp = hexdec($varsub_hinten);
        if ($schritt == 255) {
            $temp = $temp . '.5';
        }
        //Temp,Wert wird übergeben
        $this->error .= parent::getValuesFromDevicePostprocessingValue(0, $temp);
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

}


?>
