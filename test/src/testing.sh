#!/bin/bash
max_users=8
### make sure the right number of arguments is passed
if [ $# -ne 0 ] && [ $# -ne 1 ]
then
    echo "Usage: $0 (number_of_users)"
    exit 1
fi

### support at most $max_users users
if [ $# -ne 0 ] && [ $1 -gt $max_users ]
then 
    echo "Error: too many users, maximum is 10"
    exit 2
fi

### connect to linux01.di.uoa.gr to compile source code, prepare input files and run the server
ssh sdi1500071@linux01.di.uoa.gr 'bash -s' < initialize.sh $1
exit 0

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
echo -e "done\nexiting now..."