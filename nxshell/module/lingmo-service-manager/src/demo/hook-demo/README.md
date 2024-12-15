Expected results:
./demo-server
Data: "BBB"

./demo-client
Server1:setData fail
Server2:setData success


Actual results:
./demo-server
Data: "AAA"  // BUG1: the call cannot be aborted
Data: "BBB"

./demo-client
Server1:setData fail
Server2:setData fail // BUG2:service2 not need check, but it is impossible to judge whether the call is service1 or service2