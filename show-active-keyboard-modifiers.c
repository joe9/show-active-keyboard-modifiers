
/* copied the below from test_xi2.c of xinput source code

   got the xinput source code from
     git clone git://anongit.freedesktop.org/xorg/app/xinput
   to build xinput:
    cd /home/j/dev/apps/x11/xinput && autoreconf -i && ./configure && make

    got this idea from

    http://stackoverflow.com/questions/16369850/xinput-2-key-code-to-string
    http://unix.stackexchange.com/questions/129159/record-every-keystroke-and-store-in-a-file

    to print the modifier keysym, check out
      PrintModifierMapping of exec.c of xmodmap source

    to compile:
	gcc -o show-active-keyboard-modifiers \
	`pkg-config --cflags --libs xi` \
	`pkg-config --cflags --libs x11` show-active-keyboard-modifiers.c

 */

/* #include "xinput.h" */
#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
/* which declares:
     KeySym XkbKeycodeToKeysym(Display *dpy, KeyCode kc,
			       unsigned int group, unsigned int level); */
#include <stdio.h>
#include <stdlib.h>

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 1
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 0
#endif

int xinput_version(Display* display);

#include <string.h>
int xi_opcode;

extern struct modtab {
   const char *name;
   char *first_keysym_name;
} modifier_table[];

struct modtab modifier_table[] = {	/* keep in order so it can be index */
   { "shift", 0   },
   { "lock", 0    },
   { "control", 0 },
   { "mod1", 0    },
   { "mod2", 0    },
   { "mod3", 0    },
   { "mod4", 0    },
   { "mod5", 0    }};

static void output(int check, char * label, FILE *fp)
{
   int first_set_bit = 1;
   int i = 0;
   if (0 < check) {
      fprintf(fp, "%s:[", label);
      for (i = 0; i < 8; i++) {
	 if ((1 << i) & check) {
	    fprintf(fp, "%s%s",
		    (first_set_bit ? "" : ","),
		    modifier_table[i].first_keysym_name);
	    first_set_bit = 0;
	 }
      }
      fprintf(fp, "]");
   }
}

static void print_deviceevent(XIDeviceEvent* event)
{
   double *val;
   int i;

   printf("    device: %d (%d)\n", event->deviceid, event->sourceid);
   printf("    detail: %d\n", event->detail);
   switch(event->evtype) {
      case XI_KeyPress:
      case XI_KeyRelease:
	 printf("    flags: %s\n", (event->flags & XIKeyRepeat) ?  "repeat" : "");
	 break;
#if HAVE_XI21
      case XI_ButtonPress:
      case XI_ButtonRelease:
      case XI_Motion:
	 printf("    flags: %s\n", (event->flags & XIPointerEmulated) ?  "emulated" : "");
	 break;
#endif
   }

   printf("    root: %.2f/%.2f\n", event->root_x, event->root_y);
   printf("    event: %.2f/%.2f\n", event->event_x, event->event_y);

   printf("    buttons:");
   for (i = 0; i < event->buttons.mask_len * 8; i++)
      if (XIMaskIsSet(event->buttons.mask, i))
	 printf(" %d", i);
   printf("\n");

   printf("    modifiers: locked %#x latched %#x base %#x effective: %#x\n",
	  event->mods.locked, event->mods.latched,
	  event->mods.base, event->mods.effective);
   printf("    group: locked %#x latched %#x base %#x effective: %#x\n",
	  event->group.locked, event->group.latched,
	  event->group.base, event->group.effective);
   printf("    valuators:\n");

   val = event->valuators.values;
   for (i = 0; i < event->valuators.mask_len * 8; i++)
      if (XIMaskIsSet(event->valuators.mask, i))
	 printf("        %i: %.2f\n", i, *val++);

   printf("    windows: root 0x%lx event 0x%lx child 0x%lx\n",
	  event->root, event->event, event->child);
}

static const char* type_to_name(int evtype)
{
   const char *name;

   switch(evtype) {
      case XI_DeviceChanged:    name = "DeviceChanged";       break;
      case XI_KeyPress:         name = "KeyPress";            break;
      case XI_KeyRelease:       name = "KeyRelease";          break;
      case XI_ButtonPress:      name = "ButtonPress";         break;
      case XI_ButtonRelease:    name = "ButtonRelease";       break;
      case XI_Motion:           name = "Motion";              break;
      case XI_Enter:            name = "Enter";               break;
      case XI_Leave:            name = "Leave";               break;
      case XI_FocusIn:          name = "FocusIn";             break;
      case XI_FocusOut:         name = "FocusOut";            break;
      case XI_HierarchyChanged: name = "HierarchyChanged";    break;
      case XI_PropertyEvent:    name = "PropertyEvent";       break;
      case XI_RawKeyPress:      name = "RawKeyPress";         break;
      case XI_RawKeyRelease:    name = "RawKeyRelease";       break;
      case XI_RawButtonPress:   name = "RawButtonPress";      break;
      case XI_RawButtonRelease: name = "RawButtonRelease";    break;
      case XI_RawMotion:        name = "RawMotion";           break;
      case XI_TouchBegin:       name = "TouchBegin";          break;
      case XI_TouchUpdate:      name = "TouchUpdate";         break;
      case XI_TouchEnd:         name = "TouchEnd";            break;
      case XI_RawTouchBegin:    name = "RawTouchBegin";       break;
      case XI_RawTouchUpdate:   name = "RawTouchUpdate";      break;
      case XI_RawTouchEnd:      name = "RawTouchEnd";         break;
      default:
	 name = "unknown event type"; break;
   }
   return name;
}


