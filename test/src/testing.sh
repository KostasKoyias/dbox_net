#!/bin/bash
##############################################################################################################################################
# this script uses ssh to connect to linux01.di.uoa.gr , locates the path to a certain application, let us say "my_app", that must contain a #
# makefile which generates binary programs named "dbserver" and "dbclient". It then initializes a trial folder under my_app/test/rsrc/trial  #
# containg folders client01, client02, ..., client0n with random content, runs the dbserver. Right after that, it creates one "dbclient"     #
# process for each machine: linux01.di.uoa.gr-linux0${3}.di.uoa.gr. So, the first argumet is the sdi, 2nd is the application name and 3rd is #
# the # of clients(max is 8), default is 5. After $1*10 seconds, it sends a SIGINT to each of those clients and after 10 more to the server  #
##############################################################################################################################################

max_users=8        # change to your preference
id=${1-1500071}
app_name=${2-dbox_net}
users=${3-5}
### make sure the right number of arguments is passed
if [ $# -ne 2 ] && [ $# -ne 3 ] 
then
    echo "Usage: $0 <sdi> <app_name> (<number_of_users>)"
    exit 1
fi

### support at most $max_users users
if [ $users -gt $max_users ]
then 
    echo -e "testing.sh: \e[1;31mError\e[0m, too many users, maximum is $max_users"
    exit 2
fi

### connect to linux01.di.uoa.gr to compile source code, prepare input files and run the server
mkdir out >& /dev/null
ssh sdi$id@linux01.di.uoa.gr 'bash -s' < initialize.sh $users $app_name >& out/server.txt &
sleep 30

### run a client process for each user in a different machine at linux workstations of di.uoa.gr
secs_of_operation=$(($users*15))
echo "testing.sh: running $users clients... for $secs_of_operation seconds each"
declare -a pids
for ((i = 1;i <= $users; i++))
do
    ssh sdi$id@linux0$i.di.uoa.gr 'bash -s' < client.sh $secs_of_operation $i >& out/client0$i.txt &
    pids+=($!)
done

### wait for all clients to terminate
for i in ${pids[@]}
do
    kill -INT $i
    wait $i
done
echo -e "testing.sh: all processes terminated\ntesting.sh: exiting now..."