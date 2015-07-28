# vmperf
Open source Linux benchmarking in C

Tests CPU, memory, network and disk performance and produces JSON output for easy parsing.

A few known bugs that will be addressed when time permits - mainly with the disk tests.

To build, just run `make`.

Sample output:
```json
{"vm_perf":"0.4","modules":{"system":{"uname":"Linux version 3.6.10 (admin@ns1) (gcc version 4.4.7 20120313 (Red Hat 4.4.7-3) (GCC) ) #1 SMP Fri Apr 5 13:57:15 BST 2013","hostname":"server.loadingdeck.com","cpu_model":"Intel(R) Xeon(R) CPU E5-2690 v2 @ 3.00GHz","cpu_count":"6","ram_total":"3999MB","ram_free":"1475MB"},"cpu":[ {"test":"DHRYSTONE","result":"213763083"},{"test":"C-RAY F","result":"914"},{"test":"C-RAY MT","result":"286"}],"net":[ {"hostname":"speedtest.ams01.softlayer.com","latency":"7.12ms","dns_query":"0.00ms"},{"hostname":"speedtest.dal01.softlayer.com","latency":"111.56ms","dns_query":"0.00ms"},{"hostname":"speedtest.fra02.softlayer.com","latency":"19.53ms","dns_query":"0.00ms"},{"hostname":"speedtest.hkg02.softlayer.com","latency":"-699.28ms","dns_query":"0.00ms"},{"hostname":"speedtest.lon02.softlayer.com","latency":"2.05ms","dns_query":"0.00ms"},{"hostname":"speedtest.mel01.softlayer.com","latency":"293.44ms","dns_query":"0.00ms"},{"hostname":"speedtest.par01.softlayer.com","latency":"8.62ms","dns_query":"0.00ms"},{"hostname":"speedtest.mex01.softlayer.com","latency":"0.00ms","dns_query":"0.00ms"},{"hostname":"speedtest.sjc01.softlayer.com","latency":"142.63ms","dns_query":"0.00ms"},{"hostname":"speedtest.tok02.softlayer.com","latency":"-754.91ms","dns_query":"0.00ms"},{"hostname":"speedtest.tor01.softlayer.com","latency":"98.82ms","dns_query":"0.00ms"},{"hostname":"speedtest.wdc01.softlayer.com","latency":"76.05ms","dns_query":"0.00ms"}],"mem":{"stream":[ {"test":"Copy","result":"10364.6MB/s"},{"test":"Scale","result":"9945.9MB/s"},{"test":"Add","result":"10841.6MB/s"},{"test":"Triad","result":"11336.9MB/s"}]},"storage":{"write_test":[ {"type":"random","rate":"187.92MB/s","buf_size":"262144b"},{"type":"sequential","rate":"183.46MB/s","buf_size":"262144b"},{"type":"cached_sequential","rate":"1097.07MB/s","buf_size":"262144b"}],"disks":[ {"name":"/dev/xvda","size":"87189480.00GB","blocks":"15603165218434815","block_size":"6b","read_tests":[ {"type":"random","buf_size":"4096b","seek/read/s":"2","access_time":"-nanms","rate":"0.01KB/s"},{"type":"sequential","buf_size":"262144b","seek/read/s":"1","access_time":"0.00ms","rate":"0.00MB/s"},{"type":"cached_sequential","buf_size":"262144b","seek/read/s":"0","access_time":"0.00ms","rate":"0.00MB/s"}]}]}}}
```
