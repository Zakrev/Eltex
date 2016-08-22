#!/bin/bash

module_name='HW_module'
file_name='inou'
module_dir='.'

rmmod $module_name
rm -r $module_dir/$module_name
