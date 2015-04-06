
all : show-active-keyboard-modifiers

show-active-keyboard-modifiers : show-active-keyboard-modifiers.c
	gcc -o show-active-keyboard-modifiers \
	  `pkg-config --cflags --libs xi` \
	  `pkg-config --cflags --libs x11` show-active-keyboard-modifiers.c

clean :
	rm show-active-keyboard-modifiers
