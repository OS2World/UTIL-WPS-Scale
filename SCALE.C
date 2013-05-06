/* SCALE.C -- Program to display grayscale (or redscale, or cyanscale, or...)
   Version 0.94 */
/* This program is copyright (c) 1992 by Raja Thiagarajan. However, you may
   use it for any non-commercial purpose. Thanks to Peter Nielsen
   (pnielsen@finabo.abo.fi) for some clever ideas.
   You may contact me at sthiagar@bronze.ucs.indiana.edu (internet) or
   72175,12 (compuserve). I check my internet account almost every
   day, but only check compuserve once a week or so. */

#include <stdlib.h>         /* include atoi(), ldiv(), max(), min() */
#define INCL_GPI
#define INCL_WIN
#include <os2.h>

MRESULT EXPENTRY ClientWinProc (HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2);

static USHORT redTarg;
static USHORT greenTarg;
static USHORT blueTarg;

/* Read redTarg, greenTarg, and blueTarg from the command line */
VOID ParseArgs (INT ac, CHAR * av[])
{
   USHORT j;
   redTarg = greenTarg = blueTarg = 0;
   for (j = 1; j < ac; j++) {
      CHAR a = av[j][0];
      if ((a == '-') || (a == '/')) {
         CHAR a = av[j][1];
         SHORT v = atoi (&av[j][2]);
#define MIN_VAL 0
#define MAX_VAL 255
         v = min (max (v, MIN_VAL), MAX_VAL);
         if (a == 'r') {
            redTarg = v;
         } else if (a == 'g') {
            greenTarg = v;
         } else if (a == 'b') {
            blueTarg = v;
         }
      }
   }
   if (!(redTarg | greenTarg | blueTarg)) {
      redTarg = greenTarg = blueTarg = MAX_VAL;
   }
}

static LONG   hasPalMan;      /* Whether the Palette Manager is available */
static LONG   colorsToShow;   /* How many colored rectangles to draw */
static HAB    hab;            /* This program's hab */

INT main (INT argc, CHAR * argv [])
{
   HMQ   hmq;         /* The usual bunch of variables for PM programs */
   QMSG  qmsg;
   HWND  hwnd,
         hwndClient;

   ULONG createFlags  =  FCF_TITLEBAR | FCF_SYSMENU | FCF_SIZEBORDER
                                      | FCF_MINMAX | FCF_SHELLPOSITION
                                      | FCF_TASKLIST;

#define clientClass "scale"

   ParseArgs (argc, argv);

   {
      LONG sColors; /* How many colors are available */
      HDC hdcScr;   /* Used to query the screen driver */
      hdcScr = GpiQueryDevice (WinGetPS (HWND_DESKTOP));
      DevQueryCaps (hdcScr, CAPS_COLORS, 1L, &sColors);
      DevQueryCaps (hdcScr, CAPS_ADDITIONAL_GRAPHICS, 1, &hasPalMan);
      hasPalMan &= CAPS_PALETTE_MANAGER;
      colorsToShow = sColors;
#define MAX_SHADES 64
      if (colorsToShow > MAX_SHADES) {
        colorsToShow = MAX_SHADES;
      }
   }

   hab = WinInitialize (0);  /* initialize PM usage */

   hmq = WinCreateMsgQueue (hab, 0);   /* create message queue */

   WinRegisterClass (hab, clientClass, (PFNWP) ClientWinProc, CS_SIZEREDRAW, 0);

      /* Create standard window and client. Note that the title message
         depends on whether the Palette Manager is available or not */
   if (hasPalMan) {
      hwnd = WinCreateStdWindow (HWND_DESKTOP, WS_VISIBLE, &createFlags,
                                 clientClass, "Palette-Managed Color Scale", 0L,
                                 0UL, 0, &hwndClient);
   } else {
      hwnd = WinCreateStdWindow (HWND_DESKTOP, WS_VISIBLE, &createFlags,
                                 clientClass, "Color Scale", 0L, 0UL, 0,
                                 &hwndClient);
   }

   while (WinGetMsg (hab, &qmsg, NULLHANDLE, 0, 0)) { /*  msg dispatch loop */
      WinDispatchMsg (hab, &qmsg);
   }

   WinDestroyWindow (hwnd);           /* destroy frame window */
   WinDestroyMsgQueue (hmq);          /* destroy message queue */
   WinTerminate (hab);                /* terminate PM usage */

   return 0;
}

