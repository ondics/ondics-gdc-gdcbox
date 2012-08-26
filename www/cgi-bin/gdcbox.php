<!--#!/usr/bin/php-cgi-->
<?php
    /*
    //  GDCBox Frontend
    //  (C) Ondics,2012
    */


    // begin session handling
    session_start();
    $arySessionVars=array('dev-mode','lastaction');
    foreach( $arySessionVars as $var) {
        if (!isset($_SESSION[$var])) $_SESSION[$var] = "";
    }

    require_once("../gdcbox/platforms.inc");
    require_once($env["classinc"]);

    // datenbankzugriff herstellen
    if (! ($pdo=new PDO('sqlite:'.$env["database"])) ) {
        die("<html><body>error: db open</body></html>");
    }
    
    //////////////////////////////////////////////
    // declare some functions for global use
    //
    function getValueFromURLSave($key) {
        return isset($_GET[$key])?htmlentities($_GET[$key],ENT_QUOTES):''; 
    }

    // to stop cron, all lines containing "gdcbox_" OR a crontab comment "# gdcbox_" are deleted
    // comments are not working in cron with variable substitution.
    function cron_stop() {
        $cronjobs=(int)shell_exec('crontab -l|grep gdcbox_ |wc -l');
        echo "<p>Currently $cronjobs device".($cronjobs!=1?'s are':'is').' running</p>';
        $tmpfile='/tmp/gdcbox.'.getmypid();
        shell_exec('crontab -l|grep -v gdcbox_ > '.$tmpfile);
        shell_exec('crontab '.$tmpfile);
        unlink($tmpfile);
    }

    function cron_start() {
        global $pdo;
        global $env; 
        $query = $pdo->prepare("SELECT id,interval_min,name,active FROM devices ORDER BY id ASC");
        $query->execute();
        // to reduce load, each minute only one processes is started 
        $proc_num=0;
        $tmpfile='/tmp/gdcbox.'.getmypid();
        shell_exec('crontab -l > '.$tmpfile);
        $line_end=" # gdcbox_\n"; 
        file_put_contents($tmpfile,"################".$line_end,FILE_APPEND);
        // in variable substitution, comments are not allowed! so omitting $line_end!
        // lines are recognized with variable name containing "gdcbox_"!!!
        file_put_contents($tmpfile,"gdcbox_execdir=".$env["cronjob_execdir"]."\n",FILE_APPEND);
        file_put_contents($tmpfile,"gdcbox_logfile=".$env["cronjob_logfile"]."\n",FILE_APPEND);
        file_put_contents($tmpfile,"gdcbox_cronjob=".$env["cronjob_script_path"]."\n",FILE_APPEND);
        $crontime="";
        while ( $row = $query->fetch() ) {
            if ($row['active']=="no") continue;
            if (is_numeric($row['interval_min']))
                $crontime='*/'.$row['interval_min'].' * * * * ';
            else {
                // check if interval has crontab-time-format
                if (preg_match('/^(([^\s]+)\s){5}/',$row['interval_min']))
                    $crontime=$row['interval_min'];
                else {
                    echo "<p>Warning: Interval ".$row['name']. "(id=".$row['id'].") incorrect. ".
                         "Setting to 5 Minutes</p>";
                    $crontime  ='*/5 * * * * ';   
                }
            }
            // build crontab-entry
            $cronjob=$crontime.'(cd $gdcbox_execdir; $gdcbox_cronjob '.$row['id'].')'.
                     ' >> $gdcbox_logfile 2>&1 # '.$row['name'].$line_end;
            echo '<p>adding cronjob ['.$cronjob.']</p>';
            file_put_contents($tmpfile,$cronjob,FILE_APPEND);
            $proc_num++;
        }
        // now make temp-crontab to new crontab
        if ($proc_num) {
            shell_exec('crontab '.$tmpfile);
            unlink($tmpfile);
        }
        return $proc_num;
    }


    // page selector
    $action=isset($_GET['action'])?htmlentities($_GET['action'],ENT_QUOTES):'main';

    echo '<!DOCTYPE html PUBLIC "-//WAPFORUM//DTD XHTML Mobile 1.0//EN" "http://www.wapforum.org/DTD/xhtml-mobile10.dtd">';
    echo '<html xmlns="http://www.w3.org/1999/xhtml" lang="de" xml:lang="de">';
    echo '<head>';
    echo '<title>GDCBox</title>';
    echo '<meta charset="utf-8">';
    //echo '<meta name="viewport" content="width=device-width, initial-scale=1"> ';
    //echo '<meta name="viewport" > ';
    //echo '<meta name="viewport" content="width=device-width; initial-scale=1.0; maximum-scale=1.0; user-scalable=no">';
    echo '<meta name="viewport" content="width=device-width; initial-scale=0.5; minimum-scale=1.0; maximum-scale=4.0;user-scalable=yes">';

    echo '<link rel="stylesheet" href="'.$env["baseurl"].'/jqm/jquery.mobile-1.1.0/jquery.mobile-1.1.0.min.css" />';
    echo '<script src="'.$env["baseurl"].'/jqm/jquery-1.6.4.min.js"></script>';
    echo '<script src="'.$env["baseurl"].'/jqm/jquery.mobile-1.1.0/jquery.mobile-1.1.0.min.js"></script>';
    // gdc-specific styles
    echo '<link rel="stylesheet" type="text/css" href="'.$env["baseurl"].'/gdcbox/gdcbox.css">';

    echo '</head>';
    echo '<body>';


    // jquery themeing
    echo '<div data-role="page" class="type-interior">';

    // jquery: Header of Page
    echo '<div data-role="header" data-position="fixed" data-theme="b">';
    // there are actions where "back" should be home!
    $actionswithonlyhome=array('appstore_appinstall','appstore_appremove_ok',
                               'makenewdevice_ok','configuredevice_ok','removedevice',
                               'gdcbox_start','gdcbox_stop','devmode-toggle','');
    if ($action!="main") {
        if (!in_array($action,$actionswithonlyhome))
            echo '<a href="'.$env["myurl"].'" data-icon="back" data-rel="back">Back</a>';
        echo '<a href="'.$env["myurl"].'" data-icon="home">Home</a>';
    }
    echo '<h1>GDCBox</h1>';
    echo '</div><!-- /header -->';
