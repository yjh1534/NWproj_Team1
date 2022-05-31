#!/bin/usr/env bash

# log file path
foo=$1 #"log/wifi_ad" 

# data of chat log
awk '$1=="1:1msg" {for (i=1; i<NF; i++) printf $i " "; print $NF}' $foo.dat > "$foo"_chat.dat
awk '$1=="room0" {for (i=1; i<NF; i++) printf $i " "; print $NF}' $foo.dat > "$foo"_room0.dat
awk '$1=="room1" {for (i=1; i<NF; i++) printf $i " "; print $NF}' $foo.dat > "$foo"_room1.dat
awk '$1=="room2" {for (i=1; i<NF; i++) printf $i " "; print $NF}' $foo.dat > "$foo"_room2.dat
awk '$1=="room3" {for (i=1; i<NF; i++) printf $i " "; print $NF}' $foo.dat > "$foo"_room3.dat
awk '$1=="room4" {for (i=1; i<NF; i++) printf $i " "; print $NF}' $foo.dat > "$foo"_room4.dat
# for t in room{0..4}; do declare -p t; awk '$1=={$t} {for (i=1; i<NF; i++) printf $i " "; print $NF}' bus.dat > bus_$t.dat; echo $t; done

# data of troughput
awk '$1=="tp" {print $2 "\t" $3}' $foo.dat > "$foo"_tp.dat