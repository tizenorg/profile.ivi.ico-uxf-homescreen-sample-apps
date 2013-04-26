/*
 * Copyright (c) 2013, TOYOTA MOTOR CORPORATION.
 *
 * This program is licensed under the terms and conditions of the 
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */
/**
 * @brief   library which displays 3D field of view
 *
 * @date    Apr-25-2013
 */

#include <stdlib.h>
#include <string.h>
#include "samplenavi.h"
#include "e3d.h"
#include "ico_apf_log.h"

/*============================================================================*/
/* Define fixed parameters                                                    */
/*============================================================================*/
/* Get route from websocket */
#define ROUTE_START_TAG "<route>"
#define ROUTE_END_TAG   "</route>"
#define BR_TAG          "<BR>"

/*============================================================================*/
/* Define data types                                                          */
/*============================================================================*/
/* Landmark Points and Cubes */
typedef struct _PointLength {
    int index;
    double length;
    int drawable;
} PointLength;

/*============================================================================*/
/* Function prototype for static(internal) functions                          */
/*============================================================================*/
static void load_route_from_websocket(char *p_in, size_t len);

/* Local Functions for Route. */
static void calc_route_coord_from_lonlat();

/* Local Functions for Landmark. */
static void load_landmarks_from_csv();
static void calc_landmarks_coord_from_lonlat();

void init_e3d(Evas *e3d, void *p_in, size_t len);

/* Landmark Points and Cubes */
static const char *landmark_img_path(int i);

void e3d_cleanup();
void calc_route_distance_to_camera();
void check_route_drawable_by_distance_to_camera();
double calc_square_length(Point p0, Point p1);
int length_comp(const void *pl0, const void *pl1);

/* Local Functions for Drawing on Evas */
Plane *_plane_new(Evas *e3d, Evas_Coord w, Evas_Coord d, Evas_Coord h);
void _plane_draw(Plane *plane);
void _plane_free(Plane *plane);
Cube *_cube_new(Evas *e3d, Evas_Coord w, Evas_Coord h, Evas_Coord d, const char *img_path);
void _cube_draw(Cube *c, Evas_Coord x, Evas_Coord y, Evas_Coord z, double dx, double dy, double dz);
void _cube_free(Cube *c);
void goal_object_new(Evas *e3d);
void goal_object_draw();
void goal_object_free();

/* Local Functions for Route. */
void calc_route_coord_by_camera();
void check_route_darawable_by_z_limit_fix();
void draw_route(Evas *e3d);
double calc_length(Point p0, Point p1);
void calc_route_distance();
void rotate_xz(Point src, Point *dist, double angle);
void calc_base_box_points(int routenum);
void calc_base_box_points_all();
void calc_intersection(Point p0, Point p1, Point p2, Point p3, Point *point);
void calc_bezier_points(Point p0, Point p1, Point p2, Point *points);
void draw_curve(int routenum);
Point calc_fix_point(int routenum, int point_index);
void calc_fixed_box_points(int routenum);
void calc_fixed_box_points_all();

/* Local Functions for Landmark. */
void calc_landmarks_coord_by_camera();
void draw_landmark(Evas *e3d);

/*============================================================================*/
/* Tables and Valiables                                                       */
/*============================================================================*/
Camera camera;
CameraGeocode camera_geocode;
MapPos map_pos;
char set_route;
char enable_navi;
#ifdef _SHOW_FPS_
int polygon_count;
#endif

/* Route Points and Planes*/
int route_data_count;
CsvRoute csv_route[MAX_ROUTE_POINTS];
Point route_raw[MAX_DRAW_ROUTE_POINTS];
Point route[MAX_DRAW_ROUTE_POINTS];
double route_square_length_camera[MAX_DRAW_ROUTE_POINTS];
#ifdef _USE_Z_LIMIT_FIX_
Point route_pre_fix[MAX_DRAW_ROUTE_POINTS];
#endif
Plane *base_plane[MAX_DRAW_ROUTE_POINTS - 1];
Plane *fixed_plane[MAX_DRAW_ROUTE_POINTS - 1];
Plane *division_plane[MAX_DRAW_ROUTE_POINTS - 2][ROUTE_DIVISION_NUMBER];
int route_draw_num;
double route_distance[MAX_DRAW_ROUTE_POINTS - 1];
Evas_Object *goal_object;

/* Landmark Points and Cubes */
int landmark_data_count;
CsvLandmark csv_landmark[MAX_LANDMARK_POINTS];
Point landmark_raw[MAX_LANDMARK_POINTS];
Point landmark[MAX_LANDMARK_POINTS];
Cube *landmark_cube[MAX_LANDMARK_POINTS];

/*============================================================================*/
/* functions                                                                  */
/*============================================================================*/
/*--------------------------------------------------------------------------*/
/*
 * @brief   load_route_from_websocket
 *          route information acquisition.
 *
 * @param[in]   p_in              receive message
 * @param[in]   len               message size[BYTE]
 * @return      none
 */
/*--------------------------------------------------------------------------*/
static void
load_route_from_websocket(char *p_in, size_t len)
{
    /* Local variable */
    size_t  index = 0;
    char    str[256];
    route_data_count = 0;
    
    /* Get string */
    while( index < len ) {
        /* Get start tag */
        if( ( strlen( &p_in[index] ) >= strlen( ROUTE_START_TAG ) ) && 
            ( strncmp( &p_in[index], ROUTE_START_TAG, strlen( ROUTE_START_TAG ) ) == 0 ) ) 
        {
            index += strlen( ROUTE_START_TAG );
        }
        /* Get end tag */
        else if( ( strlen( &p_in[index] ) >= strlen( ROUTE_END_TAG ) ) && 
                 ( strncmp( &p_in[index], ROUTE_START_TAG, strlen( ROUTE_END_TAG ) ) == 0 ) ) 
        {
            break;
        }
        /* Get br tag */
        else if( ( strlen( &p_in[index] ) >= strlen( BR_TAG ) ) && 
                 ( strncmp( &p_in[index], BR_TAG, strlen( BR_TAG ) ) == 0 ) ) 
        {
            index += strlen( BR_TAG );
        }
        /* Get parameter */
        else {
            /* Get lng */
            memset( str, 0, sizeof( str ) );
            while( ( p_in[index] != ',' ) && ( index < len ) ) {
                str[strlen( str )] = p_in[index];
                index++;
            }
            csv_route[route_data_count].lon = atof( str );
            index++;
            
            /* Get lat */
            memset( str, 0, sizeof( str ) );
            while( ( p_in[index] != '<' ) && ( index < len ) ) {
                str[strlen( str )] = p_in[index];
                index++;
            }
            csv_route[route_data_count].lat = atof( str );
            route_data_count++;
        }
    }
    if(route_data_count > 0){
        route_data_count--;
    }
}

