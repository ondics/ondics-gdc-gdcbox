<?php



class rasppi_1wire_sensor_temperature extends Device {
    
    function __construct() {
        $this->system_info_available = true;
    }

    function setDefaultValues() {
        parent::setDefaultValues();
        $this->generic_device_specs['version']='0.0.2';
        $this->generic_device_specs['name']='Raspberry PI 1-Wire Temperature';
        $this->generic_device_specs['name_long']='1-Wire-Sensor connected to Raspberry PI.';
        $this->generic_device_specs['description']='Hot. Very hot!';
        $this->generic_device_specs['appfile']='rasppi_1wire_sensor_temperature';
        $this->device_defaults['description']='1-Wire-Sesnsor connected to Raspberry PI.';
        $this->device_config_defaults['IP-Address']=array('value'=>'localhost','attribs'=>'');
        $this->device_config_defaults['NumValues']=array('value'=>'1','attribs'=>'visible:no');
        $this->device_config_defaults['0_1-Wire_Adress']=array('value'=>'0000-000-000','attribs'=>'');  
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
        
        $zeilen = file('/sys/bus/w1/devices/' . $this->device_config_values['0_1-Wire_Adress']['value'] . '/w1_slave');
        //Shell_exec liefert 3 Zeilen zurück, in der zweiten Zeile
        // steht nach einem "t=" die temperatur.
        $aryTemp = explode("t=",$zeilen[1]);
        $temp = substr($aryTemp[1],0,2) . '.' . substr($aryTemp[1],2,2);

        $this->error .= parent::getValuesFromDevicePostprocessingValue(0, $temp);
        return $this->error;       
    }
    
    static function getSystemInfoInHTML() {
                
    $database = new SQlite3('/var/www/db/onewire_id.db');
    $sql = "SELECT * FROM onewire";
    $result = $database->query($sql);

    $family_adress = array();
    $device = array();
    $name = array();

    $i=0;
    while($res = $result->fetchArray(SQLITE3_ASSOC)){
 
        $family_adress[$i] = trim($res['family']);
        $device[$i] = trim($res['device']);
        $name[$i] = trim($res['name']);
        $i++;
    }

    $aryNumbers = array("0"=>"0","1"=>"1","2"=>"2","3"=>"3","4"=>"4","5"=>"5","6"=>"6","7"=>"7","8"=>"8","9"=>"9");

    //Directory auslesen in Array
    $alledateien = scandir("/sys/bus/w1/devices/");
    $html = "<p>Available 1-Wire Adresses on your Raspberry PI: <br>";
    //Nur Onewire Adressen in Array abspeichern
    $i = 0;

    foreach ($alledateien as $datei) {
 
        if(in_array($datei{0},$aryNumbers) == TRUE ){
            $family_number = trim(substr($datei,0,2));
                if(in_array($family_number, $family_adress)){
      
                     $index = array_search($family_number,$family_adress);
                    $html = $html . "<br>" . trim($datei) . "<br>__Name: " . $device[$index] . "<br>__Bezeichnung: " . $name[$index] . "<br>";
     
                }else{
                    $html = $html . "<br>" . trim($datei);
                }
        $i++;
        }
    }
    return $html;
  }

    
}

?>
