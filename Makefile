CFLAGS += -Wall $(shell pkg-config --cflags libzmq)
LIBS += $(shell pkg-config --libs libzmq)

zmq-spy: zmq-spy.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	$(RM) *.o
	$(RM) zmq-spy

# Automatic dependency rules:
%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<
