<?php
//require_once('classes.inc');


class lufft_l2p extends Device {

    function setDefaultValues() {
        parent::setDefaultValues();
        $this->generic_device_specs['version']='0.1.0';
        $this->generic_device_specs['name']='Lufft L2P';
        $this->generic_device_specs['name_long']='Lufft L2P Device. Climatological Measurements.';
        $this->generic_device_specs['description']='Hot. Very hot!';
        $this->generic_device_specs['appfile']='lufft_l2p.inc';
        $this->device_defaults['description']='OPUS20.THIP oder OPUS20...';
        $this->device_config_defaults['Port']=array('value'=>'52015','attribs'=>'');
        $this->device_config_defaults['L2P-Driver-Path']=array('value'=>'/home/clauss/git-repos/ondics-lufft-opus20/','attribs'=>'');
        $this->device_config_defaults['NumValues']=array('value'=>'7','attribs'=>'visible:no');
        $this->device_config_defaults['0_Channel']=array('value'=>'A0 00','attribs'=>'');  // in reverse hex!
        $this->device_config_defaults['0_Description']=array('value'=>'Temperature avg. in °C','attribs'=>'');
        $this->device_config_defaults['0_Formula']=array('value'=>'round($value,3,PHP_ROUND_HALF_UP)','attribs'=>'');
        $this->device_config_defaults['1_Channel']=array('value'=>'2C 01','attribs'=>'');
        $this->device_config_defaults['1_Description']=array('value'=>'abs. air pressure in hPa','attribs'=>'');
        $this->device_config_defaults['1_Formula']=array('value'=>'round($value,3,PHP_ROUND_HALF_UP)','attribs'=>'');
        $this->device_config_defaults['2_Channel']=array('value'=>'C8 00','attribs'=>'');
        $this->device_config_defaults['2_Description']=array('value'=>'relative humidity in %','attribs'=>'');
        $this->device_config_defaults['2_Formula']=array('value'=>'round($value,3,PHP_ROUND_HALF_UP)','attribs'=>'');
        $this->device_config_defaults['3_Channel']=array('value'=>'6E 00','attribs'=>'');
        $this->device_config_defaults['3_Description']=array('value'=>'dewpoint in %','attribs'=>'');
        $this->device_config_defaults['3_Formula']=array('value'=>'round($value,3,PHP_ROUND_HALF_UP)','attribs'=>'');
        $this->device_config_defaults['4_Channel']=array('value'=>'24 27','attribs'=>'');
        $this->device_config_defaults['4_Description']=array('value'=>'battery voltage in V','attribs'=>'');
        $this->device_config_defaults['4_Formula']=array('value'=>'round($value,3,PHP_ROUND_HALF_UP)','attribs'=>'');
        $this->device_config_defaults['5_Channel']=array('value'=>'31 01','attribs'=>'');
        $this->device_config_defaults['5_Description']=array('value'=>'rel. air pressure in hPa','attribs'=>'');
        $this->device_config_defaults['5_Formula']=array('value'=>'round($value,3,PHP_ROUND_HALF_UP)','attribs'=>'');
        $this->device_config_defaults['6_Channel']=array('value'=>'CD 00','attribs'=>'');
        $this->device_config_defaults['6_Description']=array('value'=>'absolute humidity in %','attribs'=>'');
        $this->device_config_defaults['6_Formula']=array('value'=>'round($value,3,PHP_ROUND_HALF_UP)','attribs'=>'');  
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
