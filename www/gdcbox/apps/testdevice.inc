<?php
// needs classinc to be set!

class testdevice extends Device {

    function setDefaultValues() {
        echo "<p>testdevice::setDefaultValues()</p>";
        $this->generic_device_specs = array(
            'version'=>'0.0.1',
            'name'=>'Testdevice',
            'name_long'=>'Testdevice',
            'description'=>'sollte nur zum Testen installiert werden',
            'url'=>'http://srv1.ondics.de/appstore/devicehelp.html',
            'appfile'=>'testdevice.inc'
        );
            
        $this->device_defaults = array(
            'name'=>$this->generic_device_specs['name'],
            'description'=>'Nur so ein Testdevice zum Testen. Liefert als Messwert nur 1',
            'location'=>'im Netz, jedenfalls nicht im Besprechnungszimmer',
            'interval_min'=>5,
            'gdc_sid'=>'===***unset***===',
            'generic_device_name'=>$this->generic_device_specs['name']
        );
        
        $this->device_config_defaults = array(
            'IP-Address'=>'0.0.0.0',
            'Port'=>0,
            'NumValues'=>1,
            '0_Description'=>'Immer der Wert 123',
            '0_Unit'=>'keine Einheit',
            '0_Formula'=>'$value+1'
        );
    }

    function getValuesFromDevice() {
        $error="";
        if (!parent::isLoaded()) {$error.="error: device not loaded";return; }
        for ($i=0; $i<$this->device_config_values['NumValues']; $i++) {
            $value=123;
            // now do postprocessing
            $phpcode='$value=('.$this->device_config_values[$i."_Formula"].');';
            $eval_ok=eval($phpcode);
            if ($eval_ok===FALSE)
                $error.=" eval failed for value [$i].";
            $this->values[$i]=$value;
        }
        return $error; 
    }
}

?>
