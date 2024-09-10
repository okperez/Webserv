# while (1)
# {
#     ;
# }


if (%ENV{'REQUEST_METHOD'} eq "POST")
{
    @split = split('&', %ENV{'QUERY_STRING'});
    $fname = substr($split[0], index($split[0], '='), length($split[0]) - index($split[0], '='));
    $lname = substr($split[1], index($split[1], '='), length($split[1]) - index($split[1], '='));
    if (-e "Data_user")
    {
        $option = ">>";
    }
    else
    {
        $option = ">";
    }
    if (open($read_fh, '<', "Data_user"))
    {
        while (my $String = <$read_fh>)
        {
            if($String =~ /$fname/ || $String =~ /$lname/) 
            {
                print "Location: http://localhost:8080/sign_in.html\n";
                $fname = undef;
                $lname = undef;
                close(read_fh);
                @split = ();
                exit (0);
            } 
        }
        close($read_fh);
    }
    if (open(fh, $option,"Data_user"))
    {
        if ($fname eq "=" || $lname eq "=" )
        {
            print "Location: http://localhost:8080/sign_up.html\n";
        }
        else 
        {
            print fh "----------------------------------------- USER -----------------------------------------\n\n";
            print fh "fname${fname}\n";
            print fh "lname${lname}\n\n";
            print "Location: http://localhost:8080/sign_in.html\n";
        }
        close(fh);
    }
    else
    {
        print "1";
    }
    $fname = undef;
    $lname = undef;
    @split = ();
}
else
{
    print "1";
}