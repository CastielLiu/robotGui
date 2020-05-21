#!/bin/sh

roslaunch robot_nav nav_wan.launch 

gnome-terminal -x bash -c 'source ~/.bashrc;roslaunch robot_nav nav_wan.launch'

