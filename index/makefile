libsl.a: sorted-list.o
	ar r libsl.a sorted-list.o

sl: main.o sorted-list.o
	gcc -o sl main.o sorted-list.o

main.o: main.c sorted-list.h
	gcc -c main.c

sorted-list.o: sorted-list.c sorted-list.h
	gcc -c sorted-list.c

clean:
	rm -f sorted-list
	rm -f *.o