#!/bin/bash
rm CompleteBST.out
rm CompleteBSTDriver.o
rm Song.o
rm CD.o

g++ -I./ -I./CSC1310/include -c Song.cpp
g++ -I./ -I./CSC1310/include -c CD.cpp
g++ -I./ -I./random/include -I./CSC1310/include -c CompleteBSTDriver.cpp
g++ -L./ -L./random/lib -L./CSC1310/lib -o CompleteBST.out CompleteBSTDriver.o Song.o CD.o -lrandom -lCSC1310 -pthread
./CompleteBST.out
