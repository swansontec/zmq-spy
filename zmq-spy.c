/*
 * Copyright 2014 William Swanson
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * “Software”), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 */

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <zmq.h>

volatile sig_atomic_t done = 0;

void handle_break(int signal)
{
    done = 1;
}

int show_error(char const *where)
{
    fprintf(stderr, "error: %s: %s\n", where, strerror(errno));
    return 1;
}

/**
 * Formats and displays binary data.
 */
void display(void *data, size_t size)
{
    fputc('\t', stdout);
    fwrite(data, size, 1, stdout);
    fputc('\n', stdout);
}

/**
 * Forwards a complete message from the in socket to the out socket.
 */
void forward_message(void *in, void *out)
{
    int more;
    do {
        /* Read: */
        zmq_msg_t msg;
        zmq_msg_init(&msg);
        zmq_msg_recv(&msg, in, 0);
        more = zmq_msg_more(&msg);

        /* Display: */
        display(zmq_msg_data(&msg), zmq_msg_size(&msg));

        /* Forward: */
        zmq_msg_send(&msg, out, more ? ZMQ_SNDMORE : 0);
        zmq_msg_close(&msg);
    } while (more);
}

int main(int argc, char *argv[])
{
    /* Check command-line arguments: */
    if (3 != argc) {
        fprintf(stderr, "usage: %s <listen address> <server address>\n",
            argv[0]);
        return 1;
    }

    /* Set up signal handling: */
    if (SIG_ERR == signal(SIGINT, handle_break))
        return show_error("installing signal handler");

    /* Create zeromq resources: */
    void *ctx = zmq_ctx_new();
    if (!ctx)
        return show_error("creating context");
    void *listen = zmq_socket(ctx, ZMQ_DEALER);
    if (!listen)
        return show_error("creating listening socket");
    void *server = zmq_socket(ctx, ZMQ_DEALER);
    if (!server)
        return show_error("creating server-side socket");

    /* Establish connections: */
    if (-1 == zmq_bind(listen, argv[1]))
        return show_error("binding listening socket");
    if (-1 == zmq_connect(server, argv[2]))
        return show_error("connecting to server");

    /* Main loop: */
    while (!done) {
        zmq_pollitem_t items[] = {
            {listen, 0, ZMQ_POLLIN, 0},
            {server, 0, ZMQ_POLLIN, 0}
        };
        zmq_poll(items, 2, -1);

        if (items[0].revents & ZMQ_POLLIN) {
            printf("client -> server:\n");
            forward_message(listen, server);
        }
        if (items[1].revents & ZMQ_POLLIN) {
            printf("client <- server:\n");
            forward_message(server, listen);
        }
    }

    fprintf(stderr, "shutting down...\n");
    zmq_close(listen);
    zmq_close(server);
    zmq_ctx_destroy(ctx);
    return 0;
}
