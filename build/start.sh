#!/bin/bash

function stop_server {
    kill "$server_pid"
    exit 0
}

./redis_server &
server_pid=$!

sleep 1

trap stop_server SIGINT
trap stop_server SIGTERM

./redis_client

stop_server
