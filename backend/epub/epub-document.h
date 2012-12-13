/* epub-document.h: Implementation of EvDocument for epub book archives
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

#ifndef __EPUB_DOCUMENT_H__
#define __EPUB_DOCUMENT_H__

#include "ev-document.h"

G_BEGIN_DECLS

#define EPUB_TYPE_DOCUMENT    (epub_document_get_type ())
#define EPUB_DOCUMENT(obj)    (G_TYPE_CHECK_INSTANCE_CAST ((obj), EPUB_TYPE_DOCUMENT, EpubDocument))
#define EPUB_IS_DOCUMENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EPUB_TYPE_DOCUMENT))

typedef struct _EpubDocument EpubDocument;

GType                 epub_document_get_type (void) G_GNUC_CONST;

G_MODULE_EXPORT GType register_evince_backend  (GTypeModule *module);

G_END_DECLS

#endif /* __EPUB_DOCUMENT_H__ */
