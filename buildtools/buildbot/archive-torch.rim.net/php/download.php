<?php
//These are passed to this scipt via web iface
$type = $_GET['type'];
$dir = $_GET['dir'];
$name = $_GET['name'];
$what = $_GET['what'];

$allowedExtensions = array ('.sfi', '.dmp');
$home = "/home/buildbot/public_html/binaries";
$orderOfInstallation = array ('rim.*\.sfi', 'FSLinked\.dmp');
$pid = getmypid();


//     +++++ BATCH FILES +++++
//Generate the batch file and include it in the archive.
$fileContent = file_get_contents ("install_device.bat.template");

//
$fileContent = str_replace ("<PID>", $pid, $fileContent);

//Now add each file in the [$home/$dir] path to the batch file.
//The order of installation is important so that will be implemented here
$allFiles = null;
$handle = opendir ("$home/$dir");
while ($file = readdir ($handle)){

    if (is_dir ("$home/$dir/$file")) {
        continue;
    }

    if (is_link ("$home/$dir/$file") && ($what != "all")) {
        continue;
    }
    foreach ($allowedExtensions as $extension) {
        if (preg_match("/$extension$/", $file)){
            $allFiles[] = $file;
        }
    }
}
closedir ($handle);

//Install files in the proper order
foreach ($orderOfInstallation as $order) {
    foreach ($allFiles as $file){
        if (preg_match("/$order/", $file)) {
            $fileContent .= "call :Run_File $file\n";
            $filesAdded[] = $file;
        }
    }
}
//Now add the rest...
foreach ($allFiles as $file) {
    if (! in_array ("$file", $filesAdded)) {
        $fileContent .= "call :Run_File $file\n";
    }
}

$fileContent .= "GOTO End";

//Write temp files.
$installDeviceFile = "/tmp/install_device.$pid.bat";
$handle = fopen ($installDeviceFile, "w");
fwrite ($handle, $fileContent);
fclose ($handle);
$javaLoaderFile = "/tmp/JavaLoader.$pid.exe";
copy("JavaLoader.exe", $javaLoaderFile);

//     +++++ ARCHHIVES +++++
//Currently only supporting zip archives
//but may support batch file which will download a dynamically generated batch
//file that will use wget to download and install the software onto your device
switch ($type){
case "zip":
    $zipCommand = "/usr/bin/zip -jr -@";
    $findCommand = "/usr/bin/find $home/$dir $installDeviceFile";
    if ($what != "all") {
        $findCommand .= " -type f";
    }
    $zipFile="/tmp/$name.$pid.zip";
    shell_exec ("$findCommand | $zipCommand $zipFile $javaLoaderFile");
    header ('Content-Description: File Transfer' );
    header ('Content-Type: application/zip');
    header ('Content-Length: ' . filesize($zipFile));
    header ('Content-Disposition: attachment; filename=' . basename($zipFile));
    readfile ($zipFile);
    unlink ($zipFile);
    break;
default:
    echo "<h2>Error: Unknow type received. [$type]";
}
unlink ($installDeviceFile);
unlink ($javaLoaderFile);
?>
