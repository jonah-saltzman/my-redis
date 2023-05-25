# Use Ubuntu as the base image
FROM ubuntu:latest

# Set the working directory in the Docker container
WORKDIR /usr/src/app

# Install the necessary tools
RUN apt-get update && apt-get install -y \
    build-essential \
    g++-aarch64-linux-gnu \
    gcc-aarch64-linux-gnu

# Copy the contents of the current directory (on your system) to the working directory in the container
COPY . .

# Set the path to the cross compiler
ENV CXX=aarch64-linux-gnu-g++

# Run the make command to compile the application
RUN make clean && make
RUN ls bin/

# Command to run when the container starts
CMD ["./bin/redis_server"]
