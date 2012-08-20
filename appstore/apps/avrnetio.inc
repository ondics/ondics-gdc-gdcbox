<?php

// todo:nutzer der avnetio-klasse von
// http://www.saschakimmel.de/assets/AvrNetIo.phps
// beschrieben in http://www.saschakimmel.de/2010/02/avr-net-io-mit-php-ansteuern

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
        $this->device_config_defaults['IP-Address'] =array('value'=>'localhost','attribs'=>'');        
        $this->device_config_defaults['Port']       =array('value'=>'50290','attribs'=>'');
        $this->device_config_defaults['NumValues']  =array('value'=>'4','attribs'=>'visible:no');
        $this->device_config_defaults['0_Channel']  =array('value'=>'1','attribs'=>'');
        $this->device_config_defaults['0_Description']=array('value'=>'ADC1','attribs'=>'');
        $this->device_config_defaults['0_Formula']  =array('value'=>'$value','attribs'=>'');
        $this->device_config_defaults['1_Channel']  =array('value'=>'2','attribs'=>'');
        $this->device_config_defaults['1_Description']=array('value'=>'ADC2','attribs'=>'');
        $this->device_config_defaults['1_Formula']  =array('value'=>'$value','attribs'=>'');
        $this->device_config_defaults['2_Channel']  =array('value'=>'3','attribs'=>'');
        $this->device_config_defaults['2_Description']=array('value'=>'ADC3','attribs'=>'');
        $this->device_config_defaults['2_Formula']  =array('value'=>'$value','attribs'=>'');
        $this->device_config_defaults['3_Channel']  =array('value'=>'4','attribs'=>'');
        $this->device_config_defaults['3_Description']=array('value'=>'ADC4','attribs'=>'');
        $this->device_config_defaults['3_Formula']  =array('value'=>'$value','attribs'=>'');
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

            //echo "requesting [".$this->device_config_values[$i.'_Description']."]\n";
            // make an umb ascii request with pattern "& <ID> M <CHANNEL>CR"
            // see AVR Net IO manual, page 34
            $request= 'GETADC '.$this->device_config_values[$i.'_Channel']['value']."\r\n";
                     
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
