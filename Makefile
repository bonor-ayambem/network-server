server: libhashmap.a server.o static
	g++ build/objects/release/server.o -o server -lhashmap -Lbuild/lib/release -std=c++17
	mkdir -p build/bin/release
	mv server build/bin/release

static:
	mkdir -p build
	mkdir -p build/lib
	mkdir -p build/lib/release
	mkdir -p build/objects
	g++ src/lib.cpp -c -I include -I lib
	ar rs libhashmap.a lib.o
	mv *.o build/objects
	mv *.a build/lib/release

server.o:
	g++ src/bin/server.cpp -c -lhashmap -Lbuild/lib/release -Iinclude -std=c++17
	mkdir -p build/objects/release
	mv server.o build/objects/release

libhashmap.a:
	mkdir -p build
	mkdir -p build/lib
	mkdir -p build/lib/release
	mkdir -p build/objects
	g++ src/lib.cpp -c -I include -I lib
	ar rs libhashmap.a lib.o
	
clean:
	rm -rf build