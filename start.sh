#!/bin/bash

function stop_server {
    kill "$server_pid"
    exit 0
}

function handle_interrupt {
    stop_server
}

./bin/redis_server &
server_pid=$!

sleep 0.1

trap handle_interrupt SIGINT
trap stop_server SIGTERM

./bin/redis_client

stop_server
