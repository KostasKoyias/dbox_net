#!/bin/bash
### make sure the right number of arhuments is passed
if [ $# -ne 0 ] && [ $# -ne 1 ]
then
    echo "Usage: $0 (number_of_users)"
    exit 1
fi

### support at most 10 users
if [ $# -ne 0 ] && [ $1 -gt 10 ]
then 
    echo "Error: too many users, maximum is 10"
    exit 2
fi
### find dbox_net folder on linux.di servers of uoa
linux_server="ssh sdi1500071@linux01.di.uoa.gr"
dbox_net=$($linux_server "find . -type d -name "dbox_net" | head")
echo path_to dbox_net: $dbox_net


client=$($linux_server "cd $dbox_net/main;make" )
echo just compiled
exit 0

### get paths of: i)the input file generator script and ii)the user's executable 
create_input=./create_infiles.sh
exe=$(find .. -name "dbclient")
if [ $? -ne 0 ]
then 
    echo "Error: dbclient executable could not be found, enter \"main\" folder and type 'make'"
    exit 3
fi 

### for each user:i)generate a unique user ID, ii)create an input folder and iii)run a mirror executable iv)store their pids in a list
users=${1-5}
declare -a pids
echo "runnin $users clients..."
for ((i = 0;i < $users; i++))
do
    let id=$i*11+1
    $create_input $tst/inputs/input_$id 5 3 2 > /dev/null
    $mirror -n $id -c $tst/common -i $tst/inputs/input_$id -m $tst/mirrors/mirror_dir_$id -b 32 -l $tst/logs/log_$id &> $tst/trash/trash_$id &
    pids+=($!)
done
echo "done"

### let all users sync with each other
let time=users*10
echo "sleeping for $time seconds, letting them sync with each other..."
sleep $time

### terminate all mirror processes generated, wait until each of them exits safely
echo "woke up, sending an interrupt signal to all mirror processes in the system"
for i in ${pids[@]}
do 
    kill -INT $i 
    wait $i
done
echo "done"

### get statistics of the operation
echo "getting stats from log files, putting them in $tst/stats..."
cat $tst/logs/* | $(find .. -name "get_stats.sh") > $tst/stats
echo -e "done\nexiting now..."
