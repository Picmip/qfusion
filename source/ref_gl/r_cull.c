/*
Copyright (C) 2007 Victor Luchits

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

#include "r_local.h"


/*
=============================================================

FRUSTUM AND PVS CULLING

=============================================================
*/

/*
* R_SetupSideViewFrustum
*/
void R_SetupSideViewFrustum( const refdef_t *rd, int side, float nearClip, float farClip, cplane_t *frustum, vec3_t corner[4] ) {
	int i;
	float sign;
	int a0, a1, a2;
	vec3_t forward, left, up;

	// 0 - left
	// 1 - right
	// 2 - down
	// 3 - up
	// 4 - nearclip
	// 5 - farclip

	sign = side & 1 ? -1 : 1;
	a0 = (AXIS_FORWARD + 3 * (side >> 1)) % 12;
	a1 = (AXIS_RIGHT + 3 * (side >> 1)) % 12;
	a2 = (AXIS_UP + 3 * (side >> 1)) % 12;

	VectorCopy( &rd->viewaxis[a0], forward );
	VectorCopy( &rd->viewaxis[a1], left );
	VectorCopy( &rd->viewaxis[a2], up );

	VectorScale( forward, sign, forward );

	if( rd->rdflags & RDF_USEORTHO ) {
		VectorNegate( left, frustum[0].normal );
		VectorCopy( left, frustum[1].normal );
		VectorNegate( up, frustum[2].normal );
		VectorCopy( up, frustum[3].normal );

		for( i = 0; i < 4; i++ ) {
			frustum[i].type = PLANE_NONAXIAL;
			frustum[i].dist = DotProduct( rd->vieworg, frustum[i].normal );
			frustum[i].signbits = SignbitsForPlane( &frustum[i] );
		}

		frustum[0].dist -= rd->ortho_x;
		frustum[1].dist -= rd->ortho_x;
		frustum[2].dist -= rd->ortho_y;
		frustum[3].dist -= rd->ortho_y;
	} else {
		vec3_t right;
		vec_t fpx, fnx, fpy, fny;
		vec_t dist;

		VectorNegate( left, right );

		// rotate rn.vpn right by FOV_X/2 degrees
		RotatePointAroundVector( frustum[0].normal, up, forward, -( 90 - rd->fov_x / 2 ) );
		// rotate rn.vpn left by FOV_X/2 degrees
		RotatePointAroundVector( frustum[1].normal, up, forward, 90 - rd->fov_x / 2 );
		// rotate rn.vpn up by FOV_X/2 degrees
		RotatePointAroundVector( frustum[2].normal, right, forward, 90 - rd->fov_y / 2 );
		// rotate rn.vpn down by FOV_X/2 degrees
		RotatePointAroundVector( frustum[3].normal, right, forward, -( 90 - rd->fov_y / 2 ) );

		for( i = 0; i < 4; i++ ) {
			frustum[i].type = PLANE_NONAXIAL;
			frustum[i].dist = DotProduct( rd->vieworg, frustum[i].normal );
			frustum[i].signbits = SignbitsForPlane( &frustum[i] );
		}

		// calculate frustum corners

		// change this dist to nearClip to calculate the near corner
		// instead of an arbitrary corner on the frustum
		dist = 1024.0f;

		fpx = dist * tan( rd->fov_x * M_PI / 360.0 );
		fnx = -1.0 * fpx;
		fpy = dist * tan( rd->fov_y * M_PI / 360.0 );
		fny = -1.0 * fpy;

		VectorMA( rd->vieworg, dist, forward, corner[0] );
		VectorMA( corner[0], fnx, left, corner[0] );
		VectorMA( corner[0], fny, up, corner[0] );

		VectorMA( rd->vieworg, dist, forward, corner[1] );
		VectorMA( corner[1], fnx, left, corner[1] );
		VectorMA( corner[1], fpy, up, corner[1] );

		VectorMA( rd->vieworg, dist, forward, corner[2] );
		VectorMA( corner[2], fpx, left, corner[2] );
		VectorMA( corner[2], fpy, up, corner[2] );

		VectorMA( rd->vieworg, dist, forward, corner[3] );
		VectorMA( corner[3], fpx, left, corner[3] );
		VectorMA( corner[3], fny, up, corner[3] );
	}

	// near clip
	VectorCopy( forward, frustum[4].normal );
	frustum[4].type = PLANE_NONAXIAL;
	frustum[4].dist = DotProduct( rd->vieworg, frustum[4].normal ) + nearClip;
	frustum[4].signbits = SignbitsForPlane( &frustum[4] );

	// farclip
	VectorNegate( forward, frustum[5].normal );
	frustum[5].type = PLANE_NONAXIAL;
	frustum[5].dist = DotProduct( rd->vieworg, frustum[5].normal ) - farClip;
	frustum[5].signbits = SignbitsForPlane( &frustum[5] );
}

