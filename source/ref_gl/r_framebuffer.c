/*
Copyright (C) 2008 Victor Luchits

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

// r_framebuffer.c - Framebuffer Objects support

#include "r_local.h"

#define MAX_FRAMEBUFFER_OBJECTS	    1024

typedef struct
{
	bool registered;
	int registrationSequence; // -1 if builtin
	unsigned int objectID;
	unsigned int depthRenderBuffer;
	bool depthStencilRenderBuffer;
	int width, height;
	image_t *depthTexture;
	image_t *colorTexture;
} r_fbo_t;

static bool r_frambuffer_objects_initialized;
static bool r_framebuffer_objects_rendering_enabled;
static int r_bound_framebuffer_objectID;
static r_fbo_t *r_bound_framebuffer_object;
static int r_num_framebuffer_objects;
static r_fbo_t r_framebuffer_objects[MAX_FRAMEBUFFER_OBJECTS];

/*
* RFB_Init
*/
void RFB_Init( void )
{
	r_num_framebuffer_objects = 0;
	memset( r_framebuffer_objects, 0, sizeof( r_framebuffer_objects ) );

	r_framebuffer_objects_rendering_enabled = false;

	r_frambuffer_objects_initialized = true;
}

/*
* RFB_DeleteObject
* 
* Delete framebuffer object along with attached render buffer
*/
static void RFB_DeleteObject( r_fbo_t *fbo )
{
	if( fbo->depthRenderBuffer )
	{
		qglDeleteRenderbuffersEXT( 1, &( fbo->depthRenderBuffer ) );
		fbo->depthRenderBuffer = 0;
	}

	if( fbo->objectID )
	{
		qglDeleteFramebuffersEXT( 1, &( fbo->objectID ) );
		fbo->objectID = 0;
	}

	fbo->registered = false;
}

/*
* RFB_CreateGLObjectForFBO
*
* Creates the object name for the FBO so it can be bound in the current context.
*/
static void RFB_CreateGLObjectForFBO( r_fbo_t *fbo )
{
#ifndef GL_ES_VERSION_2_0
	int drawBuffer = ( fbo->colorTexture ? GL_COLOR_ATTACHMENT0_EXT : GL_NONE );
#endif

	qglGenFramebuffersEXT( 1, &fbo->objectID );

	qglBindFramebufferEXT( GL_FRAMEBUFFER_EXT, fbo->objectID );

#ifndef GL_ES_VERSION_2_0
	qglDrawBuffer( drawBuffer );
	qglReadBuffer( drawBuffer );
#endif

	if( fbo->colorTexture ) {
		qglFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fbo->colorTexture->texnum, 0 );
	}

	if( fbo->depthTexture ) {
		qglFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, fbo->depthTexture->texnum, 0 );
		if( ( fbo->depthTexture->flags & ( IT_DEPTH|IT_STENCIL ) ) == ( IT_DEPTH|IT_STENCIL ) ) {
			qglFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_2D, fbo->depthTexture->texnum, 0 );
		}
	} else if( fbo->depthRenderBuffer ) {
		qglFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, fbo->depthRenderBuffer );
		if( fbo->depthStencilRenderBuffer ) {
			qglFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, fbo->depthRenderBuffer );
		}
	}

	if( r_bound_framebuffer_objectID ) {
		qglBindFramebufferEXT( GL_FRAMEBUFFER_EXT, r_bound_framebuffer_object->objectID );
	} else {
		qglBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
	}
}

/*
* RFB_InitRendering
*
* Initializes the usage of FBOs in the current context.
* Only one GL context can use FBOs at once.
*/
void RFB_InitRendering( void )
{
	int i;
	r_fbo_t *fbo;

	if( !r_frambuffer_objects_initialized )
		return;

	r_framebuffer_objects_rendering_enabled = true;

	qglBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
	r_bound_framebuffer_objectID = 0;
	r_bound_framebuffer_object = NULL;

	for( i = 0, fbo = r_framebuffer_objects; i < r_num_framebuffer_objects; i++, fbo++ ) {
		if( fbo->registered ) {
			RFB_CreateGLObjectForFBO( fbo );
		}
	}
}