MRESULT EXPENTRY ClientWinProc (HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
{
   static HPS    hps;
   static HDC    hdc;
   static HPAL   hpal;
   static USHORT cWid, cHi; /* client area width and height */
   static BOOL   firstPaint = TRUE; /* Whether this is the first response
                                       to WM_PAINT */

   switch (msg)
      {
         case WM_CREATE:
            {
               SIZEL sizl = {0L, 0L};
               hdc = WinOpenWindowDC (hwnd);
               hps = GpiCreatePS (hab, hdc, &sizl, PU_PELS | GPIF_DEFAULT
                        | GPIT_MICRO | GPIA_ASSOC);
               if (hasPalMan) {
                  ULONG  tbl [MAX_SHADES];
                  INT    j;
                  for (j = 0; j < colorsToShow; j++) {
                     USHORT rj = redTarg * j / (colorsToShow - 1);
                     USHORT gj = greenTarg * j / (colorsToShow - 1);
                     USHORT bj = blueTarg * j / (colorsToShow - 1);
                     tbl [j] = 65536 * rj + 256 * gj + bj;
                  }
                  hpal = GpiCreatePalette (hab, 0L, LCOLF_CONSECRGB,
                                           colorsToShow, tbl);
                  GpiSelectPalette (hps, hpal);
               }
            }
            return (MRESULT) FALSE;
         case WM_DESTROY:
            if (hasPalMan) {
               GpiSelectPalette (hps, NULLHANDLE);
               GpiDeletePalette (hpal);
            }
            GpiDestroyPS (hps);
            return (MRESULT) FALSE;
         case WM_ERASEBACKGROUND:
            return (MRESULT) TRUE;
         case WM_PAINT:
            {
               POINTL ptl;
               LONG   j;

               WinBeginPaint (hwnd, hps, NULL);
               if (hasPalMan && firstPaint) {
                 /* Realize the palette when you paint for the first time.
                    This is necessary in case the program doesn't start
                    in the foreground. */
                 ULONG palSize = colorsToShow;
                 WinRealizePalette (hwnd, hps, &palSize);
                 firstPaint = FALSE;
               }
               for (j = 0; j < colorsToShow; j++) {
                  if (hasPalMan) { /* use the exact color */
                     GpiSetColor (hps, j);
                  } else { /* find a reasonable facsimile */
                     USHORT rj = redTarg * j / (colorsToShow - 1);
                     USHORT gj = greenTarg * j / (colorsToShow - 1);
                     USHORT bj = blueTarg * j / (colorsToShow - 1);
                     LONG i = GpiQueryColorIndex (hps, 0, 65536 * rj
                                                          + 256 * gj + bj);
                     GpiSetColor (hps, i);
                  }
                  ptl.y = 0;
                  ptl.x = j * cWid / colorsToShow;
                  GpiMove (hps, &ptl);
                  ptl.y = cHi;
                  ptl.x += cWid / colorsToShow;
                  GpiBox (hps, DRO_FILL, &ptl, 0L, 0L);
               }
               WinEndPaint (hps);
            }
            return (MRESULT) FALSE;
         case WM_REALIZEPALETTE:
            {
               ULONG palSize = colorsToShow;
               if (hasPalMan) {
                  if (WinRealizePalette (hwnd, hps, &palSize)) {
                     WinInvalidateRect (hwnd, NULL, FALSE);
                  }
               }
               return (MRESULT) FALSE;
            }
         case WM_SIZE:
            cWid = SHORT1FROMMP (mp2);
            cHi = SHORT2FROMMP (mp2);
            return (MRESULT) FALSE;
         default:
            return (WinDefWindowProc (hwnd, msg, mp1, mp2));
            break;
      }  /*end switch*/
   return (MRESULT) FALSE;
}
