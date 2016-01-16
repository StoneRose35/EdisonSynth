#!/bin/bash

g++ -O0 -g3 -Wall -c -fmessage-length=0 -o "bin/Envelope.o" "src/Envelope.cpp"
g++ -O0 -g3 -Wall -c -fmessage-length=0 -o "bin/Filter.o" "src/Filter.cpp"
g++ -O0 -g3 -Wall -c -fmessage-length=0 -o "bin/LFO.o" "src/LFO.cpp"
g++ -O0 -g3 -Wall -c -fmessage-length=0 -o "bin/Oscillator.o" "src/Oscillator.cpp"
g++ -O0 -g3 -Wall -c -fmessage-length=0 -o "bin/Voice.o" "src/Voice.cpp"
g++ -O0 -g3 -Wall -c -fmessage-length=0 -o "bin/edisonSynth.o" "src/edisonSynth.cpp"

g++  -o "edisonSynth_linux"  ./bin/Envelope.o ./bin/Filter.o ./bin/LFO.o ./bin/Oscillator.o ./bin/Voice.o ./bin/edisonSynth.o -lasound
 