/*
* RFB_RegisterObject
*/
int RFB_RegisterObject( int width, int height, bool builtin, bool depthRB, bool stencilRB )
{
	int i;
	GLuint fbID;
	GLuint rbID;
	r_fbo_t *fbo;

	if( !r_frambuffer_objects_initialized )
		return 0;

	for( i = 0, fbo = r_framebuffer_objects; i < r_num_framebuffer_objects; i++, fbo++ ) {
		if( !fbo->registered ) {
			// free slot
			goto found;
		}
	}

	if( i == MAX_FRAMEBUFFER_OBJECTS )
	{
		Com_Printf( S_COLOR_YELLOW "RFB_RegisterObject: framebuffer objects limit exceeded\n" );
		return 0;
	}

	i = r_num_framebuffer_objects++;
	fbo = r_framebuffer_objects + i;

found:
	memset( fbo, 0, sizeof( *fbo ) );
	fbo->registered = true;
	if( builtin )
		fbo->registrationSequence = -1;
	else
		fbo->registrationSequence = rsh.registrationSequence;
	fbo->width = width;
	fbo->height = height;

	if( depthRB ) {
		int format;

		qglGenRenderbuffersEXT( 1, &rbID );
		fbo->depthRenderBuffer = rbID;
		qglBindRenderbufferEXT( GL_RENDERBUFFER_EXT, rbID );

		if( stencilRB ) {
			format = GL_DEPTH24_STENCIL8_EXT;
			fbo->depthStencilRenderBuffer = true;
		} else if( glConfig.ext.depth24 ) {
			format = GL_DEPTH_COMPONENT24;
		} else if( glConfig.ext.depth_nonlinear ) {
			format = GL_DEPTH_COMPONENT16_NONLINEAR_NV;
		} else {
			format = GL_DEPTH_COMPONENT16;
		}
		qglRenderbufferStorageEXT( GL_RENDERBUFFER_EXT, format, width, height );

		qglBindRenderbufferEXT( GL_RENDERBUFFER_EXT, 0 );	
	}

	if( r_framebuffer_objects_rendering_enabled ) {
		RFB_CreateGLObjectForFBO( fbo );
	}

	return i+1;
}

/*
* RFB_UnregisterObject
*/
void RFB_UnregisterObject( int object )
{
	r_fbo_t *fbo;

	assert( object > 0 && object <= r_num_framebuffer_objects );
	if( !object ) {
		return;
	}

	fbo = r_framebuffer_objects + object - 1;
	RFB_DeleteObject( fbo );
}

/*
* RFB_TouchObject
*/
void RFB_TouchObject( int object )
{
	r_fbo_t *fbo;

	assert( object > 0 && object <= r_num_framebuffer_objects );
	if( !object ) {
		return;
	}

	fbo = r_framebuffer_objects + object - 1;
	fbo->registrationSequence = rsh.registrationSequence;
}

/*
* RFB_BoundObject
*/
int RFB_BoundObject( void )
{
	return r_bound_framebuffer_objectID;
}

/*
* RFB_BindObject
*
* DO NOT call this function directly, use R_BindFrameBufferObject instead.
*/
void RFB_BindObject( int object )
{
	if( !object )
	{
		if( r_frambuffer_objects_initialized )
			qglBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
		r_bound_framebuffer_objectID = 0;
		r_bound_framebuffer_object = NULL;
		return;
	}

	if( !r_frambuffer_objects_initialized ) {
		return;
	}

	assert( object > 0 && object <= r_num_framebuffer_objects );
	if( object <= 0 || object > r_num_framebuffer_objects ) {
		return;
	}

	if( r_bound_framebuffer_objectID == object ) {
		return;
	}

	r_bound_framebuffer_objectID = object;
	r_bound_framebuffer_object = r_framebuffer_objects + object - 1;
	qglBindFramebufferEXT( GL_FRAMEBUFFER_EXT, r_bound_framebuffer_object->objectID );
}

/*
* RFB_AttachTextureToObject
*/
void RFB_AttachTextureToObject( int object, image_t *texture )
{
	r_fbo_t *fbo;
	int attachment;

	assert( object > 0 && object <= r_num_framebuffer_objects );
	if( object <= 0 || object > r_num_framebuffer_objects ) {
		return;
	}

	assert( texture != NULL );
	if( !texture ) {
		return;
	}

	fbo = r_framebuffer_objects + object - 1;

	if( r_framebuffer_objects_rendering_enabled ) {
		qglBindFramebufferEXT( GL_FRAMEBUFFER_EXT, fbo->objectID );
	}

	if( texture->flags & IT_DEPTH ) {
		attachment = GL_DEPTH_ATTACHMENT_EXT;
		fbo->depthTexture = texture;
	} else {
		attachment = GL_COLOR_ATTACHMENT0_EXT;
		fbo->colorTexture = texture;
#ifndef GL_ES_VERSION_2_0
		if( r_framebuffer_objects_rendering_enabled ) {
			qglDrawBuffer( GL_COLOR_ATTACHMENT0_EXT );
			qglReadBuffer( GL_COLOR_ATTACHMENT0_EXT );
		}
#endif
	}
	texture->fbo = object;

	if( r_framebuffer_objects_rendering_enabled ) {
		// attach texture
		qglFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, attachment, GL_TEXTURE_2D, texture->texnum, 0 );
		if( ( texture->flags & ( IT_DEPTH|IT_STENCIL ) ) == ( IT_DEPTH|IT_STENCIL ) ) {
			qglFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_2D, texture->texnum, 0 );
		}

		qglBindFramebufferEXT( GL_FRAMEBUFFER_EXT, r_bound_framebuffer_objectID ? r_bound_framebuffer_object->objectID : 0 );
	}
}

/*
* RFB_GetObjectTextureAttachment
*/
image_t	*RFB_GetObjectTextureAttachment( int object, bool depth )
{
	r_fbo_t *fbo;

	assert( object > 0 && object <= r_num_framebuffer_objects );
	if( object <= 0 || object > r_num_framebuffer_objects ) {
		return NULL;
	}

	fbo = r_framebuffer_objects + object - 1;
	return depth ? fbo->depthTexture : fbo->colorTexture;
}

