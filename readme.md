# ZeroMQ-spy

This is a simple program for spying on zeromq communications. It creates
two sockets and forwards data back and forth between them, printing each
message to the console.

To use this program, simply insert it between your client and server. The
program will connect one socket to the address given in the "server socket"
command-line option, and will listen on the address given in the "listen
socket" command-line option.

This program is compatible with REQ, REP, DEALER, and ROUTER socket types.

This program is available under an MIT license.
