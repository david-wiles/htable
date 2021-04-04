CC := gcc

TESTSRC := test_htable.c
SRC := htable.c

OBJ := $(SRC:%=build/%.o)

TESTFLAGS := -pthread -fPIC  -std=gnu99 -ggdb -Wall -Wextra -Wshadow -fsanitize=undefined  -fno-omit-frame-pointer -fsanitize=address -I/usr/include
CFLAGS := -pthread -fPIC -std=gnu99 -O3  -Wall -Wextra -Wshadow -I/usr/include

.PHONY: all
all: clean htable.o test

.PHONY: clean
clean:
	rm -rf bin
	rm -rf build
	rm -rf debug

.PHONY: test
test: debug/test
	./$^
	rm $^

debug/test: clean
	mkdir -p $(dir $@)
	$(CC) $(TESTFLAGS) $(SRC) $(TESTSRC) -o $@

htable.o: bin/htable.o
	cp $^ $@

bin/htable.o: $(OBJ)
	mkdir -p $(dir $@)
	ld -r $(OBJ) -o $@

build/%.c.o: %.c $(SRC)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