/*
* RFB_BlitObject
*
* The target FBO must be equal or greater in both dimentions than
* the currently bound FBO!
*/
void RFB_BlitObject( int dest, int bitMask, int mode )
{
	int bits;
	int dx, dy, dw, dh;
	r_fbo_t *fbo = r_bound_framebuffer_object, 
		*destfbo = r_framebuffer_objects + dest - 1;

	if( !r_bound_framebuffer_object ) {
		return;
	}
	if( !glConfig.ext.framebuffer_blit ) {
		return;
	}

	assert( dest > 0 && dest <= r_num_framebuffer_objects );
	if( dest <= 0 || dest > r_num_framebuffer_objects ) {
		return;
	}

	bits = bitMask;
	if( !bits ) {
		return;
	}

	RB_ApplyScissor();

	switch( mode ) {
		case FBO_COPY_CENTREPOS:
			dx = (destfbo->width - fbo->width) / 2;
			dy = (destfbo->height - fbo->height) / 2;
			dw = fbo->width;
			dh = fbo->height;
			break;
		case FBO_COPY_INVERT_Y:
			dx = 0;
			dy = destfbo->height - fbo->height;
			dw = fbo->width;
			dh = fbo->height;
			break;
		default:
			dx = 0;
			dy = 0;
			dw = fbo->width;
			dh = fbo->height;
			break;
	}

	qglBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
	qglBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, fbo->objectID );
	qglBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, destfbo->objectID );
	qglBlitFramebufferEXT( 0, 0, fbo->width, fbo->height, dx, dy, dx + dw, dy + dh, bits, GL_NEAREST );
	qglBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, 0 );
	qglBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, 0 );
	qglBindFramebufferEXT( GL_FRAMEBUFFER_EXT, fbo->objectID );

	assert( qglGetError() == GL_NO_ERROR );
}

/*
* RFB_CheckObjectStatus
* 
* Boolean, returns false in case of error
*/
bool RFB_CheckObjectStatus( void )
{
	GLenum status;

	if( !r_frambuffer_objects_initialized )
		return false;

	status = qglCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT );
	switch( status )
	{
		case GL_FRAMEBUFFER_COMPLETE_EXT:
			return true;
		case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
			return false;
		default:
			// programming error; will fail on all hardware
			assert( 0 );
	}
	
	return false;
}

/*
* RFB_GetObjectSize
*/
void RFB_GetObjectSize( int object, int *width, int *height )
{
	r_fbo_t *fbo;

	if( !object ) {
		*width = glConfig.width;
		*height = glConfig.height;
		return;
	}

	assert( object > 0 && object <= r_num_framebuffer_objects );
	if( object <= 0 || object > r_num_framebuffer_objects ) {
		return;
	}

	fbo = r_framebuffer_objects + object - 1;
	*width = fbo->width;
	*height = fbo->height;
}

/*
* RFB_FreeUnusedObjects
*/
void RFB_FreeUnusedObjects( void )
{
	int i;
	r_fbo_t *fbo = r_framebuffer_objects;
	int registrationSequence;

	if( !r_frambuffer_objects_initialized )
		return;

	for( i = 0; i < r_num_framebuffer_objects; i++, fbo++ ) {
		registrationSequence = fbo->registrationSequence;
		if( ( registrationSequence < 0 ) || ( registrationSequence == rsh.registrationSequence ) ) {
			continue;
		}
		RFB_DeleteObject( fbo );
	}
}

/*
* RFB_ShutdownRendering
*
* Disables FBO binding in the current context and destroys the GL objects of FBOs.
*/
void RFB_ShutdownRendering( void )
{
	int i;
	r_fbo_t *fbo;

	if( !r_frambuffer_objects_initialized || !r_framebuffer_objects_rendering_enabled )
		return;

	r_framebuffer_objects_rendering_enabled = false;

	qglBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
	r_bound_framebuffer_objectID = 0;
	r_bound_framebuffer_object = NULL;

	for( i = 0, fbo = r_framebuffer_objects; i < r_num_framebuffer_objects; i++, fbo++ ) {
		if( fbo->registered && fbo->objectID ) {
			qglDeleteFramebuffersEXT( 1, &fbo->objectID );
			fbo->objectID = 0;
		}
	}
}

/*
* RFB_Shutdown
* 
* Delete all registered framebuffer and render buffer objects, clear memory
*/
void RFB_Shutdown( void )
{
	int i;

	if( !r_frambuffer_objects_initialized )
		return;

	RFB_ShutdownRendering();

	for( i = 0; i < r_num_framebuffer_objects; i++ ) {
		RFB_DeleteObject( r_framebuffer_objects + i );
	}

	r_frambuffer_objects_initialized = false;
	r_num_framebuffer_objects = 0;
	memset( r_framebuffer_objects, 0, sizeof( r_framebuffer_objects ) );
}
