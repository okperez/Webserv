<?php

$file = file_get_contents('Data_user');
if ($file != false && strlen($file) > 0)
{
    $fname = substr(getenv("QUERY_STRING"), strpos(getenv("QUERY_STRING"), "fname"), strpos(getenv("QUERY_STRING"), '&') - strpos(getenv("QUERY_STRING"), "fname"));
    $lname = substr(getenv("QUERY_STRING"), strpos(getenv("QUERY_STRING"), "lname"), strrpos(getenv("QUERY_STRING"), '&') - strpos(getenv("QUERY_STRING"), "lname"));
    if (strpos($file, $fname) > -1 && strpos($file, $lname) > -1)
    {
        if ((str_contains (getenv("QUERY_STRING"), "rememberMe=on")) && getenv("REQUEST_METHOD") == "POST")
        {
            echo "Set-Cookie: $fname; path=/sign_in.html;\n";
            echo "Set-Cookie: $lname; path=/sign_in.html;\n";
        }
        echo "Location: http://localhost:8080/welcome.html\n";
    }
    else
    {
        echo "Location: http://localhost:8080/session.html\n";
    }
}
else
    echo "1"
?>