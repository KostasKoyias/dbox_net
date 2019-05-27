#!/bin/bash
####################################################################################################
# this script locates dbclient binary, creates a client and send an interrupt signal to that client#
# after $1 seconds. The input directory must be under client0$2                                    #
####################################################################################################

secs_of_operation=$1
sp_port=8080
cl_port=8081
id=$2
server_machine=linux01.di.uoa.gr


# ensure usage is correct
if [ $# -ne 2 ]
then
    echo -e "client.sh: Usage is $0 <seconds_of_operation> <client_id>"
    exit 1
fi

# find dbclient executable 
exe=$(find . -type f -name dbclient | head -1)  
if [ "$exe" == "" ]
then
    echo -e "client.sh: \e[1;31mError\e[0m, unable to locate dbclient executable"
    exit 2
fi

# run client for a while, then send an interrupt signal to the client to terminate
input=$(find . -type d -name client0$2 | head -1)
if [ "$input" == "" ]
then
    echo -e "client.sh: \e[1;31mError\e[0m, unable to locate input directory for client $id"
    exit 3
fi
input=$input/input
$exe -w 5 -b 40 -sip $server_machine -sp $sp_port -p $cl_port -d $input &
echo "client.sh: started client0$id, process $! at linux0$id.di.uoa.gr on port $cl_port"
sleep $secs_of_operation
kill -INT $!
echo "client.sh: sent SIGINT to client0$id, process $! on linux0$id.di.uoa.gr"
wait $!
exit 0



