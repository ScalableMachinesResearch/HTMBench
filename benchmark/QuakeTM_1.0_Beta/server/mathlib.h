/*
Copyright (C) 1996-1997 Id Software, Inc.

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

/*
QuakeTM
Transactional implementation of the original Quake game
Copyright (C) 2008 Barcelona Supercomputing Center
Author: Vladimir Gajinov
*/

// mathlib.h


#ifndef PR_EDICT
typedef float vec_t;
typedef vec_t vec3_t[3];
#endif
typedef vec_t vec5_t[5];

typedef	int	fixed4_t;
typedef	int	fixed8_t;
typedef	int	fixed16_t;

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

struct mplane_s;

extern vec3_t vec3_origin;
extern	int nanmask;

#define	IS_NAN(x) (((*(int *)&x)&nanmask)==nanmask)

#define DotProduct(x,y) (x[0]*y[0]+x[1]*y[1]+x[2]*y[2])
#define VectorSubtract(a,b,c) {c[0]=a[0]-b[0];c[1]=a[1]-b[1];c[2]=a[2]-b[2];}
#define VectorAdd(a,b,c) {c[0]=a[0]+b[0];c[1]=a[1]+b[1];c[2]=a[2]+b[2];}
#define VectorCopy(a,b) {b[0]=a[0];b[1]=a[1];b[2]=a[2];}

TM_CALLABLE
void VectorMA (vec3_t veca, float scale, vec3_t vecb, vec3_t vecc);

TM_CALLABLE
vec_t Length (vec3_t v);
TM_CALLABLE
void CrossProduct (vec3_t v1, vec3_t v2, vec3_t cross);
TM_CALLABLE
float VectorNormalize (vec3_t v);		// returns vector length
TM_CALLABLE
void VectorScale (vec3_t in, vec_t scale, vec3_t out);

TM_CALLABLE
void AngleVectors (vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
TM_CALLABLE
int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, struct mplane_s *plane);
TM_PURE
float	anglemod(float a);


#define BOX_ON_PLANE_SIDE(emins, emaxs, p)	\
	(((p)->type < 3)?						\
	(										\
		((p)->dist <= (emins)[(p)->type])?	\
			1								\
		:									\
		(									\
			((p)->dist >= (emaxs)[(p)->type])?\
				2							\
			:								\
				3							\
		)									\
	)										\
	:										\
		BoxOnPlaneSide( (emins), (emaxs), (p)))

