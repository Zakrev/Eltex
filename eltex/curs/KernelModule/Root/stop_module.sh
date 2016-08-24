#!/bin/bash

#Скрипт отключает модуль, удаляет папку module_name и файл file_name

module_name='GRoot'
file_name='root'
module_dir='.'

rmmod $module_name
rm -r $module_dir/$module_name
