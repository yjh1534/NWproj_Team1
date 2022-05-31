# Run Example to Get Logfiles

* Change 'wifi_ad' to something you want to test.
* Can get Log of chatting in './log/*room.dat' files.
* Can get Log of throughput in './log/*_pt.dat' file. The format of data is 'time "\t" throughput'. 

```console
foo@bar:~$ export 'NS_LOG=wifi_ad=level_all'
foo@bar:~$ ./waf --run 'wifi_ad' > ./log/wifi_ad.dat 2>&1
foo@bar:~$ bash get_log.sh log/wifi_ad
```