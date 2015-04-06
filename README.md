# show-active-keyboard-modifiers
X11 show active keyboard modifiers

got this idea from
* http://stackoverflow.com/questions/16369850/xinput-2-key-code-to-string
* http://unix.stackexchange.com/questions/129159/record-every-keystroke-and-store-in-a-file

```Shell

copied the below from test_xi2.c of xinput source code

got the xinput source code from
    git clone git://anongit.freedesktop.org/xorg/app/xinput
to build xinput:
cd /home/j/dev/apps/x11/xinput && autoreconf -i && ./configure && make

to print the modifier keysym, check out
    PrintModifierMapping of exec.c of xmodmap source from
    http://cgit.freedesktop.org/xorg/app/xmodmap/

to compile:
    gcc -o show-active-keyboard-modifiers \
    `pkg-config --cflags --libs xi` \
    `pkg-config --cflags --libs x11` show-active-keyboard-modifiers.c

usage: ./show-active-keyboard-modifiers | sed -e 's/_[LR]//g'

```

checkout XI2 tutorials at:
* http://who-t.blogspot.com/2009/05/xi2-recipes-part-1.html
* http://who-t.blogspot.com/2009/07/xi2-recipes-part-4.html
* http://who-t.blogspot.com/2009/07/xi2-and-xlib-cookies.html