/*--------------------------------------------------------------------------*/
/*
 * @brief   calc_route_coord_from_lonlat
 *          The coordinates of a route are calculated from 
 *          latitude and longitude.
 *
 * @param     none
 * @return    none
 */
/*--------------------------------------------------------------------------*/
static void
calc_route_coord_from_lonlat()
{
    int i;

    for (i = 0; i < route_data_count; i++) {
        double x = (csv_route[i].lon - csv_route[0].lon) * LON_CONVERT * COORD_CONVERT_COEFFICIENT;
        double z = (csv_route[i].lat - csv_route[0].lat) * LAT_CONVERT * COORD_CONVERT_COEFFICIENT;
        POINT(route_raw[i], x, ROUTE_PLANE_HEIGHT, z, 0, 0);
    }
}

/*--------------------------------------------------------------------------*/
/*
 * @brief   load_landmarks_from_csv
 *          The information on a landmark is loaded from a CSV file.
 *
 * @param     none
 * @return    none
 */
/*--------------------------------------------------------------------------*/
static void
load_landmarks_from_csv()
{
    FILE *fp;
    int i;

    if ((fp = fopen(csv_landmark_path, "r")) == NULL) {
        fprintf(stderr, "%s\n", "Error : can't open file.(landmark.csv)\n");
        landmark_data_count = 0;
        return;
    }

    i = 0;
    while(fscanf(fp, "%lf,%lf,%hhd",
                &csv_landmark[i].lon, &csv_landmark[i].lat, &csv_landmark[i].id) != EOF) {
        i++;
    }
    landmark_data_count = i;

    fclose(fp);

    // for test
    for (i = 0; i < landmark_data_count; i++) {
        uim_debug("landmark data %d : %.4f, %.4f, %d\n",
                i, csv_landmark[i].lon, csv_landmark[i].lat, csv_landmark[i].id);
    }

    uim_debug("landmark data read. count = %d\n", landmark_data_count);

    return;
}

/*--------------------------------------------------------------------------*/
/*
 * @brief   calc_landmarks_coord_from_lonlat
 *          The coordinates of a landmark are calculated from
 *          latitude and longitude.
 *
 * @param     none
 * @return    none
 */
/*--------------------------------------------------------------------------*/
static void calc_landmarks_coord_from_lonlat()
{
    int i;

    for (i = 0; i < landmark_data_count; i++) {
        double x = (csv_landmark[i].lon - csv_route[0].lon) * LON_CONVERT * COORD_CONVERT_COEFFICIENT;
        double z = (csv_landmark[i].lat - csv_route[0].lat) * LAT_CONVERT * COORD_CONVERT_COEFFICIENT;
        POINT(landmark_raw[i], x, conf_data[LANDMARK_POSITION], z, 0, 0);
    }
}

/*--------------------------------------------------------------------------*/
/*
 * @brief   load_route_from_websocket
 *          route information acquisition.
 *
 * @param[in]   e3d               3D view Evas object
 * @param[in]   p_in              receive message
 * @param[in]   len               message size[BYTE]
 * @return      none
 */
/*--------------------------------------------------------------------------*/
void
init_e3d( Evas *e3d, void *p_in, size_t len )
{
    int i, j;

    goal_square_length = 1000000000000000.0;

    /***********************
     * Route Initialize *
     ***********************/

    /* load route data */
    load_route_from_websocket( (char *)p_in, len );

    calc_route_coord_from_lonlat();
    route_draw_num = route_data_count;

    for (i = 0; i < route_draw_num - 1; i++) {
        base_plane[i] = _plane_new(e3d, 100, 400, 170);
        fixed_plane[i] = _plane_new(e3d, 100, 400, 170);
    }
    for (i = 0; i < route_draw_num - 2; i++) {
        for (j = 0; j < ROUTE_DIVISION_NUMBER; j++) {
            division_plane[i][j] = _plane_new(e3d, 10, 10, 10);
        }
    }

    goal_object_new(e3d);

#ifndef _USE_Z_LIMIT_FIX_
    calc_route_distance();
#endif

    calc_route_coord_by_camera();
    calc_base_box_points_all();
    calc_fixed_box_points_all();

#ifdef _E3D_H_OUTPUT_PRINT_
    uim_debug("Base Plane point0 = %f, %f, %f \n", base_plane[0]->pt[0].x,
      base_plane[0]->pt[0].y, base_plane[0]->pt[0].z);
    uim_debug("Base Plane point1 = %f, %f, %f \n", base_plane[0]->pt[1].x,
      base_plane[0]->pt[1].y, base_plane[0]->pt[1].z);
    uim_debug("Base Plane point2 = %f, %f, %f \n", base_plane[0]->pt[2].x,
      base_plane[0]->pt[2].y, base_plane[0]->pt[2].z);
    uim_debug("Base Plane point3 = %f, %f, %f \n", base_plane[0]->pt[3].x,
      base_plane[0]->pt[3].y, base_plane[0]->pt[3].z);
#endif

    /***********************
     * Landmark Initialize *
     ***********************/

    /* load landmark data */
    load_landmarks_from_csv();
    calc_landmarks_coord_from_lonlat();

    for (i = 0; i < landmark_data_count; i++) {
        landmark_cube[i] = _cube_new(e3d, LANDMARK_WIDTH, (LANDMARK_WIDTH / 3) * 4, LANDMARK_WIDTH,
                landmark_img_path(csv_landmark[i].id));
    }

/*****************************************************************************/

    // for test
    uim_debug("Route CSV x = %f, z = %f\n", csv_route[1].lon, csv_route[1].lat);
    uim_debug("Route x = %f, z = %f\n", route_raw[1].x, route_raw[1].z);
    uim_debug("Landmark CSV x = %f, z = %f\n", csv_landmark[0].lon, csv_landmark[0].lat);
    uim_debug("Landmark x = %f, z = %f\n", landmark_raw[0].x, landmark_raw[0].z);

    /***********************
     * Camera Initialize *
     ***********************/
    _camera_pos(0, 0, 0, 0);
    camera_geocode.lon = csv_route[0].lon;
    camera_geocode.lat = csv_route[0].lat;
    camera_geocode.dir = 0;
    calc_camera_coord();

    /***********************
     * Map present location Initialize *
     ***********************/
    map_pos.lon = csv_route[0].lon;
    map_pos.lat = csv_route[0].lat;

    /* draw e3d */
    draw_route(e3d);
    draw_landmark(e3d);
}

