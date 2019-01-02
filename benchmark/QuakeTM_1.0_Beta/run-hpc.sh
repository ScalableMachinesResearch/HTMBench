#!/bin/bash
set -x
CURRENT_DIR=$(pwd)

###tunable variables
CLIENT_DIR=$CURRENT_DIR/client/TraceBot
CLIENT_NUM=${THREADS} #the number of clients
#CLIENT_CORE_BINDING=(58 62 66 70 74 78 82 86 90 94 98 102 106 110)

SERVER_DIR=$CURRENT_DIR/server
SERVER_NUM=${THREADS} #the number of server threads
SERVER_FRAMES=100 #it determines how long the server will run

###launch clients
cd $CLIENT_DIR
if [ ! -d "output" ]; then
  mkdir output
fi

for ((i=0;i<$CLIENT_NUM;i++))
do
#  echo "client $i starts";
  #taskset -c ${CLIENT_CORE_BINDING[i]} ./glqwclient -run_trace -name player$i >output/player$i &
  ./glqwclient -run_trace -name player$i >output/player$i &
  sleep 0.1;
done;

sleep 1
cd $CURRENT_DIR

###launch server
#echo "Now starts to launch server"
ln -s $SERVER_DIR/id1
eval $SERVER_DIR/qwsv -threads $SERVER_NUM -frames $SERVER_FRAMES
unlink id1
sleep 1
###Stop clients
#echo "Now stop clients"
killall glqwclient > /dev/null >&2
sleep 1
