#!/bin/bash
####################################################################################################
# this script: compiles source code, prepares input files and runs the server on linux01.di.uoa.gr #
# 1st argument = #_of_users, 2nd = application_name under user directory(the 1st match is used)    #
# make sure to store create_infiles.sh and names.sh somewhere under your application directory     #
####################################################################################################

max_users=8
users=${1-5}
port=8080
if [ $users -gt $max_users ]
then 
    exit 1
fi

### get full path to application folder
application=$(pwd)$(find . -type d -name $2 | head -1 | cut -c 2- )
if [ "$application" == "" ]
then
    echo -e "initialize.sh: \e[1;31mError\e[0m, unable to locate application folder $1"
    exit 2
fi
echo initialize.sh: Application directory found under $application

### create an "app" folder where all clients reside
trial=$application/test/rsrc/trial
if [ -e $trial ]
then
    echo "initialize.sh: Purging $trial's content to start over a new trial"
    rm -rf $trial/*
fi
mkdir -p $trial
echo initialize.sh: Trial directory created under $trial

# locate "create_infiles.sh"
fileGenerator=$(find $application -type f -name "create_infiles.sh" | head -1)
if [ "$fileGenerator" == "" ]
then
    echo -e "initialize.sh: \e[1;31mError, unable to locate 'create_infiles.sh'"
    exit 3
fi
echo initialize.sh: create_infiles.sh located under $fileGenerator

### initialize input folder for each client, containing 5 files in 3 folders, 2 levels deep
for ((i = 1;i <= $users; i++))
do 
    $fileGenerator $trial/client0$i/input  5 3 2 >& /dev/null
done
echo "initialize.sh: Created $users client directories under $trial, each of them containing 5 files in 3 folders, 2 levels deep"

### run dbserver binary on port 8080, in backgournd-mode, if it does not exist, use makefile to compile first
exe=$(find $application -name "dbserver" | head -1)
if [ "$exe" == "" ]
then
    make_path=$(find $application -iname "makefile" | head -1)
    if [ ! -e $make_path ]
    then
        echo -e "initialize.sh: \e[1;31mError, unable to locate neither 'dbserver' binary nor a makefile to produce it"
        exit 4
    fi
    cd $(dirname $make_path);make >& /dev/null
    exe=$(find $application -name "dbserver" | head -1)
    if [ "$exe" == "" ]
    then
        echo -e "initialize.sh: \e[1;31mError, unable to locate 'dbserver' even after compilation"
        exit 5
    fi
    echo "initialize.sh: Located 'dbserver' after compilation under $exe"
fi
$exe -p $port &
echo dbserver running at linux01.di.uoa.gr on port $port with pid: $! # display pid of the server

# let server operate for 5 minutes
sleep 300
kill -INT $!
wait $!
exit 0