/*
* R_SetupFrustum
*/
void R_SetupFrustum( const refdef_t *rd, float nearClip, float farClip, cplane_t *frustum, vec3_t corner[4] ) {
	R_SetupSideViewFrustum( rd, 0, nearClip, farClip, frustum, corner );
}

/*
* R_DeformFrustum
*
* Based off R_Shadow_ComputeShadowCasterCullingPlanes from Darkplaces
*
* FIXME: this should also handle the near and far frustum planes?..
*/
int R_DeformFrustum( const cplane_t *frustum, const vec3_t corners[4], const vec3_t origin, const vec3_t point, cplane_t *deformed ) {
	int i, j;
	int n;

	n = 0;

	for( i = 0; i < 4; i++ ) {
		if( PlaneDiff( point, &frustum[i] ) < -ON_EPSILON ) {
			// reject planes that put the point outside the frustum
			continue;
		}
		deformed[n++] = frustum[i];
	}

	if( n == 4 ) {
		return n;
	}

	// if the point is onscreen the result will be 4 planes exactly
	// if the point is offscreen on only one axis the result will
	// be exactly 5 planes (split-side case)
	// if the point is offscreen on two axes the result will be
	// exactly 4 planes (stretched corner case)

	for( i = 0; i < 4; i++ ) {
		cplane_t plane;

		// create a plane using the view origin and light origin, and a
		// single point from the frustum corner set
		TriangleNormal( origin, corners[i], point, plane.normal );

		VectorNormalize( plane.normal );
		plane.type = PLANE_NONAXIAL;
		plane.dist = DotProduct( origin, plane.normal );

		// see if this plane is backwards and flip it if so
		for( j = 0; j < 4; j++ ) {
			if( j != i && PlaneDiff( corners[j], &plane ) < -ON_EPSILON )
				break;
		}

		if( j < 4 ) {
			VectorNegate( plane.normal, plane.normal );
			plane.dist *= -1;

			// flipped plane, test again to see if it is now valid
			for( j = 0; j < 4; j++ ) {
				if( j != i && PlaneDiff( corners[j], &plane ) < -ON_EPSILON )
					break;
			}
		}

		// if the plane is still not valid, then it is dividing the
		// frustum and has to be rejected
		if( j < 4 ) {
			continue;
		}

		// we have created a valid plane, compute extra info
		CategorizePlane( &plane );
		deformed[n++] = plane;

		// if we've found 5 frustum planes then we have constructed a
		// proper split-side case and do not need to keep searching for
		// planes to enclose the light origin
		if( n == 5 ) {
			break;
		}
	}

	return n;
}

