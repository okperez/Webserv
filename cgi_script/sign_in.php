<?php

// while (1)
// {
//     ;
// }

$file = file_get_contents('Data_user');
if (getenv("REQUEST_METHOD") != "POST")
{
    echo "1";
    exit (0);
}
if ($file != false && strlen($file) > 0)
{
    $fname = substr(getenv("QUERY_STRING"), strpos(getenv("QUERY_STRING"), "fname"), strpos(getenv("QUERY_STRING"), '&') - strpos(getenv("QUERY_STRING"), "fname"));
    $firstname = substr(getenv("QUERY_STRING"), strpos(getenv("QUERY_STRING"), "fname") + 6, strpos(getenv("QUERY_STRING"), '&') - strpos(getenv("QUERY_STRING"), "fname") - 6);
    $lname = substr(getenv("QUERY_STRING"), strpos(getenv("QUERY_STRING"), "lname"), strrpos(getenv("QUERY_STRING"), '&') - strpos(getenv("QUERY_STRING"), "lname"));
    if (strpos($file, $fname) > -1 && strpos($file, $lname) > -1)
    {
        if ((str_contains (getenv("QUERY_STRING"), "rememberMe=on")))
        {
            echo "Set-Cookie: $fname; path=/sign_in.html;\n";
            echo "Set-Cookie: $lname; path=/sign_in.html;\n";
        }
        echo("<!DOCTYPE html>");
        echo("<html>");
        echo("<head>");
        echo("<title>Welcome</title>");
        echo ("<link rel=\"stylesheet\" type=\"text/css\" href=\"/session.css\">");
        echo("</head>");
        echo("<body>");
        echo("<div id=\"center\">");
        echo("<h1>Welcome ");
        echo($firstname);
        echo("</div>");
        echo("</h1>");
        echo("</body>");
        echo("</html>");
    }
    else
    {
        echo "Location: http://localhost:8080/session.html\n";
    }
}
else
    echo "1"
?>