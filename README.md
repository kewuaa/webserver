# build

```sh
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -G "Ninja"
cmake --build build
```

# bench

use Apache Bench tool

```sh
ab -n 1000000 -c 1000 -k http:/127.0.0.1:12345/
```

output below:

```
Server Software:        
Server Hostname:        127.0.0.1
Server Port:            12345

Document Path:          /
Document Length:        2069 bytes

Concurrency Level:      1000
Time taken for tests:   479.896 seconds
Complete requests:      10000000
Failed requests:        0
Keep-Alive requests:    10000000
Total transferred:      22050000000 bytes
HTML transferred:       20690000000 bytes
Requests per second:    20837.83 [#/sec] (mean)
Time per request:       47.990 [ms] (mean)
Time per request:       0.048 [ms] (mean, across all concurrent requests)
Transfer rate:          44870.52 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.4      0      51
Processing:    24   48   3.8     47     125
Waiting:        0   48   3.8     47     125
Total:         24   48   3.8     47     125

Percentage of the requests served within a certain time (ms)
  50%     47
  66%     48
  75%     48
  80%     49
  90%     51
  95%     55
  98%     59
  99%     64
 100%    125 (longest request)
```
