
# @split = split('&', %ENV{'QUERY_STRING'});
# if (open(fh, '>', "Data_user"))
# {
    # $fname = substr($split[0], index($split[0], '='), length($split[0]) - index($split[0], '='));
    # $lname = substr($split[1], index($split[1], '='), length($split[1]) - index($split[1], '='));
# 
    # if ($fname == "" || $lname == "")
    # {
        # print "Location: http://localhost:8080/sign_up.html\n";
        # return ;
    # }
    # print fh "fname${fname}\n";
    # print fh "lname${lname}";
    # close(fh);
    # print "Location: http://localhost:8080/sign_in.html\n";
# 
# }
# else
# {
    # print "1";
# }