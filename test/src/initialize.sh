#!/bin/bash
# this script: compiles source code, prepares input files and runs the server
max_users=8
users=${1-5}
if [ $users -gt $max_users ]
then 
    exit 1
fi

### get full path to dbox_net folder
dbox_net=$(pwd)$(find . -type d -name "dbox_net" | head -1 | cut -c 2- )
if [ $? -ne 0 ]
then
    exit 2
fi
echo dbox_net = $dbox_net

### create an "app" folder where all clients reside
rsrc=$(find $dbox_net -type d -name "test" | head -1)/rsrc
mkdir $rsrc/app >& /dev/null
if [ $? -ne 0 ]
then
    rm -rf $rsrc/app/*
fi
echo app=$rsrc/app

### initialize input folder for each client, containing 5 files in 3 folders, 2 levels deep
fileGenerator=$(find $dbox_net -type f -name "create_infiles.sh" | head -1)
echo $fileGenerator
for ((i = 1;i <= $users; i++))
do 
    $fileGenerator $rsrc/app/client0$i   5 3 2
done
exit 0

### run dbserver binary, if it does not exist, use makefile to compile 
exe=$(find $dbox_net -name "dbserver" | head -1)
if [ $? -ne 0 ]
then
    cd $dbox_net/main;make
    exe=$(find $dbox_net -name "dbserver" | head -1)
fi
echo $dbox_net/main/dbserver