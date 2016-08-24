#!/bin/bash

#Скрипт подключает модуль
#читает из лога maj и min номера файла (то, что считалось не проверяется, могут быть ошибки)
#с помощью mknod создает файл file_name в папке module_name, которую создает в дирректории module_dir

module_name='GRoot'
file_name='root'
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
