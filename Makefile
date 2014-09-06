all: attpd

init:
	mkdir ./out

attpd: init
	gcc server.c -o out/attpd

clean:
	rm -rf out
