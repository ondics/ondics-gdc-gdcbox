<?php



class rasppi_i2c extends Device {

    function setDefaultValues() {
        parent::setDefaultValues();
        $this->generic_device_specs['version']='0.1.0';
        $this->generic_device_specs['name']='Raspberry PI I2C';
        $this->generic_device_specs['name_long']='I2C-Bus connected to Raspberry PI.';
        $this->generic_device_specs['description']='Hot. Very hot!';
        $this->generic_device_specs['appfile']='rasppi-i2c.inc';
        $this->device_defaults['description']='I2C-Bus connected to Raspberry PI.';
        $this->device_config_defaults['Port']=array('value'=>'99999','attribs'=>'');
        $this->device_config_defaults['L2P-Driver-Path']=array('/home/clauss/git-repos/ondics-lufft-opus20/','attribs'=>'');
        $this->device_config_defaults['NumValues']=array('value'=>'2','attribs'=>'');
        $this->device_config_defaults['0_Channel']=array('value'=>'A0 00','attribs'=>'');  // in reverse hex!
        $this->device_config_defaults['0_Description']=array('value'=>'CHANNEL 1','attribs'=>'');
        $this->device_config_defaults['0_Formula']=array('value'=>'round($value,3,PHP_ROUND_HALF_UP)','attribs'=>'');
        $this->device_config_defaults['1_Channel']=array('value'=>'2C 01','attribs'=>'');
        $this->device_config_defaults['1_Description']=array('value'=>'CHANNEL 2','attribs'=>'');
        $this->device_config_defaults['1_Formula']=array('value'=>'round($value,3,PHP_ROUND_HALF_UP)','attribs'=>'');
        parent::checkDefaultValues();
    }        
    
    function getValuesFromDevice() {
        $this->error="";
        if (!parent::isLoaded()) {$this->error.="error: device not loaded";return; }
        
        // trace socket operations exactely!
        //error_reporting(E_ALL);
        $address = $this->device_config_values['IP-Address']['value'];
        $port = $this->device_config_values['Port']['value'];

        for ($i=0; $i<$this->device_config_values['NumValues']['value']; $i++) {
            // Create a TCP/IP socket.
            $socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
            if ($socket === false) { return $this->error.="error: socket_create() failed."; }
            // "Reason: " . socket_strerror(socket_last_error()) . "\n";
            
            if (socket_connect($socket, $address, $port) === false)
                return $this->error.="error: socket_connect() failed.";
            // "Reason: ($result) " . socket_strerror(socket_last_error($socket)) . "\n";

            // Nettodaten: (0x23,0x10,0xA0,0x00); // 0x00A0 .. durchschn. Temperatur
            //$l2p_frame=pack("C*",0x01,0x10,0x00,0x00,0x00,0x00,0x04,0x02,0x23,0x10,0xa0,0x00,0x03,0xf1,0xc4,0x04);
            $nettodaten="23 10 ".$this->device_config_values[$i.'_Channel']['value'];
            $l2p_frame=shell_exec('echo "'.$nettodaten.'" | '.
                                  $this->device_config_values['L2P-Driver-Path']['value'].'/l2p-frame -x ');
                                 
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
                              $this->device_config_values['L2P-Driver-Path']['value'].'/l2p-result -a ');
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