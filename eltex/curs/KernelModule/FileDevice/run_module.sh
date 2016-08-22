#!/bin/bash

module_name='HW_module'
file_name='inou'
module_dir='.'

echo "Start $module_name..."
insmod $module_name.ko &&
echo "Get Major and Minor from log..." &&
cat /var/log/kern.log | grep $module_name | cut -f 5 -d : | tail -n 1 | cut -f 3,5 -d ' ' > log.tmp &&
read major minor < log.tmp &&
rm log.tmp &&
echo "Create $module_dir/$module_name/$file_name ($major - $minor)..." &&
mkdir $module_dir/$module_name &&
mknod $module_dir/$module_name/$file_name c $major $minor &&
echo "Success"
