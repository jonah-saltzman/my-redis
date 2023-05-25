#!/bin/bash

# Function to stop server
function stop_server {
    echo "Caught SIGTERM signal! Stopping server..."
    kill $server_pid
}

# Start the server in the background
./bin/redis_server &
server_pid=$!

echo "Server started pid=$server_pid"

# Stop server on SIGTERM
trap stop_server SIGTERM

# Start the client
./bin/redis_client
