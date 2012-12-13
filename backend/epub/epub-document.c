/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; c-indent-level: 8 -*- */
/*
 * Copyright (C) 2012 Daniel Garcia <danigm@wadobo.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <config.h>

#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <libgepub/gepub.h>

#include "epub-document.h"
#include "ev-document-misc.h"
#include "ev-file-helpers.h"


#define WIDTH 660
#define HEIGHT 900

#define MARGIN 40


typedef struct _EpubDocumentClass EpubDocumentClass;

struct _EpubDocumentClass
{
	EvDocumentClass parent_class;
};

struct _EpubDocument
{
	EvDocument parent_instance;

	GepubDoc *gepub_doc;
	GList *epub_pages;
};

EV_BACKEND_REGISTER (EpubDocument, epub_document)

static GList*
epub_document_read_pages (EvDocument *doc) {
	int lines, lines2;
	int width, height;
	PangoContext *context;
	PangoLayout *layout;
	gchar *text = NULL, *prev_text = NULL, *chunk_text = NULL;
	GList *l = NULL, *chunks = NULL, *pages = NULL;
	EpubDocument *epub_document = EPUB_DOCUMENT (doc);

	width = WIDTH - MARGIN * 2;
	height = HEIGHT - MARGIN * 2;

	context = gdk_pango_context_get_for_screen (gdk_screen_get_default ());
	layout = pango_layout_new (context);

	pango_layout_set_width (layout, width * PANGO_SCALE);
	pango_layout_set_ellipsize (layout, PANGO_ELLIPSIZE_NONE);

	chunks = gepub_doc_get_text (epub_document->gepub_doc);

	prev_text = g_strdup("");

	GepubTextChunk *chunk = NULL;
	for (l=chunks; l; l = l->next) {
		chunk = GEPUB_TEXT_CHUNK (l->data);
		chunk_text = NULL;
		text = NULL;
		if (chunk->type == GEPUBTextHeader) {
			chunk_text = g_strdup_printf ("\n<span size=\"x-large\" font-weight=\"bold\">%s</span>\n", chunk->text);
		} else if (chunk->type == GEPUBTextNormal) {
			chunk_text = g_strdup_printf ("%s", chunk->text);
		} else if (chunk->type == GEPUBTextItalic) {
			chunk_text = g_strdup_printf ("<i>%s</i>", chunk->text);
		} else if (chunk->type == GEPUBTextBold) {
			chunk_text = g_strdup_printf ("<b>%s</b>", chunk->text);
		}

		if (!chunk_text)
			continue;

		text = g_strdup_printf ("%s%s", prev_text, chunk_text);

		pango_layout_set_height (layout, -1);
		pango_layout_set_markup (layout, text, -1);
		lines = pango_layout_get_line_count (layout);

		pango_layout_set_height (layout, height * PANGO_SCALE);
		pango_layout_set_markup (layout, text, -1);
		lines2 = pango_layout_get_line_count (layout);

		if (lines2 < lines) {
			pages = g_list_append (pages, prev_text);
			prev_text = chunk_text;
		} else {
			g_free (prev_text);
			prev_text = text;
		}
	}

	if (prev_text && strcmp (prev_text, ""))
		pages = g_list_append (pages, prev_text);

	g_object_unref (layout);
	g_object_unref (context);

	return pages;
}

static gboolean
epub_document_load (EvDocument *document,
		    const char *uri,
		    GError    **error)
{
	EpubDocument *epub_document = EPUB_DOCUMENT (document);
	gchar *filename = NULL;
	GList *spine = NULL;

	filename = g_filename_from_uri (uri, NULL, error);
	epub_document->gepub_doc = gepub_doc_new (filename);

	spine = gepub_doc_get_spine (epub_document->gepub_doc);
	// All chapters
	while (spine->next) {
		// reading chapter pages
		epub_document->epub_pages = g_list_concat (epub_document->epub_pages, epub_document_read_pages (document));
		gepub_doc_go_next (epub_document->gepub_doc);
		spine = gepub_doc_get_spine (epub_document->gepub_doc);
	}

	return epub_document->gepub_doc != NULL;
}

static int
epub_document_get_n_pages (EvDocument *document)
{
	EpubDocument *epub_document = EPUB_DOCUMENT (document);

	return g_list_length (epub_document->epub_pages);
}

static void
epub_document_get_page_size (EvDocument *document,
			     EvPage     *page,
			     double     *width,
			     double     *height)
{
	*width = (double)WIDTH;
	*height = (double)HEIGHT;
}

static cairo_surface_t *
epub_document_render (EvDocument      *document,
		      EvRenderContext *rc)
{
	cairo_surface_t *surface;
	cairo_t *cr;
	double width_points, height_points;
	int width, height;
	PangoLayout *layout;
	gchar *text = NULL;

	EpubDocument *epub_document = EPUB_DOCUMENT (document);

	epub_document_get_page_size (document,
				     rc->page,
				     &width_points, &height_points);

	if (rc->rotation == 90 || rc->rotation == 270) {
		width = (int) ((height_points * rc->scale));
		height = (int) ((width_points * rc->scale));
	} else {
		width = (int) ((width_points * rc->scale));
		height = (int) ((height_points * rc->scale));
	}

	surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
					      width, height);

	cr = cairo_create (surface);
	cairo_translate (cr, MARGIN * rc->scale, MARGIN * rc->scale);
	cairo_scale (cr, rc->scale, rc->scale);
	cairo_rotate (cr, rc->rotation * (M_PI / 180));

	cairo_set_source_rgb (cr, 0, 0, 0);
	layout = pango_cairo_create_layout (cr);
	pango_layout_set_width (layout, (width_points-MARGIN) * PANGO_SCALE);
	pango_layout_set_height (layout, (height_points-MARGIN) * PANGO_SCALE);

	text = g_list_nth_data (epub_document->epub_pages, rc->page->index);

	if (text) {
		pango_layout_set_markup (layout, text, -1);

		pango_cairo_show_layout (cr, layout);
	}

	g_object_unref (layout);
	cairo_destroy (cr);

	return surface;
}

static void
epub_document_finalize (GObject *object)
{
	EpubDocument *epub_document = EPUB_DOCUMENT (object);


	if (epub_document->gepub_doc) {
		g_object_unref (G_OBJECT (epub_document->gepub_doc));
	}

	if (epub_document->epub_pages) {
		g_list_foreach (epub_document->epub_pages, (GFunc)g_free, NULL);
		g_list_free (epub_document->epub_pages);
	}

	G_OBJECT_CLASS (epub_document_parent_class)->finalize (object);
}

static void
epub_document_class_init (EpubDocumentClass *klass)
{
	GObjectClass    *gobject_class = G_OBJECT_CLASS (klass);
	EvDocumentClass *ev_document_class = EV_DOCUMENT_CLASS (klass);

	gobject_class->finalize = epub_document_finalize;

	ev_document_class->load = epub_document_load;
	ev_document_class->get_n_pages = epub_document_get_n_pages;
	ev_document_class->get_page_size = epub_document_get_page_size;
	ev_document_class->render = epub_document_render;
}

static void
epub_document_init (EpubDocument *epub_document)
{
	epub_document->gepub_doc = NULL;
	epub_document->epub_pages = NULL;
}
