<?php

    require_once("../gdcbox/platforms.inc");

    // we need classinc for device instantiation
    require_once($env["classinc"]);
    
    // build array $applist containing all apps (from app directory):
    // how? we extract 'name', 'version', 'platforms' and 'appfile' from each class
    // 'platforms' has format "platform1,platform2,..." or "ANY" if platform independend
    // supported platforms for gdcbox are defined in platforms.inc
    $applist=array();
    foreach(glob('./apps/*/*.inc') as $file) {
        //echo "<p>file=[$file]</p>";
        //require_once($file);
        $file_content=file_get_contents($file);
        //if (!preg_match("/^\s*class\s*(?'classname'\S*)\s*Device/",$file_content,$matches))
        if (!preg_match("/(class[\s]*)([\w]*)([\s]*extends[\s]*)/",$file_content,$matches)) 
            { echo "<p>no class found</p>"; continue; }
        require_once($file);
        //echo "<p>matches:<br>";var_dump($matches);echo "</p>";
        $classname=$matches[2]; // just classname is relevant!
        $device=new $classname();
        $device->setDefaultValues();
        $applist[]=array('name'     => $device->generic_device_specs['name'],
                         'version'  => $device->generic_device_specs['version'],
                         'platforms'=> $device->generic_device_specs['platforms'],
                         'file'     => $device->generic_device_specs['appfile']);
        unset($device);
    }
    
    //
    // appstore-main processing
    //
    $action=isset($_GET['action'])?htmlentities($_GET['action'],ENT_QUOTES):'';
    $machine_os=isset($_GET['machine_os'])?htmlentities($_GET['machine_os'],ENT_QUOTES):'';
    
    switch ($action) {
        // applist returns json-encoded list with all apps
        // available for specified platform
        // request: action=applist&machine_os=<platform>
        // <platform> is optional and may be raspberry, ubuntu, openwrt,... 
        case 'applist':
            $applist_platform=array();
            foreach($applist as $app) {
                $platforms=explode(",",$app["platforms"]);
                if ($platforms[0]=="ANY" || in_array($machine_os,$platforms))
                    $applist_platform[]=$app;
            }
            echo json_encode($applist_platform);
            break;
        
        
        
        // download returns requested app as binary file package (<file>.zip)
        // that contains at least ./<file>/<file>.inc
        // 
        // currently, only one file (*.inc) is transfered
        case 'download':
            $appfile=$_GET["appfile"];
            // prüfen, ob app auch in applist ist.
            $i=count($applist);
            while (--$i>=0)
                if ($applist[$i]['file'] == $appfile) break;
            if ($i<0) die("<p>error: app ".$appfile." nicht gefunden<p>");
            
            // is app already zipped? no, zip it first and create <appfile>.gz
            $app_dir=$env["basepath"]."/appstore/apps/$appfile";
            if (is_dir($app_dir)) {
                $shell_cmd="cd ".$env["basepath"]."/appstore/apps && tar zcf ./$appfile.gz ./$appfile";
                $result=shell_exec($shell_cmd);
                if ($result!="") die("error zipping");
            } else
                die("error: dir not found");
            $zip_archive_gz=$app_dir.".gz";
            // jetzt download starten            
            header('Content-Length: '.filesize($zip_archive_gz));
            header('Content-Type: application/x-download');
            header('Content-Disposition: attachment; filename="'.$appfile.'.gz"');
            header('Content-Transfer-Encoding: binary');
            $fp=fopen($zip_archive_gz,"rb");
            while($fp && !feof($fp) && (connection_status()==0)) {
                print(fread($fp, filesize($zip_archive_gz)));
                flush();
            }
            fclose($fp);
            break;

        case 'userapp-publish':
            
            $username=isset($_POST['username'])?htmlentities($_POST['username'],ENT_QUOTES):'';
            $email=isset($_POST['email'])?htmlentities($_POST['email'],ENT_QUOTES):'';
            $appname=isset($_POST['appname'])?htmlentities($_POST['appname'],ENT_QUOTES):'';
            $description=isset($_POST['description'])?htmlentities($_POST['description'],ENT_QUOTES):'';
            
            if ($username=="" || $email=="" || $description=="") {
                echo "error: Please fill out all fields";
                break;
            }

            $publish_base = "/home/clauss/gdcbox-appstore-incoming/";
            $publish_dir = $publish_base.date("Ymd-His")."-incoming-app/";
            if (is_dir($publish_dir)) {
                echo "error: dir exists.\n";
                break;
            }
            if (!mkdir($publish_dir)) {
                echo "error: cannot make dir.\n";
                break;
            }
            // first save information about publisher
            $content =  "New published App\n".
                        "=================\n".
                        "published  : ".date("Y-m-d H:i:s")."\n".
                        "user       : ".$username."\n".
                        "email      : ".$email."\n".                        
                        "appname    : ".$appname."\n".
                        "description\n".
                        "-----------\n".
                        wordwrap($description,50)."\n\n".
                        "Sonstiges\n".
                        "---------\n";
            foreach($_SERVER as $key => $value)
                $content .= $key.' = ['.$value.']'."\n";
                
            file_put_contents($publish_dir."readme.txt",$content);

            // then save file
            $publish_file = $publish_dir . basename($_FILES['file_contents']['name']);
            if (!move_uploaded_file($_FILES['file_contents']['tmp_name'], $publish_file)) {
                echo "ok. published.\n";
            } else {
                echo "error!\n";
            }
            
            // finally send incoming email
            $mail_subject="neue gdcapp wurde publiziert";
            $mail_to="w@ondics.de";
            $headers   = array();
            $headers[] = "MIME-Version: 1.0";
            $headers[] = "Content-type: text/plain; charset=iso-8859-1";
            $headers[] = "From: GDCBox Appstore <clauss@srv1.ondics.de>";
            $headers[] = "Reply-To: support@ondics.de";
            $headers[] = "Subject: {$subject}";
            $headers[] = "X-Mailer: PHP/".phpversion();

            $result=mail($mail_to, $mail_subject, $content, implode("\r\n", $headers));
            
            break;
        
        default:
            die("<p>error in appstore: unknown command</p>");
    }

?>