static int
print_version(void)
{
   XExtensionVersion	*version;
   Display *display;

   display = XOpenDisplay(NULL);

   printf("XI version on server: ");

   if (display == NULL)
      printf("Failed to open display.\n");
   else {
      version = XGetExtensionVersion(display, INAME);
      if (!version || (version == (XExtensionVersion*) NoSuchExtension))
	 printf(" Extension not supported.\n");
      else {
	 printf("%d.%d\n", version->major_version,
		version->minor_version);
	 XFree(version);
	 return 0;
      }
   }

   return 1;
}

int
xinput_version(Display	*display)
{
   XExtensionVersion	*version;
   static int vers = -1;

   if (vers != -1)
      return vers;

   version = XGetExtensionVersion(display, INAME);

   if (version && (version != (XExtensionVersion*) NoSuchExtension)) {
      vers = version->major_version;
      XFree(version);
   }

   /* Announce our supported version so the server treats us correctly. */
   if (vers >= XI_2_Major)
   {
      const char *forced_version;
      int maj = 2,
	    min = 2; /* HAVE_XI22 */

      forced_version = getenv("XINPUT_XI2_VERSION");
      if (forced_version) {
	 if (sscanf(forced_version, "%d.%d", &maj, &min) != 2) {
	    fprintf(stderr, "Invalid format of XINPUT_XI2_VERSION "
		    "environment variable. Need major.minor\n");
	    exit(1);
	 }
	 printf("Overriding XI2 version to: %d.%d\n", maj, min);
      }

      XIQueryVersion(display, &maj, &min);
   }

   return vers;
}

int
main(int argc, char * argv[])
{
   Display	*display;
   XIEventMask mask[1] = { 0 };
   XIEventMask *m;
   Window win;
   int use_root = 0;
   int rc;
   int event, error;
   XModifierKeymap *map = NULL;
   unsigned char byte = 1;
   int i = 0;
   XIDeviceEvent* cookieData;
   int min_keycode, max_keycode, keysyms_per_keycode = 0;

   display = XOpenDisplay(NULL);

   if (display == NULL) {
      fprintf(stderr, "Unable to connect to X server\n");
      goto out;
   }

   if (!XQueryExtension(display, "XInputExtension", &xi_opcode, &event, &error)) {
      printf("X Input extension not available.\n");
      goto out;
   }

   if (!xinput_version(display)) {
      fprintf(stderr, "%s extension not available\n", INAME);
      goto out;
   }

   /* set line buffering to stdout */
   setvbuf(stdout, NULL, _IOLBF, 0);

   /* got this from
      PrintModifierMapping of exec.c of xmodmap source */
   map = XGetModifierMapping (display);
   XDisplayKeycodes (display, &min_keycode, &max_keycode);
   XGetKeyboardMapping (display, min_keycode, (max_keycode - min_keycode + 1),
			&keysyms_per_keycode);
   for (i = 0; i < 8; i++) {
      /*       fprintf(stdout, "%-10s", modifier_table[i].name); */

      int j = 0;
      for (j = 0; j < map->max_keypermod; j++) {
	 int k = (i * map->max_keypermod) + j;
	 if (map->modifiermap[k]) {
	    KeySym ks;
	    int index = 0;
	    char *nm;
	    do {
	       ks = XkbKeycodeToKeysym( display, map->modifiermap[k],
					0, index);
	       index++;
	    } while ( !ks && index < keysyms_per_keycode);
	    nm = XKeysymToString(ks);
	    /*	    fprintf (stdout, "%s (0x%0x) ", */
	    /*		     (nm ? nm : "Badkey") , map->modifiermap[k]); */
	    if (0 == j && nm) {
	       modifier_table[i].first_keysym_name = nm;
	    }
	 }
      }
/*       fprintf(stdout, "\n"); */
   }
/*    fprintf (stdout, "\n"); */

   use_root = 1;
   win = DefaultRootWindow(display);

   /* Select for motion events */
   m = &mask[0];
   m->deviceid = XIAllDevices;
   m->mask_len = XIMaskLen(XI_LASTEVENT);
   m->mask = calloc(m->mask_len, sizeof(char));
   XISetMask(m->mask, XI_KeyPress);
   XISetMask(m->mask, XI_KeyRelease);

   XISelectEvents(display, win, &mask[0], 1);
   XSync(display, False);

   free(mask[0].mask);

   while(1)
   {
      XEvent ev;
      XGenericEventCookie *cookie = (XGenericEventCookie*)&ev.xcookie;
      XNextEvent(display, (XEvent*)&ev);

      if (XGetEventData(display, cookie) &&
	  cookie->type == GenericEvent &&
	  cookie->extension == xi_opcode)
      {
/*	 printf("EVENT type %d (%s)\n", cookie->evtype, type_to_name(cookie->evtype)); */
	 switch (cookie->evtype)
	 {
	    case XI_KeyPress:
	    case XI_KeyRelease:
	    default:
	       /*	       print_deviceevent(cookie->data); */
	       cookieData = cookie->data;
	       output(cookieData->mods.locked, "Locked", stdout);
	       output(cookieData->mods.latched, " Latched", stdout);
	       output(cookieData->mods.effective, " Effective", stdout);
	       fprintf(stdout, "\n");
	       break;
	 }
      }
      XFreeEventData(display, cookie);
   }

   XDestroyWindow(display, win);

   XSync(display, False);
   XCloseDisplay(display);
   return EXIT_SUCCESS;

out:
   if (display)
      XCloseDisplay(display);
   return EXIT_FAILURE;
}
