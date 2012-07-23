<?php
// needs classinc to be set!


class avrnetio extends Device {

    public $values = array();
    public $error = "";

    function setDefaultValues() {
        $this->generic_device_specs = array(
            'version'=>'0.0.1',
            'name'=>'AVR Net IO',
            'name_long'=>'Pollin-Board',
            'description'=>'Super Gerät, total billig.',
            'url'=>'http://srv1.ondics.de/appstore/devicehelp.html',
            'appfile'=>'avrnetio.inc'
        );
            
        $this->device_defaults = array(
            'name'=>$this->generic_device_specs['name'],
            'description'=>'Gerät wurde 2011 von Polin bezogen (Inventarnummer #1333)',
            'location'=>'Gerät ist aufgestellt in Straße/PLZ/Ort',
            'interval_min'=>5,
            'gdc_sid'=>'===***unset***===',
            'generic_device_name'=>$this->generic_device_specs['name']
        );
        
        $this->device_config_defaults = array(
            'IP-Address'=>'0.0.0.0',
            'Port'=>50290,
            'NumValues'=>4,
            '0_Channel'=>'1',
            '0_Description'=>'ADC1',
            '0_Formula'=>'$value',
            '1_Channel'=>'2',
            '1_Description'=>'ADC2',
            '1_Formula'=>'$value',
            '2_Channel'=>'3',
            '2_Description'=>'ADC3',
            '2_Formula'=>'$value',
            '3_Channel'=>'4',
            '3_Description'=>'ADC4',
            '3_Formula'=>'$value'
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
            // see AVR Net IO manual, page 34
            $request= 'GETADC '.$this->device_config_values[$i.'_Channel']."\r\n";
                     
            //echo "Sending [".substr($request,0,-1)."] to server.\n";
            socket_write($socket, $request, strlen($request));
            $input = trim(socket_read($socket, 512 , PHP_NORMAL_READ));
            //echo "Response from server is: [".$input."]\n";

            // now do some postprocessing
            if (is_numeric($input)) {
                $value=$input;
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
