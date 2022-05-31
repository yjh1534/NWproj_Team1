# Run Example

change 'wifi_ad' to something you want to test

```console
$export 'NS_LOG=wifi_ad=level_all'
$./waf --run 'wifi_ad' > ./log/wifi_ad.dat 2>&1
$bash get_log.sh log/wifi_ad
```