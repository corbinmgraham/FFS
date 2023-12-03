main:
	gcc -g filesystem.c journal.c block_service.c -lpthread -o journal.exe

clean:
	rm *.exe
