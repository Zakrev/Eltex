#!/bin/bash

module_name='HW_module'

#Все относящееся к программе
cat /var/log/kern.log | grep $module_name
#Последняя запись
#cat /var/log/kern.log | grep $module_name | cut -f 5 -d : | tail -n 1
