#!/bin/bash
####################################################################################################
# this script: compares the input folder of each client with the corresponding clone of the others #
####################################################################################################

max_users=8
if [ $# -ne 1 ] || [ $1 -gt $max_users ]
then 
    exit 1
fi
users=$1

### get full path to application folder
trial=$(pwd)$(find . -type d -name trial | head -1 | cut -c 2- )

# for each client c0
for ((i = 1;i <= $users; i++))
do

    # for each other client c1
    declare -a fail_list
    fail_list=()
    for ((j = 1;j <= $users; j++))
    do

        # omit that client himself
        if [ $i -eq $j ]
        then
            continue
        fi 

        # if c0/mirror/c1 != c1/input then c0 failed to get all files of c1 
        diff $trial/client0$i/mirror/linux0$j.di.uoa.gr_* $trial/client0$j/input >& /dev/null
        if [ $? -ne 0 ]
        then
            fail_list+=(client0$j)
        fi
    done
    
    # display results
    if [ ${#fail_list[@]} -ne 0 ]
    then
        echo -e "\e[1;4mclient0$i\e[0m: \e[1;31mFailed\e[0m to receive files of ${fail_list[@]}"
    else
        echo -e "\e[1;4mclient0$i\e[0m: \e[1;32mSuccessfully\e[0m received all files of others"
    fi
    echo  -en "\e[1;34m"
    ls $trial/client0$i/mirror
    echo -e "\e[0m"
done