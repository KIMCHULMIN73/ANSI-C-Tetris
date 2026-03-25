############################
##   Makefile of TETRIS   ##
############################
## CopyLeft by Kimchulmin ##
##        2024.10         ##
############################


# MAKE EXECUTABLE FILE
build:
	gcc -o ./output/tetris tetris_main.c tetris_frontend.c tetris_backend.c tetris_resource.c -lncurses

# EXECUTE TETRIS
run:
	./output/tetris

# CLEAN TETRIS	
clean:
	rm ./output/tetris
