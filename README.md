# Run Example

change 'wifi_ad' to something you want to test

```console
foo@bar:~$ export 'NS_LOG=wifi_ad=level_all'
foo@bar:~$ ./waf --run 'wifi_ad' > ./log/wifi_ad.dat 2>&1
foo@bar:~$ bash get_log.sh log/wifi_ad
```