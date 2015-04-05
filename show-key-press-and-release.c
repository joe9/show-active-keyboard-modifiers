/*
 * Copyright © 2009 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */


/* #include "xinput.h" */
#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <X11/Xutil.h>
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

void
test_sync_grab(Display *display, Window win)
{
    int loop = 3;
    int rc;
    XIEventMask mask;

    /* Select for motion events */
    mask.deviceid = XIAllDevices;
    mask.mask_len = 2;
    mask.mask = calloc(2, sizeof(char));
    XISetMask(mask.mask, XI_ButtonPress);

    if ((rc = XIGrabDevice(display, 2,  win, CurrentTime, None, GrabModeSync,
			   GrabModeAsync, False, &mask)) != GrabSuccess)
    {
	fprintf(stderr, "Grab failed with %d\n", rc);
	return;
    }
    free(mask.mask);

    XSync(display, True);
    XIAllowEvents(display, 2, SyncPointer, CurrentTime);
    XFlush(display);

    printf("Holding sync grab for %d button presses.\n", loop);

    while(loop--)
    {
	XIEvent ev;

	XNextEvent(display, (XEvent*)&ev);
	if (ev.type == GenericEvent && ev.extension == xi_opcode )
	{
	    XIDeviceEvent *event = (XIDeviceEvent*)&ev;
	    print_deviceevent(event);
	    XIAllowEvents(display, 2, SyncPointer, CurrentTime);
	}
    }

    XIUngrabDevice(display, 2, CurrentTime);
    printf("Done\n");
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

   setvbuf(stdout, NULL, _IOLBF, 0);

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

   /*
     test_sync_grab(display, win);
   */

   while(1)
   {
      XEvent ev;
      XGenericEventCookie *cookie = (XGenericEventCookie*)&ev.xcookie;
      XNextEvent(display, (XEvent*)&ev);

      if (XGetEventData(display, cookie) &&
	  cookie->type == GenericEvent &&
	  cookie->extension == xi_opcode)
      {
	 printf("EVENT type %d (%s)\n", cookie->evtype, type_to_name(cookie->evtype));
	 switch (cookie->evtype)
	 {
	    case XI_KeyPress:
	    case XI_KeyRelease:
	    default:
	       print_deviceevent(cookie->data);
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