/*--------------------------------------------------------------------------*/
/*
 * @brief   landmark_img_path
 *          Image file path acquisition of a landmark.
 *
 * @param[in]   i     landmark image file path identifier 
 * @return      file path address
 * @retval       > 0      success
 * @retval       NULL     error
 */
/*--------------------------------------------------------------------------*/
static const char *
landmark_img_path(int i)
{
    const char *img_path;
    switch (i) {
        case 0:
            img_path = landmark_img_path_0;
            break;
        case 1:
            img_path = landmark_img_path_1;
            break;
        case 2:
            img_path = landmark_img_path_2;
            break;
        case 3:
            img_path = landmark_img_path_3;
            break;
        case 4:
            img_path = landmark_img_path_4;
            break;
        case 5:
            img_path = landmark_img_path_5;
            break;
        case 6:
            img_path = landmark_img_path_6;
            break;
        case 7:
            img_path = landmark_img_path_7;
            break;
        case 8:
            img_path = landmark_img_path_8;
            break;
        case 9:
            img_path = landmark_img_path_9;
            break;
        case 10:
            img_path = landmark_img_path_10;
            break;
        case 11:
            img_path = landmark_img_path_11;
            break;
        case 12:
            img_path = landmark_img_path_12;
            break;
        case 13:
            img_path = landmark_img_path_13;
            break;
        case 14:
            img_path = landmark_img_path_14;
            break;
        case 15:
            img_path = landmark_img_path_15;
            break;
    }
    return img_path;
}

/*--------------------------------------------------------------------------*/
/*
 * @brief   e3d_cleanup
 *          The information on a landmark is loaded from a CSV file.
 *
 * @param     none
 * @return    none
 */
/*--------------------------------------------------------------------------*/
void
e3d_cleanup()
{
    int i, j;

    for (i = 0; i < route_draw_num - 1; i++) {
        _plane_free(base_plane[i]);
        _plane_free(fixed_plane[i]);
    }

    for (i = 0; i < route_draw_num - 2; i++) {
        for (j = 0; j < ROUTE_DIVISION_NUMBER; j++) {
            _plane_free(division_plane[i][j]);
        }
    }

    for (i = 0; i < landmark_data_count; i++) {
        _cube_free(landmark_cube[i]);
    }

    goal_object_free();
}

void draw_route(Evas *e3d) {
    int i;
    int j;

#ifdef _SHOW_FPS_
    polygon_count = 0;
#endif

    calc_route_coord_by_camera();
    calc_base_box_points_all();
    calc_fixed_box_points_all();

// ----------------------------------------------------------------------------

    if (enable_navi == FALSE || set_route == FALSE) {
        evas_object_hide(goal_object);
        return;
    }

    for (i = 0; i < route_draw_num - 1; i++) {
        if (fixed_plane[i]->drawable == TRUE) {
            _plane_draw(fixed_plane[i]);
        } else {
            evas_object_hide(fixed_plane[i]->o);
        }
    }

    /* curve draw */
    for (i = 0; i < route_draw_num - 2; i++) {
        if (fixed_plane[i]->drawable == TRUE) {
            draw_curve(i);
        } else {
            for (j = 0; j < ROUTE_DIVISION_NUMBER; j++) {
                evas_object_hide(division_plane[i][j]->o);
            }
        }
#ifdef _E3D_H_OUTPUT_PRINT_
        uim_debug("curve draw. \n");
#endif
    }

#ifdef _E3D_H_OUTPUT_PRINT_
    uim_debug("route draw comp.\n");
#endif

    goal_object_draw(fixed_plane[route_draw_num - 2]);

}

void draw_landmark(Evas *e3d) {
    static unsigned long frame_count = 0;
    PointLength draw_order[landmark_data_count];
    int i, j, k;
    Point camera_coord;
    int landmark_angle;
    double culling_length;

    if (enable_navi == FALSE) {
        for (i = 0; i < landmark_data_count; i++) {
            for (j = 0; j < 6; j++) {
                evas_object_hide(landmark_cube[i]->side[j].o);
            }
        }
        return;
    }
    calc_landmarks_coord_by_camera();

    camera_coord.x = camera.x;
    camera_coord.z = camera.z;

    /* calc landmark rotation */
    landmark_angle = (int)((conf_data[LANDMARK_ROTATION] / (1.0 / TIME_INTERVAL_AR)) * frame_count) % 360;

    culling_length = CULLING_LENGTH_LANDMARKS * CULLING_LENGTH_LANDMARKS;

    /* In order to draw from distant landmark. */
    for (i = 0; i < landmark_data_count; i++) {
        draw_order[i].index = i;
        draw_order[i].length = calc_square_length(landmark[i], camera_coord);

        /* for culling */
        if (culling_length < draw_order[i].length) {
            draw_order[i].drawable = FALSE;
        } else {
            draw_order[i].drawable = TRUE;
        }
    }

    qsort(draw_order, landmark_data_count, sizeof(PointLength), length_comp);

    for (i = 0; i < landmark_data_count; i++) {
        j = draw_order[i].index;
        if (draw_order[i].drawable == TRUE) {
            _cube_draw(landmark_cube[j], landmark[j].x, landmark[j].y, landmark[j].z, 
                    0, landmark_angle, 0);
        } else {
            for (k = 0; k < 6; k++) {
                evas_object_hide(landmark_cube[j]->side[k].o);
            }
        }
    }

    frame_count++;
}

void
_camera_pos(Evas_Coord x, Evas_Coord y, Evas_Coord z, double angle)
{
    // uim_debug("camera_pos x:%d y:%d z:%d angle:%d", x, y, z, angle);
    camera.x = x;
    camera.y = y;
    camera.z = z;
    camera.angle = angle;
}

