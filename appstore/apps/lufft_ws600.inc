<?php
// needs classinc to be set!


class lufft_ws600 extends Device {

    public $values = array();
    public $error = "";

    function setDefaultValues() {
        $this->generic_device_specs = array(
            'version'=>'0.0.1',
            'name'=>'Lufft WS600',
            'name_long'=>'Lufft WS 600 Präzisions-Outdoor-Wettmessstation',
            'description'=>'Super Gerät, sehr weiß.',
            'url'=>'http://srv1.ondics.de/appstore/devicehelp.html',
            'appfile'=>'lufft_ws600.inc'
        );
            
        $this->device_defaults = array(
            'name'=>$this->generic_device_specs['name'],
            'description'=>'Gerät wurde 2011 von Lufft bezogen (Inventarnummer #1234)',
            'location'=>'Gerät ist aufgestellt in Straße/PLZ/Ort',
            'interval_min'=>5,
            'gdc_sid'=>'===***unset***===',
            'generic_device_name'=>$this->generic_device_specs['name']
        );
        
        $this->device_config_defaults = array(
            'IP-Address'=>'0.0.0.0',
            'Port'=>1470,
            'DeviceID'=>'28673',
            'NumValues'=>2,
            '0_Channel'=>'00100',
            '0_Description'=>'Temperature',
            '0_Unit'=>'Grad Celsius',
            '0_Formula'=>'round((70-(-30))/65520*$value+(-30),4,PHP_ROUND_HALF_UP)',  // see umb-manual, page 34
            '1_Channel'=>'00200',
            '1_Description'=>'Rel. Humidity',
            '1_Unit'=>'%',
            '1_Formula'=>'round((100-(0))/65520*$value+(+0),4,PHP_ROUND_HALF_UP)'
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

            // now do some postprocessing
            if (is_numeric($input_parts[4])) {
                $value=$input_parts[4];
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
