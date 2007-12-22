/*
 * iconbox.c - icon widget
 *
 * Copyright © 2007 Julien Danjou <julien@danjou.info>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <X11/Xmd.h>
#include <X11/Xatom.h>
#include <confuse.h>
#include "util.h"
#include "focus.h"
#include "layout.h"
#include "widget.h"

extern AwesomeConf globalconf;

static int
netwmicon_draw(Widget *widget, DrawCtx *ctx, int offset,
                    int used __attribute__ ((unused)))
{
    unsigned long *data, pixel;
    Atom type;
    int format, location, width, height, size, i;
    unsigned long items, rest;
    unsigned char *image, *imgdata;
    VirtScreen vscreen = globalconf.screens[widget->statusbar->screen];
    Client *sel = focus_get_latest_client_for_tag(widget->statusbar->screen,
                                                  get_current_tag(widget->statusbar->screen));

    if(!sel)
        return 0;

    if(XGetWindowProperty(ctx->display, sel->win, 
                          XInternAtom(ctx->display, "_NET_WM_ICON", False),
                          0L, LONG_MAX, False, XA_CARDINAL, &type, &format,
                          &items, &rest, (unsigned char**) &data) != Success
       || !data)
        return 0;

    if(type != XA_CARDINAL || format != 32 || items < 2)
    {
        XFree(data);
        return 0;
    }

    width = data[0];
    height = data[1];
    size = width * height;

    if(!size)
    {
        XFree(data);
        return 0;
    }

    image = p_new(unsigned char, size * 4);
    for(imgdata = image, i = 2; i < size + 2; i++, imgdata += 4)
    {
        pixel = data[i];
        imgdata[3] = (pixel >> 24) & 0xff; /* A */
        imgdata[0] = (pixel >> 16) & 0xff; /* R */
        imgdata[1] = (pixel >>  8) & 0xff; /* G */
        imgdata[2] = pixel & 0xff;         /* B */
    }

    location = widget_calculate_offset(vscreen.statusbar->width,
                                       width,
                                       offset,
                                       widget->alignment);

    draw_image_from_argb_data(ctx, location, 0, width, height, vscreen.statusbar->height, image);

    p_delete(&image);

    XFree(data);
    return vscreen.statusbar->height;
}

Widget *
netwmicon_new(Statusbar *statusbar, cfg_t *config)
{
    Widget *w;

    w = p_new(Widget, 1);
    widget_common_new(w, statusbar, config);
    w->draw = netwmicon_draw;
    
    return w;
}
// vim: filetype=c:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=80
