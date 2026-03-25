############################
##   Makefile of TETRIS   ##
############################
## CopyLeft by Kimchulmin ##
##        2024.10         ##
############################


# MAKE OBJECT FILE
build:
	gcc -c tetris_main.c tetris_frontend.c tetris_backend.c tetris_resource.c 

# MAKE EXECUTE FILE
	gcc -o tetris tetris_main.o tetris_frontend.o tetris_backend.o tetris_resource.o -lncurses

# EXECUTE TETRIS
	./tetris
	
clean:
	rm *.o
	rm tetris
