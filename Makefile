CC := clang

TESTSRC := test_htable.c
SRC := htable.c

OBJ := $(SRC:%=build/%.o)

TESTFLAGS := -lpthread -fPIC  -std=c99 -ggdb -Wall -Wextra -Wshadow -fsanitize=undefined  -fno-omit-frame-pointer -fsanitize=address
CFLAGS := -lpthread -fPIC -std=c99 -O3  -Wall -Wextra -Wshadow

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
	$(CC) $(TESTFLAGS) $(TESTSRC) $(SRC) -o $@

htable.o: bin/htable.o
	cp $^ $@

bin/htable.o: $(OBJ)
	mkdir -p $(dir $@)
	ld -r $(OBJ) -o $@

build/%.c.o: %.c $(SRC)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
