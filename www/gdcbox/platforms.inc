<?php

    $version="2.0a";
    $testmode=false; // einblenden diversen Testhilfen ("Test" in Footer, DB-Display)
    $testmsg="";     // wird eingeblendet im footer

    // determine platform
    $system_out=exec('uname -a');
    if ($system_out===FALSE) die("error: system-command failed\n");
    if (preg_match('/(raspberry|pi)/i',$system_out))
        $machine_os="raspberry-pi";
    else if (preg_match('/openwrt/i',$system_out))
        $machine_os="openwrt";
    else if (preg_match('/linux srv1/i',$system_out))
        $machine_os="ubuntu-turnkey";            
    else
        die("error: platform unsupported\n");

    switch ($machine_os) {
        case 'ubuntu-turnkey':
            $env["basepath"] = "/home/clauss/git-repos/ondics-gdc-gdcbox/www";    
            $env["baseurl"] = "/gdcbox";
            break;
        case 'raspberry-pi':
            $env["basepath"] = "/var/www/gdcbox/ondics-gdc-gdcbox/www";
            $env["baseurl"] = "/gdcbox";
            break;

        default:
    }
    
    // *** now complete paths & urls
    // set local paths
    $env["database"] = $env["basepath"]."/gdcbox/gdcbox-db.sqlite";
    $env["classinc"] = $env["basepath"]."/gdcbox/classes.inc";
    $env["apppath"]  = $env["basepath"]."/gdcbox/apps";
    // set local urls
    $env["myurl"] = $env["baseurl"]."/cgi-bin/gdcbox.php";
    // appstore access
    $env["appstore_url"] = "http://srv1.ondics.de/gdcbox/appstore/appstore.php";
    $env["appstore_user"] = "appstore";
    $env["appstore_pass"] = "appstore";
    // local cronjob-script
    $env["tmp_dir"] = "/tmp";
    $env["cronjob_execdir"] = $env["basepath"]."/gdcbox/";
    $env["cronjob_script"] = 'gdcbox_cronjob.php';
    $env["cronjob_script_path"] = $env["basepath"].'/gdcbox/'.$env["cronjob_script"];
    $env["cronjob_logfile"] = $env["tmp_dir"]."/gdcbox_cronjob.log";
    // gdc-api access
    $env["gdc_baseurl"] = "http://gdc.ondics.de/gdc-da.php";
    // logging for gdcbox api-access
    $env["api_logfile"] = $env["tmp_dir"]."/gdcbox_api.log";
    $env["api_url"] = $env["baseurl"]."/cgi-bin/gdcbox-api.php";

?>