/*    
    echo '<div data-role="header" data-position="fixed" data-theme="b">';
    echo '<h1>GDCBox</h1>';
    echo '</div><!-- /header -->';
*/

    // jquery: content
    echo '<div data-role="content" data-theme="b">';

         
    // fuer navigation im footer
    $zurueckaction='.';
    
    //////////////////////////////////////////
    //
    // main program starts here (all menu selections are here)
    //
    //////////////////////////////////////////
    
    switch ($action) {       
    
    case "main":
        
        // Date and time are unset if system is powered on
        if (strtotime(date("Y-m-d")) < strtotime("2012-05-01") ) {
            echo '<p>Date & Time seem not to is not be set. </p>';
            echo '<p>Set Date & Time now:</p>';
            echo '<form action="'.$env["myurl"].'" method="get">';
            echo '<table><tr><td>Date</td><td>Day <input name="day" size="2"> '.
                 'Month <input name="month" size="2"> '.
                 'Year <input name="year" value="2012" size="4"></td></tr>';
            echo '<tr><td>Time</td><td>Hour <input name="hour" size="2"> '.
                 'Minute <input name="min" size="2"></td></tr>';
            echo '<tr><td><input type="hidden" name="action" value="setdatetime">';
            echo '<input type="submit" value=" Datum und Zeit setzen "></td></tr></table>';
            echo "</form>\n";
        }
        

        // display all devices installed ("app-style")
        echo '<div data-role="header" data-theme="d"><h1>Devices</h1></div>';
        echo '<div class="ui-body ui-body-d">';
        
        echo '<p style="font-size:small;">Devices available on this GDCBox (sort by ';
        $appsort=getValueFromURLSave("appsort");
        $appsorturlstub='<a href="'.$env["myurl"].'?action=main&';
        echo $appsort=='active'?'active, ':$appsorturlstub.'appsort=active">active</a>, ';
        echo $appsort=='latest'?'latest, ':$appsorturlstub.'appsort=latest">latest</a>, ';
        echo $appsort=='app'?'App, ':$appsorturlstub.'appsort=app">App</a>, ';
        echo $appsort=='name'?'Name':$appsorturlstub.'appsort=name">Name</a>';
        echo ')</p>';
        switch ($appsort) {
            case 'active': $sqlorderby="d.active DESC"; break; // no after yes
            case 'latest': $sqlorderby="d.id DESC"; break; 
            case 'app': $sqlorderby="d.generic_device_name ASC"; break;
            default: $sqlorderby="d.name ASC";
        }
        $query = $pdo->prepare("SELECT gd.appfile, d.id, d.name, d.location, d.gdc_send, d.active ".
                               "FROM devices d, generic_devices gd ".
                               "WHERE d.generic_device_name=gd.name ".
                               "ORDER BY ".$sqlorderby);
        $query->execute();
        $row = $query->fetch();
        //var_dump($row);
        if (!$row) {
            echo '<p>No devices installed<p>';
        } else {
        
            // background color of apps
            define("GREEN","65e080");
            define("WHITE","ffffff");
            define("BLUE","6899d3");
            define("GREY","dddddd");
            
            echo '<table cellspacing="2"><tbody style="font-size:x-small;"><tr>';
            echo '<td width="20" bgcolor="#'.GREY.'"></td><td>inactive</td>';
            echo '<td width="5"></td>';            
            echo '<td width="20" style="border:1px solid #bbb;" bgcolor="#'.WHITE.'"></td><td>Device ok</td>';
            echo '<td width="5"></td>';
            echo '<td width="20" bgcolor="#'.BLUE.'"></td><td>GDC active</td>';
            echo '</tr></tbody></table>';
            echo '<p></p>';
        
            echo '<p><table class="app-table"><tr>';
    
            // layouting: one app needs width of image + 2 x margin + 30x left margin
            // -> break, when width is reached
            $app_image_width=114;
            $app_image_margin=5;
            // make layout a little responsive (iphone/android get little icons
            if (stripos($_SERVER['HTTP_USER_AGENT'],"android") ||
                stripos($_SERVER['HTTP_USER_AGENT'],"iPhone"))
                $app_image_size_percent=50;
            else
                $app_image_size_percent=100;
            
            $app_image_size=(int)($app_image_width*($app_image_size_percent/100));
            $app_width=(int)($app_image_width*($app_image_size_percent/100)+2*$app_image_margin);
            $font_size=$app_image_size_percent<100?"x-small":"small";

            $appcount=0;
            do {   
                $appcount++;
                /* break line depending on screen width (javascript required!) */
                echo "\n".'<script language="JavaScript">';
                echo 'width = window.innerWidth ;';
                // iphone bug?
                echo 'if ((width<20) || (width>1900)) { width=480;};';
                echo "x = 30 + ( $app_width * ". ($appcount-1) ." );";
                echo "xbreak =  (width-30)-180;";
                //echo 'alert("[x="+x+",width="+width+",xbreak="+xbreak+"]");';
                echo 'if (x>xbreak ) { document.write("</tr><tr>");}';
                echo "</script>\n";
                
                if ($row['active']=="no")           $bgcolor=GREY;  // inactive
                elseif ($row['gdc_send']=="yes")    $bgcolor=BLUE;  // GDC sending
                else                                $bgcolor=WHITE; // normal
                
                // set text to white or dark grey depending on bgcolor
                $textcolor=((int)("0x".$bgcolor<0x777777))?"ffffff":"000000"; 
            
                echo '<td width="'.($app_width).'" class="app-cell" '.
                     "style=\"background-color:#$bgcolor;\">";

                //echo '<a href="'.$env["myurl"].'?action=configuredevice&device_id='.$row['id'].'">';
                echo '<a href="'.$env["myurl"].'?action=configuredevice&device_id='.$row['id'];
                echo '" style="color:#'.$textcolor.';">';
            
                echo '<div class="app-cell-top" style="height:'.$app_image_size.'px;">';
                echo '<img src="'.$env["baseurl"].'/gdcbox/apps/'.$row[0].'/'.$row[0].'.png" '.
                        " width=\"$app_image_size\" height=\"$app_image_size\">";
                echo '</div>';
            
                echo '<div class="app-cell-bottom" >';
                echo '<p style="font-size:'.$font_size.';">'.$row['location'].'</p>';
                echo '<p style="font-size:'.$font_size.';">'.$row['name'].'</p>';
                echo '</div>';
            
                echo '</a>';
    
                echo '</td>'; // end of app-display
                
            }  while ($row = $query->fetch() );
            //if ($appcount % $column_per_row != 0) echo '</tr>';
            echo "</tr>";
        
            echo "</table></p>\n";
        }

        // display "make a new device or go to appstore?"
        $query = $pdo->prepare("SELECT count(*) FROM generic_devices");
        $query->execute();
        $row = $query->fetch();
        
        if ($row[0]>=1)
            echo '<p><a href="'.$env["myurl"].'?action=makenewdevice">Make new Device</a></p>';

        switch ($row[0]) {
            case 0: echo '<p>Currently there is <b>no app</b> installed.<br>'; break;
            case 1: echo '<p>Currently there is <b>one app</b> installed.<br>'; break;
            default: echo '<p>Currently there are <b>'.$row[0].' apps</b> installed. '; 
        }
        echo 'Goto <a href="'.$env["myurl"].'?action=appstore">GDCBox AppStore</a> to manage your Apps.</p>';
    
        echo "</div><p></p>";

        // development-mode?
        if ($_SESSION['dev-mode']=="on") {
            echo '<div data-role="header" data-theme="e"><h1>Development</h1></div>';
            echo '<div class="ui-body ui-body-d">';
            echo '<ul>';
            echo '<li>Show <a href="'.$env["myurl"].'?action=test-dbdump">Database Contents</a> (database dump)</li>';
            echo '<li>Show <a href="'.$env["myurl"].'?action=test-cronjoblogfile">Logfile of cron-Jobs</a></li>';
            echo '</ul>';
            echo '<p>You can <a href="'.$env["myurl"].'?action=userapp-upload">upload your own apps</a> to the GDCBox<p>';
            echo "</div><p></p>";
        }
        
        echo '<div data-role="header" data-theme="d"><h1>Operation</h1></div>';
        echo '<div class="ui-body ui-body-d">';

        $cronjobs=(int)shell_exec('crontab -l| grep gdcbox |wc -l');

        echo '<tr><td><p>GDCBox is <span style="color:'.($cronjobs>0?'green">':'red">not').' running.</span></p></td></tr>';
        // start/stop button
        echo '<table boder="0">';
        echo '<tr><td><form action="'.$env["myurl"].'" method="get">';
        echo '<input type="hidden" name="action" value="gdcbox_'.($cronjobs>0?'stop':'start').'">';
        echo '<input type="submit" value=" '.($cronjobs>0?'Stop':'Start').' "></form></td>';
        // when running, add restart-button
        if ($cronjobs>0) {
            echo '<td><form action="'.$env["myurl"].'" method="get">';
            echo '<input type="hidden" name="action" value="gdcbox_restart">';
            echo '<input type="submit" value=" Restart "></form></td>';
        }
        echo "</tr>";
        echo "</table></p>";
        echo "</div><p></p>";
        
        echo '<div data-role="header" data-theme="d"><h1>GDCBox System Information</h1></div>';
        echo '<div class="ui-body ui-body-d">';
        echo '<p><table border="0" style="font-size:small;">';
        echo '<tr><td align="right">Current Date & Time</td><td> <b>'.date("Y-m-d H:i:s").'</b></td></tr>';
        $data = shell_exec('uptime');
        $uptime = explode(' up ', $data);
        $uptime = explode(',', $uptime[1]);
        $uptime = $uptime[0].', '.$uptime[1];
        echo '<tr><td align="right">Uptime</td><td> <b>'.$uptime.'</b></td></tr>';
        echo '<tr><td align="right">IP-Address</td><td><b>'.$_SERVER['SERVER_ADDR'].'</b></td></tr>';
        echo '<tr><td align="right">Current User</td><td><b>'.shell_exec('whoami').'</b></td></tr>';
        
        echo "</table></p>";
        echo "</div><p></p>";
        
        break; 

    case "setdatetime":

        echo "<h2>Set Date&Time</h2>\n";
        
        $year=$_GET['year'];
        $month=$_GET['month'];
        $day=$_GET['day'];
        $hour=$_GET['hour'];
        $min=$_GET['min'];
        if (is_numeric($year) && is_numeric($month) && is_numeric($day)
            && is_numeric($hour) && is_numeric($min)
            && $year>=2012 && $month>0 && $month<=12  && $day>0 && $day<=31
            && $hour>=0 && $hour<=23 && $min>=0 && $min<=59) {
            $newdatetime=sprintf("%04d.%02d.%02d-%02d:%02d:00",$year,$month,$day,$hour,$min);
            system("date ".$newdatetime);
            echo "<p>Date and Time set to ".$newdatetime."</p>";
        } else {
            echo "<p>error: Date or Time invalid</p>";   
        }

        break; 

    case "appstore":
        
        // if dev-mode then use local appstore        
        if ($_SESSION['dev-mode']!="on") {
            $appstore_url=$env["appstore_url"];
            $title="Official GDCBox AppStore";
        }
        else {
            $appstore_url="http://localhost".$env["baseurl"]."/appstore/appstore.php";
            $title="Local AppStore for Development";
        }
        echo "<h2>$title</h2>\n";

        $appstore_url.="?action=applist&machine_os=".$machine_os;
        echo "<p>url=$appstore_url</p>";
        $ch = curl_init();
        curl_setopt($ch, CURLOPT_URL, $appstore_url);
        curl_setopt($ch, CURLOPT_HEADER, false);
        curl_setopt($ch, CURLOPT_HTTPHEADER, array('Content-Type: application/json'));
        curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_ANY); 
        curl_setopt($ch, CURLOPT_USERPWD, $env["appstore_user"].':'.$env["appstore_pass"]); 
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
        $return = curl_exec($ch);
        $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
        curl_close($ch);
        $applist=array();
        if ($httpCode=="200") {
            $applist=json_decode($return,true);
            /*
             echo "\n<pre><code>\n";
            echo var_dump($applist);
            echo "\n</code></pre>\n";
            */
            echo '<table border="1">';
            echo '<tr><th>App</th><th>Version</th><th>Actions</th></tr>';
            $appcount=0;
            foreach ($applist as $app ) {
                echo '<tr><td>'.$app['name'].'</td><td>'.$app['version'].'</td>';
                echo '<td >';
                $query = $pdo->prepare("SELECT name,name_long FROM generic_devices ".
                                       "WHERE name='".$app['name']."'");
                $query->execute();
                $row = $query->fetch();
                // is app already installed?
                if ($row) {
                    // yes: add action "remove"
                    echo '<form action="'.$env["myurl"].'" method="get">';
                    echo '<input type="hidden" name="action" value="appstore_appremove">';
                    echo '<input type="hidden" name="name" value="'.$app['name'].'">';
                    echo '<input type="hidden" name="name_long" value="'.$row['name_long'].'">';
                    echo '<input type="submit" value=" Remove... "></form>';
                } else {
                    // no: add action "download&install"
                    echo '<form action="'.$env["myurl"].'" method="get">';
                    echo '<input type="hidden" name="action" value="appstore_appinstall">';
                    echo '<input type="hidden" name="name" value="'.$app['name'].'">';
                    echo '<input type="hidden" name="file" value="'.$app['file'].'">';
                    echo '<input type="submit" value=" Download & Install "></form>';                
                }
                if ($_SESSION['dev-mode']=="on") {
                    echo '<form action="'.$env["myurl"].'" method="get">';
                    echo '<input type="hidden" name="action" value="userapp-publish">';
                    echo '<input type="hidden" name="name" value="'.$app['name'].'">';
                    echo '<input type="hidden" name="file" value="'.$app['file'].'">';
                    echo '<input type="submit" value=" Publish this App "></form>';                
                }
                echo '</td></tr>';
                $appcount++;
            }
            echo '</table>';
            if ($appcount==0 && $_SESSION['dev-mode']=="on")
                echo "<p>No Apps here? You are in Development Mode ".
                     "and look in your local Appstore!</p>";
        }
        break; 

    case "appstore_appinstall":


        echo "<h2>AppStore - Installation</h2>\n";
        
        $name=isset($_GET['name'])?htmlentities($_GET['name'],ENT_QUOTES):'';
        $file=isset($_GET['file'])?htmlentities($_GET['file'],ENT_QUOTES):'';
        if ($name=="" || $file=="") {
            echo "<p>error: name or file app is missing</p>";
        } else {
            // if dev-mode then use local appstore        
            if ($_SESSION['dev-mode']!="on")
                $appstore_url=$env["appstore_url"];
            else
                $appstore_url="http://localhost".$env["baseurl"]."/appstore/appstore.php";

            echo "<p>Downloading App <b>".$file."</b> ... ";
            $filedir=$env["apppath"]."/".$file;
            $file_gz=$file.".gz";
            $fp = fopen($env["apppath"]."/$file_gz", "wb");
            if (!$fp) {
                echo "<p>error: fileopen failed for '".$env["apppath"]."/$file_gz.'</p>";
            } else {
                $appstore_url .= "?action=download&appfile=".$file;
                $ch = curl_init();
                curl_setopt($ch, CURLOPT_URL, $appstore_url);
                curl_setopt($ch, CURLOPT_HEADER, false);
                curl_setopt($ch, CURLOPT_HTTPHEADER, array('Content-Type: application/json'));
                curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_ANY); 
                curl_setopt($ch, CURLOPT_USERPWD, $env["appstore_user"].':'.$env["appstore_pass"]); 
                curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
                $httpBody=curl_exec($ch);
                $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
                curl_close($ch);
                if ($httpCode=="200") {
                    fwrite($fp,$httpBody);
                    fclose($fp);
                    echo "done (".strlen($httpBody)." Bytes)</p>";
                    echo "<p>decompressing... (";
                    $shell_cmd="cd ".$env["apppath"]." && tar zxf ./".$file_gz;
                    shell_exec($shell_cmd);
                    echo $shell_cmd.") ... done<p>";
                    
                    echo "<p>Installing App <b>".$name."</b> locally ($filedir/$file.inc)</p>";
                    require_once("$filedir/$file.inc");
                    // classname ist filename ohne endung!
                    //$classname=substr($file,0,strpos($file,"."));
                    $classname=$file;
                    // jetzt object erstellen...
                    $device = new $classname();
                    // ... und in db als generic_device speichern!
                    $device->setDefaultValues();
                    $device->createNewGenericDeviceInDB();
                    echo "done</p>";
                    unset($device);
                } else {
                    fclose($fp);
                    echo "<p>error: downloading failed</p>";
                }
                
            }
        }

        break; 

    case "appstore_appremove":

        $name=isset($_GET['name'])?htmlentities($_GET['name'],ENT_QUOTES):'';
        $name_long=isset($_GET['name_long'])?htmlentities($_GET['name_long'],ENT_QUOTES):'';

        echo "<h2>GDCBox AppStore - Remove installed App</h2>\n";
        echo "<p>When removing this App, all configured devices are removed, too.</p>";
        echo "<p>Do you really want to remove the App '".$name_long."'?</p>";
        
        echo '<td><form action="'.$env["myurl"].'" method="get">';
        echo '<input type="hidden" name="action" value="appstore_appremove_ok">';
        echo '<input type="hidden" name="name" value="'.$name.'">';
        echo '<input type="hidden" name="name_long" value="'.$name_long.'">';
        echo '<input type="submit" value=" Yep, please! "></form></td></tr>';

        break; 

    case "appstore_appremove_ok":

        $name=isset($_GET['name'])?htmlentities($_GET['name'],ENT_QUOTES):'';
        $name_long=isset($_GET['name_long'])?htmlentities($_GET['name_long'],ENT_QUOTES):'';

        echo "<h2>GDCBox AppStore - Remove installed App</h2>\n";
        echo "<p>Removing App '".$name_long."' from your GDCBox now...</p>";
        $device=new Device();
        $device->removeGenericDeviceFromDB($name);
        echo "<p>...done. App is removed";

        break; 

    case "makenewdevice":

        echo '<h2>Make new Device</h2>';
        echo '<p>Device Apps available on this GDCBox:</p>';

        $query = $pdo->prepare("SELECT * FROM generic_devices ORDER BY name ASC");
        $query->execute();
        $row = $query->fetch();
        if (!$row) {
            echo '<p>No Device Apps available<p>';
        } else {
            echo '<table border="1">';
            echo '<tr><th>Device</th><th>Description</th><th>Version</th><th>Actions</th></tr>';
            do {
                echo '<tr><td>'.$row['name'].'</td>';
                echo '<td>'.$row['name_long'].'<br>'.$row['description'];
                if ($row['url']!='') 
                    echo '<br><a href="'.$row['url'].'">'.$row['url'].'</a>';
                echo '<td>'.$row['version'].'</td>';
                
                // Make new Device
                echo '<td><form action="'.$env["myurl"].'" method="get">';
                echo '<input type="hidden" name="action" value="makenewdevice_ok">';
                echo '<input type="hidden" name="name" value="'.$row['name'].'">';
                echo '<input type="hidden" name="name_long" value="'.$row['name_long'].'">';
                echo '<input type="hidden" name="appfile" value="'.$row['appfile'].'">';
                echo '<input type="submit" value=" Create a new '.$row['name'].' Device "></form></td>';                
                echo '</tr>';
                
            }  while ($row = $query->fetch() );
            echo "</table>\n";
        }
        echo '<p>Device not found? Check the <a href="'.$env["myurl"].'?action=appstore">GDCBox AppStore</a></p>';

        break; 

    case "makenewdevice_ok":

        $name=isset($_GET['name'])?htmlentities($_GET['name'],ENT_QUOTES):'';
        $name_long=isset($_GET['name_long'])?htmlentities($_GET['name_long'],ENT_QUOTES):'';
        $appfile=isset($_GET['appfile'])?htmlentities($_GET['appfile'],ENT_QUOTES):'';

        echo '<h2>Make new Device</h2>';
        echo '<p>Create a new Device of type <b>'.$name_long.'</b> ...</p>';

        // get appfile
        require_once($env["apppath"]."/$appfile/$appfile.inc");
        // classname ist filename ohne endung!
        //$classname=substr($appfile,0,strpos($appfile,"."));
        $classname=$appfile;
        // jetzt object erstellen...
        $device = new $classname();
        $device->setDefaultValues();
        // ... und in db als generic_device speichern!
        $id=$device->createNewDeviceInDB();
        
        echo '<p>...done. Device can be <a href="'.$env["myurl"].
             '?action=configuredevice&device_id='.$id.'">configured</a> now.</p>';
             
        unset($device);


        break; 

    case "configuredevice":

        echo '<h2>Configure Device</h2>';

        $device_id=getValueFromURLSave('device_id');
        
        $device=new Device();
        $device->loadDeviceFromDB($device_id);
        
        echo "<p>Don't need this device anymore? Then ";
        echo '<a href="'.$env["myurl"].'?action=removedevice&device_id=';
        echo $device->device_values['id'].'">remove it</a> (Attention: configuration for ';
        echo 'this device will be lost!).</p>';

        // show device params
        echo 'This device uses the app <b>'.$device->device_values['generic_device_name'].'.</p>';
        echo '<form action="'.$env["myurl"].'" method="get">';
        echo '<input type="hidden" name="action" value="configuredevice_ok">';
        echo '<input type="hidden" name="device_id" value="'.$device_id.'">';
        echo '<table border="0">';
        //echo '<tr><th>Parameter</th><th>Value</th></tr>';
        foreach ($device->device_values as $key => $value) {
            echo "<tr>";
            switch ($key) {
                case 'id': 
                case 'generic_device_name':
                    break;
                default:
                    echo '<td>'.$key.'</td><td><input type="text" size="50" name="'.$key.
                         '" value="'.$value.'"></td>';
            }
            echo "</tr>\n";
        }
        // show device_configs
        foreach ( $device->device_config_values as $key => $valueattribs) {
            $value=$valueattribs['value'];
            // first extract all attribute specifications(separated by "|")
            $attribs=explode('|',$valueattribs['attribs']); // separate attributes
            // second build array $singleattribs with all attribute key/value pairs
            $singleattribs="";
            foreach ($attribs as $singleattrib) {
                if ($singleattrib!="") {
                    $singleattribs=explode(':',$singleattrib);
                }
            }
            // now display form elements considering specified attributes!

            // if not visible, continue!
            if (isset($singleattribs[0])
                && $singleattribs[0]=="visible"
                && $singleattribs[1]=="no")
                continue;
            
            // display editable key/value            
            echo '<tr><td>'.$key.'</td><td><input type="text" size="50" name="'.
                         $key.'" value="'.$value.'"></td></tr>';
            echo "\n";    
        }
        echo "</table>";
        echo '<p><input type="submit" value=" Save Changes "></form></p>';

        if (isset($device->device_config_values['SysInfo'])
             && $device->device_config_values['SysInfo']['value']=='yes') {
            echo '<p>This device helps you configuring by displaying <br>';
            echo 'some Information about the system (opens in new window):<br>';
            echo '<form>';
            echo '<input type="button" value=" System Information ... " onclick="window.open('.
                 '\''.$env["myurl"].'?action=show_system_info&device_id='.$device_id.'\');">';
            echo '</form></p>';
        }


        break; 

    case "configuredevice_ok":

        echo '<h2>Configure Device</h2>';
        $device_id=getValueFromURLSave('device_id');
        $device=new Device();
        $device->loadDeviceFromDB($device_id);
        // save values from html-form to device
        foreach ($device->device_values as $key => $valueattribs) {
            $newvalue=getValueFromURLSave($key);
            if ($key!='id' && $key!='generic_device_name')
                $device->device_values[$key]=html_entity_decode(getValueFromURLSave($key));
        }
                
        foreach ($device->device_config_values as $key => $valueattribs) {
            // save all keys. keys with "visible:no" are not in url!
            $value=$valueattribs['value'];
            $attribs=$valueattribs['attribs'];
            $newvalue=getValueFromURLSave($key);
            if ($newvalue!="")
                $device->device_config_values[$key]['value']=html_entity_decode($newvalue);
        }
        
        $device->saveDeviceToDB();
        echo '<p>Saved.</p>';
        
        // check, if restart is required
        if ( $device->device_values['active']!="no" ) {
            $cronjobs=(int)shell_exec('crontab -l|grep gdcbox |wc -l');
            if ($cronjobs>0) {
                echo "<p>Restarting...</p>";
                cron_stop();
                $processes_started=cron_start();
                echo "<p>Restarting done.</p>";
            }
        }

        break; 

    case "show_system_info":

        echo '<h2>System Information</h2>';
        $device_id=getValueFromURLSave('device_id');
        
        // load device-app
        $query = $pdo->prepare("SELECT gd.appfile,d.active,d.gdc_send ".
                               "FROM devices d, generic_devices gd ".
                               "WHERE d.generic_device_name=gd.name ".
                               "AND d.id=".$device_id);
        $query->execute();
        $row = $query->fetch();
        // load dynamic device app code
        $appfile=$row[0];
        require_once($env["apppath"]."/".$row[0]."/".$row[0]);
 
        // instantiate new device object
        //$classname=substr($appfile,0,strpos($appfile,"."));
        $classname=$appfile;
        $device=new $classname();
        $device->loadDeviceFromDB($device_id);
        if ($device->isLoaded()) {
            echo $device->getSystemInfoInHTML();
        } else {
            echo "<p>No System Information found.</p>";
        }
        echo '<form><input type="button" VALUE=" Close " onClick="top.close();"></form>';


        break; 

    case "removedevice":

        echo '<h2>Remove Device</h2>';

        $device_id=getValueFromURLSave('device_id');
        
        $device=new Device();
        $device->removeDeviceFromDB($device_id);
        echo '<p>Device removed.</p>';

        break; 

    case "gdcbox_start":

        echo '<h2>Starting GDCBox</h2>';
        

        $processes_started=cron_start();

        if ($processes_started)
            echo "<p>GDCBox started ($processes_started Device Processes running)</p>";
        else
            echo '<p>Nothing to be started. Install or activate devices first.<p>';

        break; 

    case "gdcbox_stop":

        echo '<h2>Stopping GDCBox</h2>';
        cron_stop();
        echo "<p>All Device Apps stopped.</p>";
        echo "<p>GDCBox stopped.</p>";
        
        break; 

    case "gdcbox_restart":
        
        echo '<h2>Restarting GDCBox</h2>';
        cron_stop();
        echo "<p>All Device Apps stopped.</p>";
        $processes_started=cron_start();
        if ($processes_started)
            echo "<p>GDCBox restarted ($processes_started Device Processes running)</p>";
        else
            echo '<p>Nothing to be restarted. Install devices first.<p>';

        break; 

    case "gdcbox-info":

        echo '<h3>About GDCBox</h3>';
        echo '<p>The GDCBox ... </p>';
        echo '<p>Questions? Please check the <a href=".$baseurl."/gdcbox/faq.html>GDCBox FAQ</a></p>';
        echo '<p>GDCBox Version: '.$version.'</p>';
        echo '<p>Do you want to develop apps? Read our Howto and continue with Development-Mode. ';
        $dev_mode= $_SESSION['dev-mode']=="on";
        echo 'Development Mode is currently switched '.($dev_mode?'on':'off').'</p>';
        echo '<p><a href="'.$env["myurl"].'?action=devmode-toggle&devmode=';
        echo ((!$dev_mode)?'on':'off').'" data-role="button">';
        echo 'Switch '.((!$dev_mode)?'on':'off').' Development Mode</a></p>';
        echo '<p></p>';

        echo '<p">The GDCBox is a Product of <a href="http://ondics.de">Ondics GmbH</a>.';
        echo '(C) 2012, Ondics GmbH. All rights reserved.</p>';

        break; 

    case "devmode-toggle":

        echo '<h2>Development Mode</h2>';
        $dev_mode=getValueFromURLSave('devmode');
        if ($dev_mode!="on") $dev_mode="off";
        $_SESSION['dev-mode'] = ($dev_mode=="on")?'on':'off';
        echo 'Development Mode is now switched to <b>'.($dev_mode=="on"?'on':'off').'</b></p>';
        break;
    
    case "userapp-upload":

        echo '<h3>Upload your new App to GDCBox</h3>';
        echo '<p>The App must be packed in a zipped tarfile with ending .gz</p>';
        echo '<p>Please get more help <a href="http://pi-io.com" target="_blank">pi-io.com</a>.</p>';
        // display form to upload
        echo '<p><form enctype="multipart/form-data" action="'.$env["myurl"];
        echo '?action=userapp-upload_ok" method="post" data-ajax="false" >';
        echo '<table border="0">';
        echo '<tr><td>App to upload</td>';
        echo '<td><input type="file" name="userappfile"></td></tr>';
        echo "</table>";
        echo '<p><input type="submit" value=" Upload "></form></p>';
        
        // some help for developing apps
        echo '<p></p><p>Some tips for developing your own apps:<ul>';
        echo '<li>start with an app template. You have to in development mode.</li>';
        echo '<li>the Appname must be starting with "userapp_" (e.g. userapp_myfirstapp.inc)</li>';
        echo '<li>put it in a directory named "userapp_myfirstapp/"</li>';
        echo '<li>when finished development, pack it using "tar zcf ./userapp_myfirstapp.gz ./userapp_myfirstapp"</li>';
        echo '<li>upload it to your local Appstore (this form)</li>';
        echo '<li>goto Appstore an install your app.</li>';
        echo '<li>test, debug (logfiles, database) and run!</li>';
        echo '<li>if you want to publish your app, tell us.</li>';
        echo '</ul></p>';
        break; 
    
    case "userapp-upload_ok":

        echo '<h3>Upload your new App to GDCBox</h3>';
        $appfile=$_FILES['userappfile'];
        $error="";
        $uploaddir = $env['basepath'].'/appstore/apps/';
        $uploadfile = basename($appfile['name']);
        $uploadpath = $uploaddir . $uploadfile;

        echo "<p>Uploading file <b>".$uploadfile ."</b> ";
        echo "(".filesize($appfile['tmp_name'])." Bytes).</p>";
        // checking ending
        if (!preg_match("/userapp_[a-zA-Z0-9_]+\.gz/",$uploadfile)) {
            $error="characters in filename allowed: 0-9, a-z, A-Z, underscore, ".
                   "Begin: userapp_, Ending: .gz";
        }
        // move file to appstore-destination
        if ($error=="") {
            if (move_uploaded_file($appfile['tmp_name'], $uploadpath)) {
                echo "<p>App uploaded.</p>";
            } else {
                $error="Upload failed";
            }
        }
        // ok, upload succeeded, now check archive
        if ($error=="") {
            $shell_cmd="cd ".$uploaddir.' && tar ztf ./'.$uploadfile;
            exec($shell_cmd,$cmd_lines);
            foreach($cmd_lines as $cmd_line) {
                echo "<p>path checked=$cmd_line</p>";
                $filenamebegin="./userapp_";
                if (strncmp($filenamebegin,$cmd_line,strlen($filenamebegin))!=0)
                    { $error="paths in archive must start with ./userapp"; break;}
            }
        }
        // ok, upload succeeded, now unpack archive
        if ($error=="") {
            $shell_cmd="cd ".$uploaddir.' && tar zxf ./'.$uploadfile;
            shell_exec($shell_cmd);
        }
        
        if ($error=="")
            echo "<p>Your App is now available from local Appstore!</p>";
        else
            echo "<p>Error: $error</p><p>Try again!</p>";
            
        
        break; 


    case "userapp-publish":

        echo "<h2>Publish your App</h2>\n";
        $name=getValueFromURLSave('name');
        $file=getValueFromURLSave('file');
        if ($name=="" || $file=="") {
            echo "<p>error: name or file is missing</p>";
            break;
        }
        if ($_SESSION['dev-mode']!="on") {
            echo "<p>error: not in dev-mode</p>";
            break;
        }
        echo "<p>You have developed the App <b>$name</b> and want to publish ";
        echo "this App from your local GDCBox to the official GDCBox Appstore.</p>";
        echo '<p>Please fill out the form to start publishing.</p>';
        echo '<p><form action="'.$env["myurl"].'?action=userapp-publish_ok" method="get">';
        echo '<input type="hidden" name="appname" value="'.$name.'">';
        echo '<input type="hidden" name="appfile" value="'.$file.'">';        
        echo '<p><label for="username">Your Name</label>';
        echo '<input name="username" type="text" size="50" maxlength="50"></p>';
        echo '<p><label for="email">Your Email</label>';
        echo '<input name="email" type="text" size="50" maxlength="50"></p>';
        echo '<p><label for="description">Description of your App</label>';
        echo '<textarea name="description" cols="25" rows="5" placeholder="..."></textarea></p>';
        
        echo '<p>By publishing, you agree to put the App in the Official GDC AppStore ';
        echo 'so everybody can download and use it. Your name will be provided with ';
        echo 'the App. Your Email-Adress will just be used by us to contact you and ';
        echo 'will not be published. You will be contacted from us soon after ';
        echo 'having published your app. Thanks a lot for joining the App developer team.</p> ';

        echo '<p><input type="submit" value=" Publish my App now! "></form></p>';
        
        echo '<p>Questions? We like to help you with ';
        echo '<a href="email:support@ondics.de">support@ondics.de</a></p>';        

        break;
    
    case "userapp-publish_ok":

        echo "<h2>Publish your App</h2>\n";
    
        $appname=getValueFromURLSave('appname');
        $appfile=getValueFromURLSave('appfile');
        $username=getValueFromURLSave('username');
        $email=getValueFromURLSave('email');
        $description=getValueFromURLSave('description');
        
        // publish by uploading to appstore
        $publish_url = $env['appstore_url'].'?action=userapp-publish';
        // packed file is available since user has it installed, too
	$filepath = $env['basepath'].'/appstore/apps/'.$appfile.".gz";

	$post_fields = array('appname' => $appname, 'username' => $username,
                      'email' => $email, 'description' => $description,
                      'file_contents'=>'@'.$filepath);
        // now send!        
        $ch = curl_init();
	curl_setopt($ch, CURLOPT_URL,$publish_url);
	curl_setopt($ch, CURLOPT_POST,1);
	curl_setopt($ch, CURLOPT_POSTFIELDS, $post_fields);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
        curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_ANY); 
        curl_setopt($ch, CURLOPT_USERPWD, $env["appstore_user"].':'.$env["appstore_pass"]); 
	$result=curl_exec ($ch);
        $info = curl_getinfo($ch);
        var_dump($info);
	if ($result === false || $info['http_code'] != 200) {
            echo "<p>Error: $result (".curl_error($ch)?curl_error($ch):"unknown".")</p>";
            break;
	}
	curl_close ($ch);            
        echo "<p>Thank you for your efforts and publishing your App!</p> ";
        echo "<p>We will contact you soon.</p>";
        
        break;        

    case "test-dbdump":

        function dbdump($dbtable) {
            global $pdo;
            echo '<p>'.$dbtable.'</p>';
            $query = $pdo->prepare("select * from " . $dbtable);
            $query->execute();
            if ($row=$query->fetch(PDO::FETCH_ASSOC)) {
                echo '<table border="1"><tr>';
                foreach ($row as $field => $value) echo '<th>'.$field.'</th>';
                echo '</tr>';
                do {
                    echo '<tr>';
                    foreach ($row as $field => $value) echo '<td>'.$value.'</td>';
                    echo '</tr>';
                } while ($row=$query->fetch(PDO::FETCH_ASSOC));
                echo '</table>';
            } else
                echo "<p>--- leer ---</p>";
        }

        echo '<h3>DB-Dump</h3>';
        dbdump("generic_devices");
        dbdump("devices");
        dbdump("device_configs");
        dbdump("device_values");
        break; 

    case "test-cronjoblogfile":
        echo '<h3>Cronjob-Logfile</h3>';
        echo "<p>Last 20 Lines of ".$env["cronjob_logfile"]."</p>\n";
        echo '<span style="font-size:smaller"><pre><code>';
        echo htmlentities(shell_exec("tail -n 20 ". $env["cronjob_logfile"]));
        echo "</code></pre></span>";
        break;
    
    default:
    
        echo "<p>ups...</p>";
        
    }
    

    $_SESSION['lastaction']=$action;  // speichern, um bei reloads dopplung zu verhindern
    
/*
    if ( $action != 'main' )
        echo '<p><a href="'.$env["myurl"].'?action=main" data-role="button">Home</a></p>';
*/
    
    // jquery: start content here
    echo '</div><!-- /content -->';
    
    // db-connectivity schlieﬂen
    unset($pdo);
    
    echo '<div data-role="footer"  data-theme="b">';
    echo '<table width="90%" align="center" border="0" style="font-size:small;">';
    echo '<tr><td align="left" width="33%">'.date("H:i").'</td>';
    echo '<td align="center" width="33%">'.($_SESSION['dev-mode']=="on"?'Development':'');
    echo ($testmode?"&nbsp;[Testmode]":"").'</td>';
    echo '<td align="right" width="33%"><a href="'.$env["myurl"].'?action=gdcbox-info" ';
    echo 'style="color:#fff">About GDCBox</a></td></tr>';
    echo (($testmode && $testmsg)?'<tr><td colspan="3" align="left">'.$testmsg.'</td></tr>':'');    
    echo '</table>';
    echo '</div><!-- /footer -->';
    
    echo '</div><!-- /page -->';

?>
</body>
</html>