/*
* R_ScissorForCorners
* 
* Calculates 2-D scissor for bounding box defined as 8 corner verts in 3-D space.
* Returns true if the bbox is off-screen.
*/
bool R_ScissorForCorners( const refinst_t *rnp, vec3_t corner[8], int *scissor ) {
	int i;
	int clipped[8];
	float dist[8];
	int numVerts;
	vec4_t verts[8+12];
	float x1, y1, x2, y2;
	int ix1, iy1, ix2, iy2;

	numVerts = 0;
	for( i = 0; i < 8; i++ ) {
		dist[i] = PlaneDiff( corner[i], &rnp->frustum[4] );
		clipped[i] = dist[i] <= 0;

		if( !clipped[i] ) {
			VectorCopy( corner[i], verts[numVerts] );
			numVerts++;
		}
	}

	if( numVerts == 0 ) {
		// all points are behind the nearplane
		return true;
	}

	if( numVerts < 8 ) {
		// some points are crossing the nearplane, clip edges
		// insert vertices at intersection points
		for( i = 0; i < 12; i++ ) {
			int e1 = r_boxedges[i*2+0];
			int e2 = r_boxedges[i*2+1];

			if( clipped[e1] != clipped[e2] ) {
				vec_t frac = dist[e1] / (dist[e1] - dist[e2]);
				VectorLerp( corner[e1], frac, corner[e2], verts[numVerts] );
				numVerts++;
			}
		}
	}

	// project all vertices in front of the nearplane and from the clipped edges
	x1 = y1 = 999999;
	x2 = y2 = -999999;
	for( i = 0; i < numVerts; i++ ) {
		vec4_t v, proj;

		verts[i][3] = 1.0f;
		Matrix4_Multiply_Vector( rnp->cameraProjectionMatrix, verts[i], proj );

		v[0] = ( proj[0] / proj[3] + 1.0f ) * 0.5f * rnp->viewport[2];
		v[1] = ( proj[1] / proj[3] + 1.0f ) * 0.5f * rnp->viewport[3];
		v[2] = ( proj[2] / proj[3] + 1.0f ) * 0.5f; // [-1..1] -> [0..1]

		if( !i ) {
			x1 = x2 = v[0];
			y1 = y2 = v[1];
		} else {
			x1 = min( x1, v[0] ); y1 = min( y1, v[1] );
			x2 = max( x2, v[0] ); y2 = max( y2, v[1] );
		}
	}

	ix1 = max( x1 - 1.0f, 0 );
	ix2 = min( x2 + 1.0f, rnp->viewport[2] );

	iy1 = max( y1 - 1.0f, 0 );
	iy2 = min( y2 + 1.0f, rnp->viewport[3] );

	if( ix1 >= ix2 || iy1 >= iy2 ) {
		return true;
	}

	scissor[0] = ix1 + rnp->viewport[0];
	scissor[1] = rnp->viewport[3] - iy2 + rnp->viewport[1];
	scissor[2] = ix2 - ix1;
	scissor[3] = iy2 - iy1;

	if( scissor[0] < rnp->scissor[0] ) scissor[0] = rnp->scissor[0];
	if( scissor[1] < rnp->scissor[1] ) scissor[1] = rnp->scissor[1];
	if( scissor[2] > rnp->scissor[2] ) scissor[2] = rnp->scissor[2];
	if( scissor[3] > rnp->scissor[3] ) scissor[3] = rnp->scissor[3];

	return false;
}

/*
* R_ScissorForBBox
* 
* Returns true if the bbox is off-screen.
*/
bool R_ScissorForBBox( const refinst_t *rnp, vec3_t mins, vec3_t maxs, int *scissor ) {
	int i;
	vec3_t corner[8];

	if( BoundsOverlap( rnp->viewOrigin, rnp->viewOrigin, mins, maxs ) ) {
		for( i = 0; i < 4; i++ ) {
			scissor[i] = rnp->scissor[i];
		}
		return false;
	}

	for( i = 0; i < 8; i++ ) {
		corner[i][0] = ( ( i & 1 ) ? mins[0] : maxs[0] );
		corner[i][1] = ( ( i & 2 ) ? mins[1] : maxs[1] );
		corner[i][2] = ( ( i & 4 ) ? mins[2] : maxs[2] );
	}

	return R_ScissorForCorners( rnp, corner, scissor );
}

/*
* R_FogCull
*/
bool R_FogCull( const mfog_t *fog, vec3_t origin, float radius ) {
	// note that fog->distanceToEye < 0 is always true if
	// globalfog is not NULL and we're inside the world boundaries
	if( fog && fog->shader && fog == rn.fog_eye ) {
		float vpnDist = ( ( rn.viewOrigin[0] - origin[0] ) * rn.viewAxis[AXIS_FORWARD + 0] +
			( rn.viewOrigin[1] - origin[1] ) * rn.viewAxis[AXIS_FORWARD + 1] +
			( rn.viewOrigin[2] - origin[2] ) * rn.viewAxis[AXIS_FORWARD + 2] );
		return ( ( vpnDist + radius ) / fog->shader->fog_dist ) < -1;
	}

	return false;
}

