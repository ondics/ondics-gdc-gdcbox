<?php
//require_once('classes.inc');

class lufft_ws600 extends Device {

    function setDefaultValues() {
        parent::setDefaultValues();
        $this->generic_device_specs['version']='0.0.1';
        $this->generic_device_specs['name']='Lufft WS600';
        $this->generic_device_specs['name_long']='Lufft WS 600 Präzisions-Outdoor-Wettmessstation';
        $this->generic_device_specs['description']='Super Gerät, sehr weiß.';
        $this->generic_device_specs['appfile']='lufft_ws600.inc';
        $this->device_defaults['description']='Lufft WS 600, Montiert auf...';
        $this->device_config_defaults['Port']=1470;
        $this->device_config_defaults['DeviceID']='28673';
        $this->device_config_defaults['NumValues']=2;
        $this->device_config_defaults['0_Channel']='00100';
        $this->device_config_defaults['0_Description']='Temperature';
        $this->device_config_defaults['0_Unit']='Grad Celsius';
        $this->device_config_defaults['0_Formula']='round((70-(-30))/65520*$value+(-30),4,PHP_ROUND_HALF_UP)';  // see umb-manual; page 34
        $this->device_config_defaults['1_Channel']='00200';
        $this->device_config_defaults['1_Description']='Rel. Humidity';
        $this->device_config_defaults['1_Unit']='%';
        $this->device_config_defaults['1_Formula']='round((100-(0))/65520*$value+(+0),4,PHP_ROUND_HALF_UP)';
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
            // see umb-manual, page 34
            $request= '& '.$this->device_config_values['DeviceID'].
                      " M ".$this->device_config_values[$i.'_Channel']." \n";
                     
            //echo "Sending [".substr($request,0,-1)."] to server.\n";
            socket_write($socket, $request, strlen($request));
            $input = trim(socket_read($socket, 512 , PHP_NORMAL_READ));
            //echo "Response from server is: [".$input."]\n";
            $input_parts=explode(" ",$input);
            $value=$input_parts[4];
            
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