void _camera_move(double speed, double angle) {
    double x, y, z;
    double rad;

    rad = (-angle) * M_PI / 180.0;

    x = camera.x + speed * sin(rad);
    y = camera.y;
    z = camera.z + speed * cos(rad);

    _camera_pos(x, y, z, angle);
}

void calc_camera_coord() {
    double x, z;

    x = (camera_geocode.lon - csv_route[0].lon) * LON_CONVERT * COORD_CONVERT_COEFFICIENT;
    z = (camera_geocode.lat - csv_route[0].lat) * LAT_CONVERT * COORD_CONVERT_COEFFICIENT;

    _camera_pos(x, camera.y, z, (360.0 - (double)camera_geocode.dir));

}

/******************************************************************************
 * Functions for Drawing on Evas
 ******************************************************************************/
Plane* _plane_new(Evas *e3d, Evas_Coord w, Evas_Coord d, Evas_Coord h)
{
    Plane *plane;
    Evas_Object *o;

    plane = calloc(1, sizeof(Plane));

    /* origin point */
    w -= (w / 2);

    o = evas_object_rectangle_add(e3d);
    plane->o = o;

    evas_object_resize(o, 256, 256);
    evas_object_hide(o);

    SIDE_POINT(0, -w, -h,  d,   0,   0);
    SIDE_POINT(1,  w, -h,  d, 256,   0);
    SIDE_POINT(2,  w, -h, -d, 256, 256);
    SIDE_POINT(3, -w, -h, -d,   0, 256);

    evas_object_layer_set(o, LAYER_ROUTE);

    plane->drawable = TRUE;

    return plane;
}

void _plane_draw(Plane *plane) {
    static Evas_Map *m2 = NULL;
    int i;

    Evas_Coord fix_z[4];

    if (!m2) m2 = evas_map_new(4);
    evas_map_smooth_set(m2, 0);

#ifdef _USE_Z_LIMIT_FIX_
    if(plane->drawable == FALSE) {
        evas_object_hide(plane->o);
        return;
    }
#endif

    evas_object_color_set(plane->o, 51, 178, 0, 179);

    /* set points */
    evas_map_point_coord_set(m2, 0, (Evas_Coord)(plane->pt[0].x + ((W_WIDTH) / 2)),
            (Evas_Coord)(plane->pt[0].y + ((W_NAVI_HEIGHT) / 2)), (Evas_Coord)(plane->pt[0].z + FRONT_SIDE_Z));
    evas_map_point_coord_set(m2, 1, (Evas_Coord)(plane->pt[1].x + ((W_WIDTH) / 2)),
            (Evas_Coord)(plane->pt[1].y + ((W_NAVI_HEIGHT) / 2)), (Evas_Coord)(plane->pt[1].z + FRONT_SIDE_Z));
    evas_map_point_coord_set(m2, 2, (Evas_Coord)(plane->pt[2].x + ((W_WIDTH) / 2)),
            (Evas_Coord)(plane->pt[2].y + ((W_NAVI_HEIGHT) / 2)), (Evas_Coord)(plane->pt[2].z + FRONT_SIDE_Z));
    evas_map_point_coord_set(m2, 3, (Evas_Coord)(plane->pt[3].x + ((W_WIDTH) / 2)),
            (Evas_Coord)(plane->pt[3].y + ((W_NAVI_HEIGHT) / 2)), (Evas_Coord)(plane->pt[3].z + FRONT_SIDE_Z));

    for (i = 0; i < 4; i++) {
        evas_map_point_image_uv_set(m2, i, plane->pt[i].u, plane->pt[i].v);
        evas_map_point_color_set(m2, i, 255, 255, 255, 255);
    }
    evas_map_util_3d_perspective(m2, (W_WIDTH / 2), (W_NAVI_HEIGHT / 2) - 100,
            Z0_POSITION, FOCUS_LENGTH);

    evas_object_map_enable_set(plane->o, 1);
    evas_object_map_set(plane->o, m2);

    evas_object_show(plane->o);

#ifdef _SHOW_FPS_
    polygon_count++;
#endif

}

void _plane_free(Plane *plane) {
    evas_object_del(plane->o);
    free(plane);
}

Cube *_cube_new(Evas *e3d, Evas_Coord w, Evas_Coord h, Evas_Coord d, const char *img_path) {
    Cube *c;
    int i;

    w -= (w / 2);
    h -= (h / 2);
    d -= (d / 2);
    c = calloc(1, sizeof(Cube));
    for (i = 0; i < 6; i++)
    {
        Evas_Object *o;
        switch (i) {
            case 0:
            case 1:
            case 2:
            case 3:
                o = evas_object_image_add(e3d);
                c->side[i].o = o;
                evas_object_image_file_set(o, img_path, NULL);
                evas_object_image_fill_set(o, 0, 0, 256, 256);
                evas_object_resize(o, 256, 256);
                evas_object_image_smooth_scale_set(o, 0);
                evas_object_show(o);
                evas_object_layer_set(o, LAYER_LANDMARK);
                break;
            case 4:
            case 5:
                o = evas_object_rectangle_add(e3d);
                c->side[i].o = o;
                evas_object_color_set(o, 0, 34, 119, 255);
                evas_object_resize(o, 256, 256);
                evas_object_show(o);
                evas_object_layer_set(o, LAYER_LANDMARK);
                break;
        }
    }
    // First Plane (Front)
    CUBE_POINT(0, 0, -w, -h, -d,   0,   0);
    CUBE_POINT(0, 1,  w, -h, -d, 600,   0);
    CUBE_POINT(0, 2,  w,  h, -d, 600, 800);
    CUBE_POINT(0, 3, -w,  h, -d,   0, 800);

    // Second Plane (Right side)
    CUBE_POINT(1, 0,  w, -h, -d,   0,   0);
    CUBE_POINT(1, 1,  w, -h,  d, 600,   0);
    CUBE_POINT(1, 2,  w,  h,  d, 600, 800);
    CUBE_POINT(1, 3,  w,  h, -d,   0, 800);
    
    // Third Plane (Back)
    CUBE_POINT(2, 0,  w, -h,  d,   0,   0);
    CUBE_POINT(2, 1, -w, -h,  d, 600,   0);
    CUBE_POINT(2, 2, -w,  h,  d, 600, 800);
    CUBE_POINT(2, 3,  w,  h,  d,   0, 800);

    // Fourth Plane (Left side)
    CUBE_POINT(3, 0, -w, -h,  d,   0,   0);
    CUBE_POINT(3, 1, -w, -h, -d, 600,   0);
    CUBE_POINT(3, 2, -w,  h, -d, 600, 800);
    CUBE_POINT(3, 3, -w,  h,  d,   0, 800);
    
    // Fifth Plane (Top side)
    CUBE_POINT(4, 0, -w, -h,  d,   0,   0);
    CUBE_POINT(4, 1,  w, -h,  d, 256,   0);
    CUBE_POINT(4, 2,  w, -h, -d, 256, 256);
    CUBE_POINT(4, 3, -w, -h, -d,   0, 256);
    
    // Sixth Plane (Under side)
    CUBE_POINT(5, 0, -w,  h, -d,   0,   0);
    CUBE_POINT(5, 1,  w,  h, -d, 256,   0);
    CUBE_POINT(5, 2,  w,  h,  d, 256, 256);
    CUBE_POINT(5, 3, -w,  h,  d,   0, 256);
    
    return c;
}