/*
* R_CullBoxCustomPlanes
*
* Returns true if the box is completely outside the frustum
*/
static bool R_CullBoxCustomPlanes( const cplane_t *p, unsigned nump, const vec3_t mins, const vec3_t maxs, const unsigned int clipflags ) {
	unsigned int i, bit;

	if( r_nocull->integer ) {
		return false;
	}

	for( i = 0, bit = 1; i < nump; i++, bit <<= 1, p++ ) {
		if( !( clipflags & bit ) ) {
			continue;
		}

		switch( p->signbits & 7 ) {
		case 0:
			if( p->normal[0] * maxs[0] + p->normal[1] * maxs[1] + p->normal[2] * maxs[2] < p->dist ) {
				return true;
			}
			break;
		case 1:
			if( p->normal[0] * mins[0] + p->normal[1] * maxs[1] + p->normal[2] * maxs[2] < p->dist ) {
				return true;
			}
			break;
		case 2:
			if( p->normal[0] * maxs[0] + p->normal[1] * mins[1] + p->normal[2] * maxs[2] < p->dist ) {
				return true;
			}
			break;
		case 3:
			if( p->normal[0] * mins[0] + p->normal[1] * mins[1] + p->normal[2] * maxs[2] < p->dist ) {
				return true;
			}
			break;
		case 4:
			if( p->normal[0] * maxs[0] + p->normal[1] * maxs[1] + p->normal[2] * mins[2] < p->dist ) {
				return true;
			}
			break;
		case 5:
			if( p->normal[0] * mins[0] + p->normal[1] * maxs[1] + p->normal[2] * mins[2] < p->dist ) {
				return true;
			}
			break;
		case 6:
			if( p->normal[0] * maxs[0] + p->normal[1] * mins[1] + p->normal[2] * mins[2] < p->dist ) {
				return true;
			}
			break;
		case 7:
			if( p->normal[0] * mins[0] + p->normal[1] * mins[1] + p->normal[2] * mins[2] < p->dist ) {
				return true;
			}
			break;
		default:
			break;
		}
	}

	return false;

}

/*
* R_CullBox
*
* Returns true if the bounding box is completely outside the frustum
*/
bool R_CullBox( const vec3_t mins, const vec3_t maxs, const unsigned int clipFlags ) {
	return R_CullBoxCustomPlanes( rn.frustum, sizeof( rn.frustum ) / sizeof( rn.frustum[0] ), mins, maxs, clipFlags );
}

/*
* R_DeformedCullBox
*
* Returns true if the bounding box is completely outside the deformed frustum
*/
bool R_DeformedCullBox( const vec3_t mins, const vec3_t maxs ) {
	return R_CullBoxCustomPlanes( rn.deformedFrustum, rn.numDeformedFrustumPlanes, mins, maxs, (1<<rn.numDeformedFrustumPlanes)-1 );
}

/*
* R_CullSphereCustomPlanes
*
* Returns true if the sphere is completely outside the frustum
*/
static bool R_CullSphereCustomPlanes( const cplane_t *p, unsigned nump, const vec3_t centre, const float radius, const unsigned int clipflags ) {
	unsigned int i, bit;

	if( r_nocull->integer ) {
		return false;
	}

	for( i = 0, bit = 1; i < nump; i++, bit <<= 1, p++ ) {
		if( !( clipflags & bit ) ) {
			continue;
		}
		if( DotProduct( centre, p->normal ) - p->dist <= -radius ) {
			return true;
		}
	}

	return false;
}

/*
* R_CullSphere
*
* Returns true if the sphere is completely outside the frustum
*/
bool R_CullSphere( const vec3_t centre, const float radius, const unsigned int clipFlags ) {
	return R_CullSphereCustomPlanes( rn.frustum, sizeof( rn.frustum ) / sizeof( rn.frustum[0] ), centre, radius, clipFlags );
}

/*
* R_DeformedCullSphere
*
* Returns true if the sphere is completely outside the deformed frustum
*/
bool R_DeformedCullSphere( const vec3_t centre, const float radius ) {
	return R_CullSphereCustomPlanes( rn.deformedFrustum, rn.numDeformedFrustumPlanes, centre, radius, (1<<rn.numDeformedFrustumPlanes)-1 );
}

