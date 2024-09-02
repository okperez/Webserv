# while (1)
# {
#     ;
# }
@split = split('&', %ENV{'QUERY_STRING'});
if (-e "Data_user")
{
    $option = ">>";
}
else
{
    $option = ">";
}
if (open(fh, $option,"Data_user"))
{
    $fname = substr($split[0], index($split[0], '='), length($split[0]) - index($split[0], '='));
    $lname = substr($split[1], index($split[1], '='), length($split[1]) - index($split[1], '='));

    if ($fname eq "=" || $lname eq "=" )
    {
        print "Location: http://localhost:8080/sign_up.html\n";
    }
    else 
    {
        # while (my $String = <FH>)
        # {
        #     if($String =~ /${fname}/ || $String =~ /${lname}/) 
        #     { 
        #         exit (0);
        #     } 
        # }
        print fh "----------------------------------------- USER -----------------------------------------\n\n";
		print fh "fname${fname}\n";
		print fh "lname${lname}\n\n";
		print "Location: http://localhost:8080/sign_in.html\n";
	}
    @split = ();
    $fname = undef;
    $lname = undef;
	close(fh);
}
else
{
    print "1";
}