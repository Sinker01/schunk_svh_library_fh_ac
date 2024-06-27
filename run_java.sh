#!/bin/bash
cd src

javac -d ../build fingermanager/Finger.java

java -Djava.library.path=/usr/local/lib -cp ../build Main