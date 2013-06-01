/*
 * gEDA - GNU Electronic Design Automation
 * This file is a part of gerbv.
 *
 *   Copyright (C) 2000-2003 Stefan Petersen (spe@stacken.kth.se)
 *
 * $Id$
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111 USA
 */

//! \defgroup libgerbv libgerbv
//! \defgroup gerbv Gerbv

/** \file gerbv-gdk.h
    \brief The main header file for the libgerbv-gdk library
    \ingroup libgerbv
*/

#ifndef __GERBV_GDK_H__
#define __GERBV_GDK_H__

#include "gerbv.h"

#include <glib.h>
#include <gdk/gdk.h>

#if defined(__cplusplus)
extern "C" {
#endif

void
gerbv_render_to_pixmap_using_gdk (gerbv_project_t *gerbvProject, GdkPixmap *pixmap,
		gerbv_render_info_t *renderInfo, gerbv_selection_info_t *selectionInfo,
		GdkColor *selectionColor);

#if defined(__cplusplus)
}
#endif

#endif /* __GERBV_GDK_H__ */
