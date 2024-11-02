all:
	gcc -o test-log test-log.c -g -O0 -I../ -I../../utils/ -L../lib -L../../config/lib/ -lkylog -lkyconf -lpthread -lsystemd -Wl,-rpath=../lib:../../config/lib/
	gcc -o test-setdir test-setdir.c -g -O0 -I../ -I../../utils/ -L../lib -L../../config/lib/ -lkylog -lkyconf -lpthread -lsystemd -Wl,-rpath=../lib:../../config/lib/
	gcc -o test-pressure test-pressure.c -g -O0 -I../ -I../../utils/ -L../lib -L../../config/lib/ -lkylog -lkyconf -lpthread -lsystemd -Wl,-rpath=../lib:../../config/lib/

clean:
	-rm test-log test-setdir test-pressure