<?php
//====================================
//Torch Archive Server PHP script.
//====================================

//By default, only 10 of the nightly and incremental builds are shown.
$show_num_incremental = ( $_GET['show'] == "All" )? $_GET['show']: 10;
$show_num_nightly = ( $_GET['show'] == "All" )? $_GET['show']: 1;

//These are the respective directories on the archive server:
$latest_dir = "LATEST";
$nightly_dir = "nightly";
$incremental_dir = "incremental";

function show_more_or_less($in) {
    $show_string = ( $in== "All" ) ? "Less" : "All";
    echo "<a href=?show=$show_string><small>Show $show_string...</small></a>";
}


//PHP only lets you return one thing from a function so...
$file_names = array();
$file_sizes = array();
$link_sizes = array();

//Gather file information from the given directory and store
//it in the above global arrays.
function parse_dir ( $dir ) {

    if( ! is_dir( $dir ) ){
        return 1;
    }

    global $file_names, $file_sizes, $link_sizes;
    $file_names = null;
    $file_sizes = null;
    $link_sizes = null;

    $this_handle = opendir ( "$dir" );
    while ( $file = readdir ( $this_handle ) ){

        $file_path = "$dir/$file";

        if( is_dir( $file_path ) ){
            continue;
        }
        $file_names[] = $file;
        $link_sizes[] = 0;
        $file_sizes[] = 0;

        if ( is_link( "$file_path" ) ) {
            $link_sizes[sizeof($link_sizes)-1] = filesize ( $file_path );
        } elseif ( is_file( "$file_path" ) ) {
            $file_sizes[sizeof($file_sizes)-1] = filesize ( $file_path );
        } else {
            echo "WARNING: Unknown file type [$file_path]\n";
        }
    }
    closedir( $this_handle );
}
?>


<html>
<head>
<link rel="stylesheet" type="text/css" href="php/css/archive.css" />
</head>
<title>
Torch Archive Server
</title>
<body>
<small>Welcome to the Torch Archive Server.</small>
<hr>

<h3><a name=NIGHTLY>NIGHTLY BUILDs</a></h3>
<table cellspacing=10>
<?php
//=============================
//START NIGHTLY
//=============================
$dirs_gathered = array();
$handle = opendir ( $nightly_dir );
while ( $dir = readdir ( $handle ) ){
    if ( preg_match ( "/^\.+\$/",$dir ) ){
        continue;
    }
    if ( ! is_dir ( "$nightly_dir/$dir" ) ) {
        continue;
    }
    $dirs_gathered[] = "$dir";

    $file_info_hash[$dir]  = "";

    //Get contents of this directory and save info
    parse_dir ( "$nightly_dir/$dir" );
    $file_info_hash[$dir]  = "";
    $total_size = 0;

    for($cnt=0; $cnt < sizeof ( $file_names ); $cnt++ ) {
        $this_file_size = (int) ( ( $file_sizes[$cnt] + $link_sizes[$cnt] ) / 1000 );
        $total_size += $this_file_size;
        $file_info_hash[$dir]  .= "$file_names[$cnt]: $this_file_size KB<br>";
    }
    $total_sizes[$dir] =  (int) ( $total_size / 1000 );

}
closedir ( $handle );

//Sort dirs alphabetically
rsort ( $dirs_gathered );

//Print a link to each directory to have an archive generated.
for ( $dir_cnt=0; $dir_cnt < sizeof($dirs_gathered); $dir_cnt++ ){

    $dir = $dirs_gathered[$dir_cnt];

    //Only show some archives unless show all
    if ( $show_num_nightly != "All"  && $dir_cnt >= $show_num_nightly ) {
        continue;
    }

    $info = "<span class=\"link\"><a href=\"javascript: void(0)\"><font face=verdana,arial,helvetica size=2>($total_sizes[$dir] MB)</font><span>";
    //Find out information about the contents of the directory...
    if ( "$file_info_hash[$dir]" != "" ) {
        $info .= "$file_info_hash[$dir]";
    }
    $info .= "</span></a></span>";



    $file_date = date( "F d Y H:i:s", filemtime( "$nightly_dir/$dir" ) );

    $drt_url = "http://md-athens-01.swlab.rim.net:81/buildbot/DRT_RESULTS/nightly/$dir";

    echo "<td width=222><li><b><font face=Courier>$dir</font></b></li></td>";
    echo "<td width=222>[<a href=php/download.php?dir=$nightly_dir/$dir&type=zip&name=$dir&what=all>download</a>] $info</td><tr>";
}
echo "</table>";
show_more_or_less ( $show_num_incremental );
//=============================
//END NIGHTLY
//=============================
?>
<br><br>




