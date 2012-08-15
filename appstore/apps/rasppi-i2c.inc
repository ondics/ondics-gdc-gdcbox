<?php



class rasppi-i2c extends Device {

    function setDefaultValues() {
        parent::setDefaultValues();
        $this->generic_device_specs['version']='0.1.0';
        $this->generic_device_specs['name']='Raspberry PI I2C';
        $this->generic_device_specs['name_long']='I2C-Bus connected to Raspberry PI.';
        $this->generic_device_specs['description']='Hot. Very hot!';
        $this->generic_device_specs['appfile']='rasppi-i2c.inc';
        $this->device_defaults['description']='I2C-Bus connected to Raspberry PI.';
        $this->device_config_defaults['Port']=99999;
        $this->device_config_defaults['L2P-Driver-Path']='/home/clauss/git-repos/ondics-lufft-opus20/';
        $this->device_config_defaults['NumValues']=2;
        $this->device_config_defaults['0_Channel']='A0 00';  // in reverse hex!
        $this->device_config_defaults['0_Description']='CHANNEL 1';
        $this->device_config_defaults['0_Formula']='round($value,3,PHP_ROUND_HALF_UP)';
        $this->device_config_defaults['1_Channel']='2C 01';
        $this->device_config_defaults['1_Description']='CHANNEL 2';
        $this->device_config_defaults['1_Formula']='round($value,3,PHP_ROUND_HALF_UP)';
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

            // Nettodaten: (0x23,0x10,0xA0,0x00); // 0x00A0 .. durchschn. Temperatur
            //$l2p_frame=pack("C*",0x01,0x10,0x00,0x00,0x00,0x00,0x04,0x02,0x23,0x10,0xa0,0x00,0x03,0xf1,0xc4,0x04);
            $nettodaten="23 10 ".$this->device_config_values[$i.'_Channel'];
            $l2p_frame=shell_exec('echo "'.$nettodaten.'" | '.
                                  $this->device_config_values['L2P-Driver-Path'].'/l2p-frame -x ');
                                 
            //echo "frame-laenge=".strlen($l2p_frame)."\n";
            $sent=socket_write($socket, $l2p_frame, strlen($l2p_frame));
            if ($sent === false)
                $this->error.="error: socket_write() failed.";
                
            // now reading response
            $input = socket_read($socket, 512);

            $unpacked=unpack("C*",$input);
            $hexstring="";
            foreach ($unpacked as $val) $hexstring.=sprintf("%02X ",$val);
            //echo "hexstring=[$hexstring]\n";
            $value=shell_exec('echo "'.$hexstring.'" | '.
                              $this->device_config_values['L2P-Driver-Path'].'/l2p-result -a ');
            //echo "value=[$value]\n";

            // now do some postprocessing
            if (is_numeric($value)) {
                $this->error .= parent::getValuesFromDevicePostprocessingValue($i,$value);
            } else {
                $this->values[$i]=0;
                $this->error.=" value [$i] ist not numeric.";
                $this->error.="result from device is [$hexstring]. ";
            }

            socket_close($socket);
            sleep(3);

        }
        return $this->error;       
    }    
    
}

?>