void _cube_draw(Cube *c, Evas_Coord x, Evas_Coord y, Evas_Coord z,
        double dx, double dy, double dz)
{
    static Evas_Map *m = NULL;
    int i, j, k, order[6], sorted, drawable, tmp_z;
    Evas_Coord mz[6];

    drawable = TRUE;

	// only 4 point
    if (!m) m = evas_map_new(4);
    // smooth rendering disable
    evas_map_smooth_set(m, 0);

    for (i = 0; i < 6; i++)
    {
        Evas_Coord tz[4];
        
        for (j = 0; j < 4; j++)
        {

            evas_map_point_coord_set(m, j,
                    c->side[i].pt[j].x + ((W_WIDTH) / 2) + x - camera.x, 
                    c->side[i].pt[j].y + ((W_NAVI_HEIGHT) / 2) + y - camera.y, 
                    c->side[i].pt[j].z + z - camera.z + FRONT_SIDE_Z);
            if ( !(i == 4 || i == 5)) {
                evas_map_point_image_uv_set(m, j,
                        c->side[i].pt[j].u,
                        c->side[i].pt[j].v);
            }
            evas_map_point_color_set(m, j, 255, 255, 255, 255);
             }
        evas_map_util_3d_rotate(m, dx, dy, dz,
            (W_WIDTH / 2) + x - camera.x, (W_NAVI_HEIGHT / 2) + y - camera.y, z - camera.z + FRONT_SIDE_Z);

        // for camera angle
        evas_map_util_3d_rotate(m, 0, -camera.angle, 0,
                (W_WIDTH / 2), (W_NAVI_HEIGHT / 2), FRONT_SIDE_Z);

        evas_map_util_3d_perspective(m, (W_WIDTH / 2), (W_NAVI_HEIGHT / 2) , Z0_POSITION, FOCUS_LENGTH);

#ifdef _USE_Z_LIMIT_FIX_
        for (j = 0; j < 4; j++) {
            evas_map_point_coord_get(m, j, NULL, NULL, &tmp_z);
            if (tmp_z < (Z_LIMIT + FRONT_SIDE_Z)) {
                drawable = FALSE;
            }
        }
#endif
        if (evas_map_util_clockwise_get(m))
        {
            evas_object_map_enable_set(c->side[i].o, 1);
            evas_object_map_set(c->side[i].o, m);
#ifdef _USE_Z_LIMIT_FIX_
            if (drawable == TRUE) {
#endif
                evas_object_show(c->side[i].o);
#ifdef _SHOW_FPS_
                polygon_count++;
#endif
#ifdef _USE_Z_LIMIT_FIX_
            } else {
                evas_object_hide(c->side[i].o);
            }
#endif
        }
        else {
            evas_object_hide(c->side[i].o);
        }
        
        order[i] = i;

        for (j = 0; j < 4; j++) {
            evas_map_point_coord_get(m, j, NULL, NULL, &(tz[j]));
        }

        mz[i] = (tz[0] + tz[1] + tz[2] + tz[3]) / 4;

        if (mz[i] < -512) {
            evas_object_hide(c->side[i].o);
        }
    }

    // sort by z
    sorted = 0;
    do
    {
        sorted = 1;
        for (i = 0; i < 5; i++)
        {
            if (mz[order[i]] > mz[order[i + 1]])
            {
                j = order[i];
                order[i] = order[i + 1];
                order[i + 1] = j;
                sorted = 0;
            }
        }
    }
    while (!sorted);

    evas_object_raise(c->side[order[0]].o);

    for (i = 1; i < 6; i++) {
        evas_object_stack_below(c->side[order[i]].o, c->side[order[i - 1]].o);
    }
}

void _cube_free(Cube *c)
{
    int i;
    
    for (i = 0; i < 6; i++) evas_object_del(c->side[i].o);
    free(c);
}

void goal_object_new(Evas *e3d) {
    goal_object = evas_object_image_add(e3d);
    evas_object_image_file_set(goal_object, goal_img_path, NULL);
    evas_object_image_fill_set(goal_object, 0, 0, 600, 600);

    evas_object_resize(goal_object, 256, 256);
    evas_object_image_smooth_scale_set(goal_object, 0);
    evas_object_hide(goal_object);
}