<hr><h3><a name=INCREMENTAL>INCREMENTAL BUILDs</a></h3>
<table cellspacing=10>
<?php
//=============================
//START INCREMENTAL
//=============================
$dirs_gathered = null;
$handle = opendir ( $incremental_dir );
while (  $file = readdir ( $handle ) ) {
    if ( preg_match ( "/^\.+\$/",$file ) ){
    continue;
    }
    if ( ! is_dir ( "$incremental_dir/$file" ) ) {
    continue;
    }
    $dirs_gathered[] = "$incremental_dir/$file";
}
closedir ( $handle );

//Sort files by date
array_multisort ( array_map ( 'filemtime', $dirs_gathered ), SORT_DESC, $dirs_gathered );

for ( $dir_cnt=0; $dir_cnt < sizeof($dirs_gathered); $dir_cnt++ ){

    $revision = $dirs_gathered[$dir_cnt];

    //Only show some archives unless show all
    if ( $show_num_incremental != "All"  && $dir_cnt >= $show_num_incremental ) {
    continue;
    }

    //Get the sizes of all files in this directory
    $rev_size_full = 0;
    $rev_size_part = 0;
    $file_info_full = "";
    $file_info_part = "";
    $total_file_size_full = 0;
    $total_file_size_part = 0;

    //Get the sizes of all files in this directory
    parse_dir ( "$revision" );
    for( $cnt=0; $cnt < sizeof ( $file_names ); $cnt++ ) {
        $file_size_full = (int) ( ( $file_sizes[$cnt] + $link_sizes[$cnt] ) / 1000 );
    $file_size_part = (int) ( ( $file_sizes[$cnt] ) / 1000 );

        $total_file_size_full += $file_size_full;
        $total_file_size_part += $file_size_part;

        $file_info_full .= "$file_names[$cnt] : $file_size_full KB<br>";
        if ( ! is_link ( "$revision/$file_names[$cnt]" ) ) {
            $file_info_part .= "$file_names[$cnt] : $file_size_part KB<br>";
        }
    }

    //Show sizes in MB
    $total_file_size_full = (int)( $total_file_size_full / 1000 );
    $total_file_size_part = (int)( $total_file_size_part / 1000 );

    //Prepare the info onOver
    $info_full = "<span class=\"link\"><a href=\"javascript: void(0)\"><font face=verdana,arial,helvetica size=2>($total_file_size_full MB)</font><span> $file_info_full </span></a></span>";
    $info_part = "<span class=\"link\"><a href=\"javascript: void(0)\"><font face=verdana,arial,helvetica size=2>($total_file_size_part MB)</font><span> $file_info_part </span></a></span>";


    //Prepare the output
    $rev_date = date ( "F d Y H:i:s.", filemtime( "$revision" ) );
    list ( $rev_name, $rev_revision ) = split ( '\.',$revision );
    $rev_revision = substr ( $rev_revision, 0, 8 );

    echo "<td width=222><li><b><font face=Courier>$rev_revision</font></b></li></td>";
    echo "<td width=222>[<a href=php/download.php?dir=$revision&type=zip&name=$rev_revision&what=all>complete archive</a>] <small>$info_full</small></td>";
    echo "<td width=222>[<a href=php/download.php?dir=$revision&type=zip&name=$rev_revision&what=part>Only built items</a>] <small>$info_part</small></td>";
    echo "<td width=222><small>$rev_date</small></td><tr>";
}
echo "</table>";

//Show more/less
show_more_or_less ( $show_num_incremental );

//=============================
//END INCREMENTAL
//=============================
?>

<br><br><hr>
<small>
Please see the <a href="README">README</a> file for an explanation of the directory structure in the archive server.
</small>
</body>
</html>
