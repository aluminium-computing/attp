all: attpd

init: 
	mkdir -p ./out

attpd: init
	gcc server.c -o out/attpd

clean:
	rm -rf out