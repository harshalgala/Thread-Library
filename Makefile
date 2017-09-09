all:
	# Create the static library
	gcc -g -c  queue.c -o queue.o
	gcc -g -c mythread.c -o mythread.o
	ar rcs mythread.a mythread.o queue.o

	# Compile against static library
	#gcc -g -o test tests/test.c mythread.a
	#gcc -g -o fibb tests/fibb.c mythread.a
	#gcc -g -o one tests/one.c mythread.a
	#gcc -g -o passing tests/passing.c mythread.a
	#gcc -g -o ping tests/ping.c mythread.a
	#gcc -g -o tree tests/tree.c mythread.a
	#gcc -g -o sem_test tests/sem_test.c mythread.a

clean:
	rm -rf a.out
	rm -rf test
	rm -rf fibb
	rm -rf passing
	rm -rf one
	rm -rf ping
	rm -rf tree
	rm -rf sem_test
	rm -rf *.o
	rm -rf *.a
