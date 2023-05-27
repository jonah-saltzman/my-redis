#!/bin/bash

function stop_server {
    kill "$server_pid"
    exit 0
}

function handle_interrupt {
    stop_server
}

./redis_server &
server_pid=$!

sleep 0.2

trap handle_interrupt SIGINT
trap stop_server SIGTERM

./redis_client

stop_server
