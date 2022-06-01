# Run Example to Get Logfiles

* Change 'wifi_ad' to something you want to test.
* There are details about [Log Level](https://www.nsnam.org/docs/manual/html/logging.html#severity-and-level-options).
* Can get Log of chatting in './log/*room.dat' files.
* Can get Log of throughput in './log/*_pt.dat' file. The format of data is 'time "\t" throughput'.
* Commnd Line Arguments

```c++
uint32_t client_n = 11;
bool verbose=false;		
std::string DataRate="10Mbps";
CommandLine cmd;
cmd.AddValue("verbose","Logging or not",verbose);
cmd.AddValue("client_n","The number of clients",client_n);
cmd.AddValue("datarate","Setting the datarate of the channel", DataRate);
cmd.Parse (argc, argv);
```


```console
foo@bar:~$ export 'NS_LOG=wifi_ad=level_all'
foo@bar:~$ ./waf --run 'wifi_ad' > ./log/wifi_ad.dat 2>&1
foo@bar:~$ bash get_log.sh log/wifi_ad
```