void goal_object_draw(Plane *plane) {
    static Evas_Map *m3 = NULL;
    int i;

    if (plane->drawable == FALSE) {
        evas_object_hide(goal_object);
        return;
    }

    if (!m3) m3 = evas_map_new(4);
    evas_map_smooth_set(m3, 0);

    evas_map_point_coord_set(m3, 0, (Evas_Coord)(plane->pt[0].x + ((W_WIDTH) / 2)),
            (Evas_Coord)(plane->pt[0].y + ((W_NAVI_HEIGHT) / 2)), (Evas_Coord)(plane->pt[0].z + FRONT_SIDE_Z));
    evas_map_point_coord_set(m3, 1, (Evas_Coord)(plane->pt[0].x + ((W_WIDTH) / 2)),
            (Evas_Coord)((-1) * plane->pt[0].y + (W_NAVI_HEIGHT) / 2), (Evas_Coord)(plane->pt[0].z + FRONT_SIDE_Z));
    evas_map_point_coord_set(m3, 2,(Evas_Coord)(plane->pt[3].x + ((W_WIDTH) / 2)),
            (Evas_Coord)((-1) * plane->pt[3].y + (W_NAVI_HEIGHT) / 2), (Evas_Coord)(plane->pt[3].z + FRONT_SIDE_Z));
    evas_map_point_coord_set(m3, 3, (Evas_Coord)(plane->pt[3].x + ((W_WIDTH) / 2)),
            (Evas_Coord)(plane->pt[3].y + ((W_NAVI_HEIGHT) / 2)), (Evas_Coord)(plane->pt[3].z + FRONT_SIDE_Z));

    evas_map_point_image_uv_set(m3, 0, 600,   0);
    evas_map_point_image_uv_set(m3, 1, 600, 600);
    evas_map_point_image_uv_set(m3, 2,   0, 600);
    evas_map_point_image_uv_set(m3, 3,   0,   0);

    evas_map_util_3d_perspective(m3, (W_WIDTH / 2), (W_NAVI_HEIGHT / 2) - 100,
            Z0_POSITION, FOCUS_LENGTH);

    evas_object_map_enable_set(goal_object, 1);
    evas_object_map_set(goal_object, m3);
    evas_object_show(goal_object);

    evas_object_raise(goal_object);
}

void goal_object_free() {
    evas_object_del(goal_object);
}


/******************************************************************************
 * Functions for Calculate Route Points
 ******************************************************************************/

#ifdef _USE_Z_LIMIT_FIX_
void check_route_darawable_by_z_limit_fix() {
    int i;
    Point limit_p0, limit_p1;

    POINT(limit_p0, -100, ROUTE_PLANE_HEIGHT, Z_LIMIT, 0, 0);
    POINT(limit_p1, 100, ROUTE_PLANE_HEIGHT, Z_LIMIT, 0, 0);

    for (i = 0; i < route_draw_num - 1; i++) {
        route[i].status = NONE;
        route[i].enable_fix = TRUE;

        if (route_pre_fix[i].z < Z_LIMIT) {
            route[i].status = STARTING_POINT;
            route[i].enable_fix = FALSE;
            if (route_pre_fix[i+1].z >= Z_LIMIT) {
                calc_intersection(limit_p0, limit_p1, route_pre_fix[i], route_pre_fix[i+1], &route[i]);
            } else {
                if (!(i > 0 && (route[i-1].status == ENDING_POINT))) {
                    route[i].status = BOTH_POINT;
                    route[i].x = route_pre_fix[i].x;
                    route[i].z = route_pre_fix[i].z;
                }
            }
        } else {
            // don't fix
            route[i].x = route_pre_fix[i].x;
            route[i].z = route_pre_fix[i].z;

            if (route_pre_fix[i].z < 0) {
                if (i > 0 && (fixed_plane[i - 1]->drawable == FALSE)) {
                    route[i].enable_fix = FALSE;
                }
            }
        }

        if (route_pre_fix[i+1].z < Z_LIMIT) {
            if ((route[i].status == STARTING_POINT) || (route[i].status == BOTH_POINT)) {
                route[i].status = BOTH_POINT;

                if ((i+1) == route_draw_num - 1) {
                    route[i+1].x = route_pre_fix[i+1].x;
                    route[i+1].z = route_pre_fix[i+1].z;
                } else {
                    if (route_pre_fix[i+2].z >= Z_LIMIT) {
                        calc_intersection(limit_p0, limit_p1, route_pre_fix[i+1], route_pre_fix[i+2], &route[i+1]);

                    } else {
                        route[i+1].x = route_pre_fix[i+1].x;
                        route[i+1].z = route_pre_fix[i+1].z;
                    }
                }

                if ((i < route_draw_num - 1) && (route_pre_fix[i+2].z > Z_LIMIT)) {
                    calc_intersection(limit_p0, limit_p1, route_pre_fix[i+1], route_pre_fix[i+2], &route[i+1]);
                } else {
                    route[i+1].x = route_pre_fix[i+1].x;
                    route[i+1].z = route_pre_fix[i+1].z;
                }
            } else {
                if (route_pre_fix[i].z >= Z_LIMIT) {
                    route[i].status = ENDING_POINT;
                    calc_intersection(limit_p0, limit_p1, route_pre_fix[i], route_pre_fix[i+1], &route[i+1]);
                } else {
                    route[i].status = BOTH_POINT;
                }
            }
        } else {
            route[i+1].x = route_pre_fix[i+1].x;
            route[i+1].z = route_pre_fix[i+1].z;
        }

        if (route[i].status == BOTH_POINT) {
            fixed_plane[i]->drawable = FALSE;
        } else {
            fixed_plane[i]->drawable = TRUE;
        }

    }
    

}
#endif

void calc_route_coord_by_camera() {
    int i;
    Point orizin;

    for (i = 0; i < route_draw_num; i++) {
#ifdef _USE_Z_LIMIT_FIX_
        route_pre_fix[i].x = route_raw[i].x - camera.x;
        route_pre_fix[i].z = route_raw[i].z - camera.z;
        rotate_xz(route_pre_fix[i], &route_pre_fix[i], camera.angle);

#else
        route[i].x = route_raw[i].x - camera.x;
        route[i].z = route_raw[i].z - camera.z;
        rotate_xz(route[i], &route[i], camera.angle);

#endif
    }

    orizin.x = 0;
    orizin.y = 0;
#ifdef _USE_Z_LIMIT_FIX_
    goal_square_length = calc_square_length(orizin, route_pre_fix[route_draw_num - 1]);
#else
    goal_square_length = calc_square_length(orizin, route[route_draw_num - 1]);
#endif

    for (i = 0; i < route_draw_num - 1; i++) {
        fixed_plane[i]->drawable = TRUE;
    }

#ifdef _USE_Z_LIMIT_FIX_
    check_route_darawable_by_z_limit_fix();
#endif

    calc_route_distance();
    calc_route_distance_to_camera();
    check_route_drawable_by_distance_to_camera();
}

void calc_route_distance_to_camera() {
    int i;
    Point origin;

    origin.x = 0;
    origin.z = 0;

    for ( i = 0; i < route_draw_num; i++) {
        route_square_length_camera[i] = calc_square_length(route[i], origin);
    }

}

