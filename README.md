# Run Example to Get Logfiles

### Docker
Our Docker image [Docker Hub](https://hub.docker.com/r/dbswogks1534/team1)
When using Docker, move to root/NWproj_Team1/ in container

### Directories
We implemented Chat client, Chat server, chat header and some Example scenario files.
* copy chat-client.\*, chat-server.* chat-header.* in src/application/model/
* copy chat-client-helper.\*, chat-server-helper.* in src/application/helper/
* copy get_log.sh in base directory
* modify src/application/wscript file

### CMD 
2 arguments for example scenario files.
* "verbose" for either logging or not
* "client_n" for the number of clients

### Executing & Log file
* Change 'wifi_ad' to something you want to test and arguments as you want.
* There are details about [Log Level](https://www.nsnam.org/docs/manual/html/logging.html#severity-and-level-options).
* Can get Log of chatting in './log/*room.dat' files.
* Can get Log of throughput in './log/*_pt.dat' file. The format of data is 'time "\t" throughput'.

```console
foo@bar:~$ ./waf --run "wifi_ad --verbose=true --client_n=10" > ./log/wifi_ad.dat
foo@bar:~$ bash get_log.sh log/wifi_ad
```
