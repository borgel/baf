INC_DIR = include/
OBJECTS = baf.o
CFLAGS = -g -Wall -I$(INC_DIR)

.PHONY: all clean

all: baf.a tester

%.o: src/%.c
	gcc $(CFLAGS) -c $< -o $@

%.o: test/%.c
	gcc $(CFLAGS) -c $< -o $@

baf.a: $(OBJECTS)
	ar rcs $@ $^

tester: baf.a main.o
	gcc $(CFLAGS) $^ -o $@

clean:
	-rm -f $(OBJECTS)
	-rm -f baf.a

	-rm -f tester
	-rm -f test/*.o

