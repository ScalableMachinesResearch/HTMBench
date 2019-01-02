.PHONY: all client server clean run

all: client server


client:
	$(MAKE) -C client/TraceBot

server:
	$(MAKE) -C server


clean:
	$(MAKE) -C client/TraceBot clean
	$(MAKE) -C server clean
	rm -rf output
	unlink id1
