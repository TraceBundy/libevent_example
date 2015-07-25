#INCLUDE=-I/usr/local/include/
all:echo client
echo:
	g++ ${INCLUDE} -g -o echo echo.c -levent -lrt
client:
	g++ ${INCLUDE} -g -o client client.c -levent -lrt
clean:
	rm echo client