/*
* R_VisCullBox
*/
bool R_VisCullBox( const vec3_t mins, const vec3_t maxs ) {
	int s, stackdepth = 0;
	vec3_t extmins, extmaxs;
	mnode_t *node, *localstack[2048];

	if( !rsh.worldModel || ( rn.refdef.rdflags & RDF_NOWORLDMODEL ) ) {
		return false;
	}
	if( rn.renderFlags & RF_NOVIS ) {
		return false;
	}
	if( rn.viewcluster < 0 ) {
		return false;
	}

	for( s = 0; s < 3; s++ ) {
		extmins[s] = mins[s] - 4;
		extmaxs[s] = maxs[s] + 4;
	}

	for( node = rsh.worldBrushModel->nodes;; ) {
		if( !node->plane ) {
			unsigned j;
			mleaf_t *leaf = (mleaf_t *)node;

			for( j = 0; j < leaf->numVisSurfaces; j++ ) {
				if( rn.meshlist->worldSurfVis[leaf->visSurfaces[j]] )
					return false;
			}

			if( !stackdepth )
		        return true;
		    node = localstack[--stackdepth];
		    continue;
		}

		s = BOX_ON_PLANE_SIDE( extmins, extmaxs, node->plane ) - 1;
		if( s < 2 ) {
			node = node->children[s];
			continue;
		}

		// go down both sides
		if( stackdepth < sizeof( localstack ) / sizeof( mnode_t * ) ) {
			localstack[stackdepth++] = node->children[0];
		}
		node = node->children[1];
	}

	return true;
}

/*
* R_VisCullSphere
*/
bool R_VisCullSphere( const vec3_t origin, float radius ) {
	float dist;
	int stackdepth = 0;
	mnode_t *node, *localstack[2048];

	if( !rsh.worldModel || ( rn.refdef.rdflags & RDF_NOWORLDMODEL ) ) {
		return false;
	}
	if( rn.renderFlags & RF_NOVIS ) {
		return false;
	}
	if( rn.viewcluster < 0 ) {
		return false;
	}

	radius += 4;
	for( node = rsh.worldBrushModel->nodes;; ) {
		if( !node->plane ) {
			unsigned j;
			mleaf_t *leaf = (mleaf_t *)node;

			for( j = 0; j < leaf->numVisSurfaces; j++ ) {
				if( rn.meshlist->worldSurfVis[leaf->visSurfaces[j]] )
					return false;
			}

			if( !stackdepth )
				return true;
			node = localstack[--stackdepth];
			continue;
		}

		dist = PlaneDiff( origin, node->plane );
		if( dist > radius ) {
			node = node->children[0];
			continue;
		} else if( dist < -radius ) {
			node = node->children[1];
			continue;
		}

		// go down both sides
		if( stackdepth < sizeof( localstack ) / sizeof( mnode_t * ) ) {
			localstack[stackdepth++] = node->children[0];
		}
		node = node->children[1];
	}

	return true;
}

/*
* R_CullModelEntity
*/
int R_CullModelEntity( const entity_t *e, bool pvsCull ) {
	const entSceneCache_t *cache = R_ENTCACHE( e );
	const vec_t *mins = cache->absmins;
	const vec_t *maxs = cache->absmaxs;
	float radius = cache->radius;
	bool sphereCull = cache->rotated;

	if( cache->mod_type == mod_bad ) {
		return false;
	}

	// account for possible outlines
	if( e->outlineHeight ) {
		radius += e->outlineHeight * r_outlines_scale->value * 1.733 /*sqrt(3)*/;
	}

	if( sphereCull ) {
		if( R_CullSphere( e->origin, radius, rn.clipFlags ) ) {
			return 1;
		}
	} else {
		if( R_CullBox( mins, maxs, rn.clipFlags ) ) {
			return 1;
		}
	}

	if( pvsCull ) {
		if( sphereCull ) {
			if( R_VisCullSphere( e->origin, radius ) ) {
				return 2;
			}
		} else {
			if( R_VisCullBox( mins, maxs ) ) {
				return 2;
			}
		}
	}

	return 0;
}

/*
* R_CullSpriteEntity
*/
int R_CullSpriteEntity( const entity_t *e ) {
	if( rn.renderFlags & RF_LIGHTVIEW ) {
		if( !R_ShaderNoDlight( e->customShader ) ) {
			return 1;
		}
	}
	if( rn.renderFlags & RF_SHADOWMAPVIEW ) {
		if( !R_ShaderNoShadow( e->customShader ) ) {
			return 1;
		}
	}
	return 0;
}
