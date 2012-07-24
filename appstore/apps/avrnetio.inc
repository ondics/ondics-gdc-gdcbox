<?php
//require_once('classes.inc');

class avrnetio extends Device {

    function setDefaultValues() {
        parent::setDefaultValues();        
        $this->generic_device_specs['version']='1.0.0';
        $this->generic_device_specs['name']='AVR Net IO';
        $this->generic_device_specs['name_long']='Pollin-Board mit AVR-Chip';
        $this->generic_device_specs['description']='Bestell-Nr. bei Pollin: 12345.67889. Super Gerät, total billig.';
        $this->generic_device_specs['url']='http://srv1.ondics.de/appstore/devicehelp.html';
        $this->generic_device_specs['appfile']='avrnetio.inc';
        $this->device_defaults['description']='AVR-Net-IO mit Beschaltung für...';
        $this->device_config_defaults['Port']=50290;
        $this->device_config_defaults['NumValues']=4;
        $this->device_config_defaults['0_Channel']='1';
        $this->device_config_defaults['0_Description']='ADC1';
        $this->device_config_defaults['0_Formula']='$value';
        $this->device_config_defaults['1_Channel']='2';
        $this->device_config_defaults['1_Description']='ADC2';
        $this->device_config_defaults['1_Formula']='$value';
        $this->device_config_defaults['2_Channel']='3';
        $this->device_config_defaults['2_Description']='ADC3';
        $this->device_config_defaults['2_Formula']='$value';
        $this->device_config_defaults['3_Channel']='4';
        $this->device_config_defaults['3_Description']='ADC4';
        $this->device_config_defaults['3_Formula']='$value';
        parent::checkDefaultValues();
    }
    
    function getValuesFromDevice() {
        $this->error="";
        if (!parent::isLoaded()) {$this->error.="error: device not loaded";return; }
        
        // trace socket operations exactely!
        //error_reporting(E_ALL);
        $address = $this->device_config_values['IP-Address'];
        $port = $this->device_config_values['Port'];

        for ($i=0; $i<$this->device_config_values['NumValues']; $i++) {
            // Create a TCP/IP socket.
            $socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
            if ($socket === false) { return $this->error.="error: socket_create() failed."; }
            // "Reason: " . socket_strerror(socket_last_error()) . "\n";
            
            if (socket_connect($socket, $address, $port) === false)
                return $this->error.="error: socket_connect() failed.";
            // "Reason: ($result) " . socket_strerror(socket_last_error($socket)) . "\n";

            //echo "requesting [".$this->device_config_values[$i.'_Description']."]\n";
            // make an umb ascii request with pattern "& <ID> M <CHANNEL>CR"
            // see AVR Net IO manual, page 34
            $request= 'GETADC '.$this->device_config_values[$i.'_Channel']."\r\n";
                     
            //echo "Sending [".substr($request,0,-1)."] to server.\n";
            socket_write($socket, $request, strlen($request));
            $value = trim(socket_read($socket, 512 , PHP_NORMAL_READ));
            //echo "Response from server is: [".$input."]\n";

            // now do some postprocessing
            if (is_numeric($value)) {
                $this->error .= parent::getValuesFromDevicePostprocessingValue($i,$value);
            } else {
                $this->values[$i]=0;
                $this->error.=" value [$i] ist not numeric.";
            }

            socket_close($socket);
            sleep(1);

        }
        return $this->error;       
    }    
    
}

?>
