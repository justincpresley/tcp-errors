# tcp-errors | Are errors getting past TCP?

This repository is responsible for creating a client and server that will detect errors that slip past TCP. It utilizes multiple protocols such as COBS, Vertical and Horizontal Parities, and CRCs in order to accomplish this. Effectively and simply, a strong integrity-detection file server and client is provided.

There has been little research on TCP since it's creation in the 1970s. Yet, large file downloads are seeing major drops in speed, and TCP could be the culprit. Is this the case? This repository aims to find out.

## Running

To run, do the following.

* Clone our repo. Move into the main directory.
```
git clone https://github.com/justincpresley/tcp-errors.git
cd tcp-errors
```

* Make all executables
```
make
```

* Test all modules
```
./build/test
```

* Running the Server
```
./build/server PORT
./build/server 7777
```

* Running the Client (model than example)
```
./build/client IP PORT FILE DOWNLOADS
./build/client localhost 7777 sample.txt 3
```

* Clean after running
```
make clean
cd ..
rm -rf ./tcp-errors
```