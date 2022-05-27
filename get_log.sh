#!/bin/usr/env bash
foo="wifi_ad"
awk '$1=="1:1msg" {for (i=1; i<NF; i++) printf $i " "; print $NF}' $foo.dat > "log/$foo"_chat.dat
awk '$1=="room0" {for (i=1; i<NF; i++) printf $i " "; print $NF}' $foo.dat > "log/$foo"_room0.dat
awk '$1=="room1" {for (i=1; i<NF; i++) printf $i " "; print $NF}' $foo.dat > "log/$foo"_room1.dat
awk '$1=="room2" {for (i=1; i<NF; i++) printf $i " "; print $NF}' $foo.dat > "log/$foo"_room2.dat
awk '$1=="room3" {for (i=1; i<NF; i++) printf $i " "; print $NF}' $foo.dat > "log/$foo"_room3.dat
awk '$1=="room4" {for (i=1; i<NF; i++) printf $i " "; print $NF}' $foo.dat > "log/$foo"_room4.dat
# for t in room{0..4}; do declare -p t; awk '$1=={$t} {for (i=1; i<NF; i++) printf $i " "; print $NF}' bus.dat > bus_$t.dat; echo $t; done