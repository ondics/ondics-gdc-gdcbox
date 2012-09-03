<?php

class rasppi_gpio_sensor extends Device {
    
    function __construct() {
        $this->system_info_available = true;
    }

    function setDefaultValues() {
        parent::setDefaultValues();
        $this->generic_device_specs['version']='0.0.1';
        $this->generic_device_specs['name']='Raspberry PI GPIO Auslesen';
        $this->generic_device_specs['name_long']='GPIO Pins Connected to Raspberry PI';
        $this->generic_device_specs['description']='Gpio 4 the Win :O';
        $this->generic_device_specs['appfile']='rasppi_gpio_sensor';
        $this->device_defaults['description']='GPIO Pins Connected to Raspberry PI';
        //$this->device_config_defaults['L2P-Driver-Path']='/home/clauss/git-repos/ondics-lufft-opus20/';
        $this->device_config_defaults['IP-Address']=array('value'=>'localhost','attribs'=>'visible:no');
        $this->device_config_defaults['NumValues']=array('value'=>'1','attribs'=>'visible:no');
        $this->device_config_defaults['GPIO_ADDRESS']=array('value'=>'00','attribs'=>'');  
        $this->device_config_defaults['0_Description']=array('value'=>'Sensor-Description','attribs'=>'');
        $this->device_config_defaults['0_Formula']=array('value'=>'$value','attribs'=>'visible:no');
        // attention: if SysInfo is set to yes, function getSystemInfoInHTML has to be implememted
        $this->device_config_defaults['SysInfo']=array('value'=>'yes','attribs'=>'visible:no');
        // attention: if Actor is set to yes, function getSystemInfoInHTML has to be implememted
        $this->device_config_defaults['Actor']=array('value'=>'no','attribs'=>'rangelow:0|rangehigh:1|visible:no');
        
        parent::checkDefaultValues();
    }        
    
    function getValuesFromDevice() {
        $this->error="";
        if (!parent::isLoaded()) {$this->error.="error: device not loaded";return; }
        
        //Direction zum auslesen muss vor dem erfassen des Wertes gesetzt werden.
        file_put_contents('/sys/class/gpio/gpio' . $this->device_config_values['GPIO_ADDRESS']['value'] . '/direction', 'in');
        //Wert (0 oder 1) wird eingelesen und getrimmt, da sich ein leerzeichen am Ende befinden kann.
        $value = trim(file_get_contents('/sys/class/gpio/gpio' . $this->device_config_values['GPIO_ADDRESS']['value'] . '/value'));

        $this->error .= parent::getValuesFromDevicePostprocessingValue(0, $value);
        
        return $this->error;       
    }
    
    static function getSystemInfoInHTML() {
                
    //Verfügbare GPIO Pins einlesen
    $aryAlledateien = scandir("/sys/class/gpio/");

    //"gpio" Tag aus Dateinamen entfernen um nur Busadresse zu erhalten
    $i = 0;
    foreach ($aryAlledateien as $datei) {
        if(strstr($datei,"gpio") == TRUE){
            if(strstr($datei,"chip") == FALSE){
                $arySensors[$i] = str_replace("gpio", "",trim($datei));
                $i++;
            }
        }
    }
    
    $html = "<p>Available GPIO - Pins on your Raspberry PI: <br>";
    //Gefundene Pins in DB Eintragen
    foreach($arySensors as $pin){
        
        $html = $html . "GPIO " . $pin ."<br>";

    }
    $html.="</p>";
    
    return $html;
  }

    
}

?>
