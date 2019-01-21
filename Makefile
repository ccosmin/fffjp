%.o: %.c 
	$(CC) -c -o $@ $< $(CFLAGS)

main_test: main_test.o sls_buffer.o sls_json.o ringbuf.o
	gcc -o $@ $^

main: main.o sls_buffer.o sls_json.o ringbuf.o
	gcc -o $@ $^

pretty_print_json: pretty_print_json.o sls_buffer.o sls_json.o ringbuf.o
	gcc -o $@ $^
