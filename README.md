## Building

    make all

## Usage

To start the server run `vledd`. Server runs in foreground.

Run `vledctl --help` to get information on how to use the client.

## Implementation

Server is a single-threaded event-based program, that uses _epoll_ for event processing.
Server visualizes LED on the terminal, any error messages are sent to stderr.
Server is communicating with client using named pipes, clients are responsible to serialize their access to pipes.
Server does all I/O in nonblocking mode.
