PYTHON_CFLAGS := $(shell python3-config --cflags --embed)
PYTHON_LDFLAGS := $(shell python3-config --ldflags --embed)

heap_miner: heap_miner.c
	gcc $(PYTHON_CFLAGS) $(INCLUDES) -O0 $? -o $@ $(PYTHON_LDFLAGS)