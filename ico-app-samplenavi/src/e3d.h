/*
 * Copyright (c) 2013, TOYOTA MOTOR CORPORATION.
 *
 * This program is licensed under the terms and conditions of the 
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */
/**
 * @brief   header file oflibrary which displays 3D field of view
 *
 * @date    Apr-25-2013
 */

#ifndef SAMPLENAVI_E3D_H_
#define SAMPLENAVI_E3D_H_

#include <Ecore_Evas.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "define.h"
#include "samplenavi.h"

/* Functions */
void init_e3d(Evas *e3d, void *p_in, size_t len);
void e3d_cleanup();
void draw_route(Evas *e3d);
void draw_landmark(Evas *e3d);
void calc_camera_coord();

#ifdef _USE_Z_LIMIT_FIX_
enum Check_Route
{
    NONE,
    STARTING_POINT,        /* FIXED Z_LIMIT START POINT */
    ENDING_POINT,          /* FIXED Z_LIMIT END POINT */
    BOTH_POINT             /* FIXED Z_LIMIT START/END POINT */
};
#endif

/* Structs */
typedef struct _CsvRoute
{
    double lon;
    double lat;
} CsvRoute;

typedef struct _CsvLandmark
{
    double lon;
    double lat;
    char id;
} CsvLandmark;

typedef struct _Point
{
    double x, y, z, u, v;
#ifdef _USE_Z_LIMIT_FIX_
    enum Check_Route status;
    char enable_fix;
#endif
} Point;

typedef struct _Plane
{
    Evas_Object *o;
    Point pt[4];
    int drawable;
} Plane;

typedef struct _Cube
{
    Plane side[6];
    double angle;
} Cube;

/* Camera Positions */
typedef struct _Camera
{
    Evas_Coord x, y, z;
    double angle;
} Camera;

typedef struct _CameraGeocode
{
    double lat, lon;
    int dir;
} CameraGeocode;

typedef struct _MapPos
{
    double lat;
    double lon;
    int dir;
} MapPos;

/* Extern */
extern Camera camera;
extern CameraGeocode camera_geocode;
extern MapPos map_pos;
extern char set_route;
extern char enable_navi;
extern double goal_square_length;

#ifdef _SHOW_FPS_
extern int polygon_count;
#endif

/* Macros */
#define POINT(point, xx, yy, zz, uu, vv) \
    point.x = xx; \
    point.y = yy; \
    point.z = zz; \
    point.u = uu; \
    point.v = vv

#define PLANE_POINT(plane, i, xx, yy, zz, uu, vv) \
    plane->pt[i].x = xx; \
    plane->pt[i].y = yy; \
    plane->pt[i].z = zz; \
    plane->pt[i].u = uu; \
    plane->pt[i].v = vv

#define SIDE_POINT(p, xx, yy, zz, uu, vv) \
    plane->pt[p].x = xx; \
    plane->pt[p].y = yy; \
    plane->pt[p].z = zz; \
    plane->pt[p].u = uu; \
    plane->pt[p].v = vv

#define CUBE_POINT(n, p, xx, yy, zz, uu, vv) \
    c->side[n].pt[p].x = xx; \
    c->side[n].pt[p].y = yy; \
    c->side[n].pt[p].z = zz; \
    c->side[n].pt[p].u = uu; \
    c->side[n].pt[p].v = vv


#endif /* SAMPLENAVI_E3D_H_ */