void check_route_drawable_by_distance_to_camera() {
    int i, culling_route_count;
    double culling_length;

    culling_route_count = 0;
    culling_length = CULLING_LENGTH_ROUTE * CULLING_LENGTH_ROUTE;

    for ( i = 0; i < route_draw_num - 1; i++) {
        if ((route_square_length_camera[i] > culling_length) && 
                (route_square_length_camera[i+1] > culling_length)) {
            fixed_plane[i]->drawable = FALSE;
            culling_route_count++;
        }
    }

}

void calc_fixed_box_points_all() {
    int i;
    for (i = 0; i < route_draw_num - 1; i++) {
        calc_fixed_box_points(i);
    }
}

void calc_fixed_box_points(int routenum) {
#ifdef _USE_Z_LIMIT_FIX_
    if (routenum == 0 || route[routenum].enable_fix == FALSE) {
#else
    if (routenum == 0) {
#endif
        fixed_plane[routenum]->pt[1] = base_plane[routenum]->pt[1];
        fixed_plane[routenum]->pt[2] = base_plane[routenum]->pt[2];
    } else {
        fixed_plane[routenum]->pt[1] = calc_fix_point(routenum, 1);
        fixed_plane[routenum]->pt[2] = calc_fix_point(routenum, 2);
    }

#ifdef _USE_Z_LIMIT_FIX_
    if (routenum == route_draw_num - 2 || (route[routenum].status == ENDING_POINT || route[routenum].status == BOTH_POINT)) {
#else
    if (routenum == route_draw_num - 2) {
#endif
        fixed_plane[routenum]->pt[0] = base_plane[routenum]->pt[0];
        fixed_plane[routenum]->pt[3] = base_plane[routenum]->pt[3];
    } else {
        fixed_plane[routenum]->pt[0] = calc_fix_point(routenum, 0);
        fixed_plane[routenum]->pt[3] = calc_fix_point(routenum, 3);
    }
}

Point calc_fix_point(int routenum, int point_index) {
    double m;
    Point result, p0, p1;

    if (route_distance[routenum] > (BOX_FIXED_LENGTH * 2)) {
        m = (route_distance[routenum] - BOX_FIXED_LENGTH) / route_distance[routenum];
    } else {
        m = 0.5;
    }


    POINT(result, 0, ROUTE_PLANE_HEIGHT, 0, 0, 0);
    POINT(p0, 0, ROUTE_PLANE_HEIGHT, 0, 0, 0);
    POINT(p1, base_plane[routenum]->pt[point_index].x, ROUTE_PLANE_HEIGHT, base_plane[routenum]->pt[point_index].z, 0, 0);

    if (point_index == 0) {
        p0.x = base_plane[routenum]->pt[1].x;
        p0.z = base_plane[routenum]->pt[1].z;
    } else if (point_index == 1) {
        p0.x = base_plane[routenum]->pt[0].x;
        p0.z = base_plane[routenum]->pt[0].z;
    } else if (point_index == 2) {
        p0.x = base_plane[routenum]->pt[3].x;
        p0.z = base_plane[routenum]->pt[3].z;
    } else {
        p0.x = base_plane[routenum]->pt[2].x;
        p0.z = base_plane[routenum]->pt[2].z;
    }

    result.x = (m * (p1.x - p0.x)) + p0.x;
    result.z = (m * (p1.z - p0.z)) + p0.z;

    return result;
}

void draw_curve(int routenum) {
    int i;

    Point con_point1;
    Point con_point2;
    Point curve_point1[ROUTE_DIVISION_NUMBER+1];
    Point curve_point2[ROUTE_DIVISION_NUMBER+1];

    calc_intersection(base_plane[routenum]->pt[0], base_plane[routenum]->pt[1],
                    base_plane[routenum+1]->pt[0], base_plane[routenum+1]->pt[1], &con_point1);
    calc_intersection(base_plane[routenum]->pt[3], base_plane[routenum]->pt[2],
                    base_plane[routenum+1]->pt[3], base_plane[routenum+1]->pt[2], &con_point2);

#ifdef _E3D_H_OUTPUT_PRINT_
    uim_debug("con_point1 x = %f, z = %f \n", con_point1.x, con_point1.z);
    uim_debug("con_point2 x = %f, z = %f \n", con_point2.x, con_point2.z);
#endif

    calc_bezier_points(fixed_plane[routenum]->pt[0], con_point1, fixed_plane[routenum+1]->pt[1], curve_point1);
    calc_bezier_points(fixed_plane[routenum]->pt[3], con_point2, fixed_plane[routenum+1]->pt[2], curve_point2);

    for (i = 0; i < (ROUTE_DIVISION_NUMBER) + 1 ;i++) {
        if (curve_point1[i].z < Z_LIMIT) {
            curve_point1[i].z = Z_LIMIT;
        }
        if (curve_point2[i].z < Z_LIMIT) {
            curve_point2[i].z = Z_LIMIT;
        }
    }

    for (i = 0; i < ROUTE_DIVISION_NUMBER; i++) {
        PLANE_POINT(division_plane[routenum][i], 0, curve_point1[i+1].x, ROUTE_PLANE_HEIGHT, curve_point1[i+1].z, 0, 0);
        PLANE_POINT(division_plane[routenum][i], 1, curve_point1[i].x, ROUTE_PLANE_HEIGHT, curve_point1[i].z, 256, 0);
        PLANE_POINT(division_plane[routenum][i], 2, curve_point2[i].x, ROUTE_PLANE_HEIGHT, curve_point2[i].z, 256, 256);
        PLANE_POINT(division_plane[routenum][i], 3, curve_point2[i+1].x, ROUTE_PLANE_HEIGHT, curve_point2[i+1].z, 0, 256);

        _plane_draw(division_plane[routenum][i]);

#ifdef _E3D_H_OUTPUT_PRINT_
        uim_debug("Division Plane %d : x0 = %f, z0 = %f, x1 = %f, z1 = %f, x2 = %f, z2 = %f, x3 = %f, z3 = %f\n",
            routenum, division_plane[routenum][i]->pt[0].x, division_plane[routenum][i]->pt[0].z,
               division_plane[routenum][i]->pt[1].x, division_plane[routenum][i]->pt[1].z,
               division_plane[routenum][i]->pt[2].x, division_plane[routenum][i]->pt[2].z,
               division_plane[routenum][i]->pt[3].x, division_plane[routenum][i]->pt[3].z);
        uim_debug("Draw division plane.\n");
#endif

    }
}


void calc_bezier_points(Point p0, Point p1, Point p2, Point *points) {
    int i;
    double a, b, x, z;
    Point p;

    for(i = 0; i < ROUTE_DIVISION_NUMBER + 1; i++) {
        b = (double)i / ROUTE_DIVISION_NUMBER;
        a = 1 - b;

        x = a * a * p0.x + 2 * a * b * p1.x + b * b * p2.x;
        z = a * a * p0.z + 2 * a * b * p1.z + b * b * p2.z;

        POINT(p, x, ROUTE_PLANE_HEIGHT, z, 0, 0);
        points[i] = p;

#ifdef _E3D_H_OUTPUT_PRINT_
        uim_debug("bezier_points %d : x = %f, y = %f, z = %f\n", i, points[i].x, points[i].y, points[i].z);
#endif
    }
}

void calc_base_box_points_all() {
    int i;
    for (i = 0; i < route_draw_num - 1; i++) {
        calc_base_box_points(i);
    }

}

void calc_base_box_points(int routenum) {
    /* Determine the angle. */
    double th = atan2((route[routenum+1].z - route[routenum].z),
                    (route[routenum+1].x - route[routenum].x));
    /* Determine the distance. */
    double dis = route_distance[routenum];

#ifdef _E3D_H_OUTPUT_PRINT_
    uim_debug("th = %f\n", (th*180/M_PI));
#endif

    PLANE_POINT(base_plane[routenum], 0,
            (cos(th)*dis - sin(th)*(-ROUTE_PLANE_WIDTH)) + route[routenum].x,
            ROUTE_PLANE_HEIGHT,
            (sin(th)*dis + cos(th)*(-ROUTE_PLANE_WIDTH)) + route[routenum].z, 0, 0);

    PLANE_POINT(base_plane[routenum], 1,
            (cos(th)*0 - sin(th)*(-ROUTE_PLANE_WIDTH)) + route[routenum].x,
            ROUTE_PLANE_HEIGHT,
            (sin(th)*0 + cos(th)*(-ROUTE_PLANE_WIDTH)) + route[routenum].z, 256, 0);

    PLANE_POINT(base_plane[routenum], 2,
            (cos(th)*0 - sin(th)*(ROUTE_PLANE_WIDTH)) + route[routenum].x,
            ROUTE_PLANE_HEIGHT,
            (sin(th)*0 + cos(th)*(ROUTE_PLANE_WIDTH)) + route[routenum].z, 256, 256);

    PLANE_POINT(base_plane[routenum], 3,
            (cos(th)*dis - sin(th)*(ROUTE_PLANE_WIDTH)) + route[routenum].x,
            ROUTE_PLANE_HEIGHT,
            (sin(th)*dis + cos(th)*(ROUTE_PLANE_WIDTH)) + route[routenum].z, 0, 256);

#ifdef _E3D_H_OUTPUT_PRINT_
    uim_debug("Plane %d : x0 = %f, z0 = %f, x1 = %f, z1 = %f, x2 = %f, z2 = %f, x3 = %f, z3 = %f\n",
            routenum, base_plane[routenum]->pt[0].x, base_plane[routenum]->pt[0].z,
               base_plane[routenum]->pt[1].x, base_plane[routenum]->pt[1].z,
               base_plane[routenum]->pt[2].x, base_plane[routenum]->pt[2].z,
               base_plane[routenum]->pt[3].x, base_plane[routenum]->pt[3].z);
#endif
}

/******************************************************************************
 * Functions for Calculate Landmark Points
 ******************************************************************************/
void calc_landmarks_coord_by_camera() {
    int i;

    for (i = 0; i < landmark_data_count; i++) {
        landmark[i].x = landmark_raw[i].x;
        landmark[i].y = landmark_raw[i].y;
        landmark[i].z = landmark_raw[i].z;
    }

}

/******************************************************************************
 * Functions for Calculate (Common)
 ******************************************************************************/
void calc_route_distance() {
    int i;
    for (i = 0; i < route_draw_num - 1; i++) {
        route_distance[i] = calc_length(route[i], route[i+1]);
    }
}

/** Calculate the distance between two points.(only x-z plane) */
double calc_length(Point p0, Point p1) {
    return sqrt( (p1.x - p0.x) * (p1.x - p0.x) + (p1.z - p0.z) * (p1.z - p0.z) );
}

/** Calculate the square of distance between two points.(only x-z plane) */
double calc_square_length(Point p0, Point p1) {
    return ((p1.x - p0.x) * (p1.x - p0.x) + (p1.z - p0.z) * (p1.z - p0.z));
}

/* rotate point (center is orizin, xz-plane) */
void rotate_xz(Point src, Point *dist, double angle) {
    double rx, rz, sine, cosine;
    double rad;

    rad = (-angle) * M_PI / 180.0;

    sine = sin(rad);
    cosine = cos(rad);

    rx = cosine * src.x - sine * src.z;
    rz = sine * src.x + cosine * src.z;

    dist->x = rx;
    dist->y = src.y;
    dist->z = rz;
    dist->u = src.u;
    dist->v = src.v;
}

void calc_intersection(Point p0, Point p1, Point p2, Point p3, Point *point) {
    double S1, S2, result_x, result_z;
    S1 = ((p2.x - p3.x) * (p1.z - p3.z) - (p2.z - p3.z) * (p1.x - p3.x))/2;
    S2 = ((p2.x - p3.x) * (p3.z - p0.z) - (p2.z - p3.z) * (p3.x - p0.x))/2;

    result_x = (p1.x + (p0.x - p1.x) * S1 / (S1 + S2));
    result_z = (p1.z + (p0.z - p1.z) * S1 / (S1 + S2));

    point->x = result_x;
    point->y = ROUTE_PLANE_HEIGHT;
    point->z = result_z;
    point->u = 0;
    point->v = 0;
}

int length_comp(const void *pl0, const void *pl1) {
    PointLength point_length0 = *(PointLength *)pl0;
    PointLength point_length1 = *(PointLength *)pl1;

    if (point_length0.length == point_length1.length) {
        return 0;
    } else if (point_length0.length < point_length1.length) {
        return 1;
    }
    return -1;
}


