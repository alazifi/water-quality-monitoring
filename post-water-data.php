<?php

// server
$servername = "localhost";
// db_name
$dbname = "db_name";
// username
$username = "username";
// password
$password = "password";

$api_key_value = "ju&o9*bg";

$api_key= $Suhu = $pH = $TDS = $EC = $Kekeruhan = $DO = $Resistivity = $Salt ="";

if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $api_key = test_input($_POST["api_key"]);
    if($api_key == $api_key_value) {
        $Suhu = test_input($_POST["Suhu"]);
        $pH = test_input($_POST["pH"]);
        $TDS = test_input($_POST["TDS"]);
        $EC = test_input($_POST["EC"]);
        $Kekeruhan = test_input($_POST["Kekeruhan"]);
        $DO = test_input($_POST["DO"]);
        $Resistivity = test_input($_POST["Resistivity"]);
        $Salt = test_input($_POST["Salt"]);

        $conn = new mysqli($servername, $username, $password, $dbname);
        
        if ($conn->connect_error) {
            die("Connection failed: " . $conn->connect_error);
        } 
        
        $sql = "INSERT INTO TableName (Suhu, pH, TDS, EC, Kekeruhan, DO, Resistivitas, Salinitas)
        VALUES ('" . $Suhu . "', '" . $pH . "', '" . $TDS . "', '" . $EC . "', '" . $Kekeruhan . "', '" . $DO . "', '" . $Resistivity . "','" . $Salt . "')";
        
        if ($conn->query($sql) === TRUE) {
            echo "New record created successfully";
        } 
        else {
            echo "Error: " . $sql . "<br>" . $conn->error;
        }
    
        $conn->close();
    }
    else {
        echo "Wrong API code";
    }

}
else {
    echo "No data";
}

function test_input($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}