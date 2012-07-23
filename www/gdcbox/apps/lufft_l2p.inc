<?php
// needs classinc to be set!


class lufft_l2p extends Device {

    public $values = array();
    public $error = "";

    function setDefaultValues() {
        $this->generic_device_specs = array(
            'version'=>'0.0.1',
            'name'=>'Lufft L2P',
            'name_long'=>'Lufft L2P Device, z.B. ein OPUS20',
            'description'=>'weiße Ware für Indoor!',
            'url'=>'http://srv1.ondics.de/appstore/devicehelp.html',
            'appfile'=>'lufft_l2p.inc'
        );
            
        $this->device_defaults = array(
            'name'=>$this->generic_device_specs['name'],
            'description'=>'Gerät wurde 2012 von Lufft zu Testzwecken überlassen',
            'location'=>'Gerät ist aufgestellt in Straße/PLZ/Ort',
            'interval_min'=>1,
            'gdc_sid'=>'===***unset***===',
            'generic_device_name'=>$this->generic_device_specs['name']
        );
        
        $this->device_config_defaults = array(
            'IP-Address'=>'0.0.0.0',
            'Port'=>52015,
            'NumValues'=>7,
            '0_Channel'=>'A0 00',  // in reverse hex!
            '0_Description'=>'Temperature avg. in °C',
            '0_Formula'=>'$value',
            '1_Channel'=>'2C 01',
            '1_Description'=>'abs. air pressure in hPa',
            '1_Formula'=>'$value',  
            '2_Channel'=>'C8 00',
            '2_Description'=>'relative humidity in %',
            '2_Formula'=>'$value',  
            '3_Channel'=>'6E 00',
            '3_Description'=>'dewpoint in %',
            '3_Formula'=>'$value',  
            '4_Channel'=>'24 27',
            '4_Description'=>'battery voltage in V',
            '4_Formula'=>'$value',  
            '5_Channel'=>'31 01',
            '5_Description'=>'rel. air pressure in hPa',
            '5_Formula'=>'$value',  
            '6_Channel'=>'CD 00',
            '6_Description'=>'absolute humidity in %',
            '6_Formula'=>'$value',  
            
        );        
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
            $l2p_frame=shell_exec('echo "'.$nettodaten.'" | /home/clauss/git-repos/ondics-lufft-opus20/l2p-frame -x ');
                                 
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
            $value=shell_exec('echo "'.$hexstring.'" | /home/clauss/git-repos/ondics-lufft-opus20/l2p-result -a ');
            echo "value=[$value]\n";

            // now do some postprocessing
            if (is_numeric($value)) {
                $phpcode='$value=('.$this->device_config_values[$i."_Formula"].');';
                //echo "postprocessing: [".$phpcode."]\n";
                $eval_ok=eval($phpcode);
                if ($eval_ok===FALSE)
                    $this->error.=" eval failed for value [$i].";
                //echo "value=[$value]\n";
                $this->values[$i]=$value;
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
