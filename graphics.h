#ifndef GRAPHICS
#define GRAPHICS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sprites.h"
#include <stdint.h>
#include <stdbool.h>


typedef struct {
    int vx;
    int vy;
} Velocity;

typedef struct {
    int x;
    int y;
} Point;

typedef struct Sprite {
    uint16_t* buffer;
    //top left corner
    Point loc;
    Velocity vel;
    int width;
    int height;
    Point past_loc;
} Sprite;

typedef struct {
    uint16_t* buffer;
    Point loc;
    Velocity vel;
    uint16_t width;
    uint16_t height;
    Point past_loc;
    int16_t next_cloud_dist;
} CloudSprite;

typedef struct {
    uint16_t* buffer;
    Point loc;
    Velocity vel;
    uint16_t width;
    uint16_t height;
    Point past_loc;
    int16_t next_obs_dist;
} ObstacleSprite;


typedef struct {
    uint8_t* buffer;
    int width;
    int height;
    Sprite sprite_arr[10];
    uint16_t rle_buf_len[10];
    CloudSprite cloud_sprite_arr[4];
    uint16_t cloud_rle_buf_len[4];
    uint8_t clouds_visible[4];
    ObstacleSprite obs_sprite_arr[8];
    uint16_t obs_rle_buf_len[8];
    uint8_t obs_visible[8];
} GraphicsManager;



typedef struct {
    uint8_t cloud_idx;
    uint8_t obs_idx;
} StartingConditions;

StartingConditions graphics_init(GraphicsManager* g, uint8_t* buffer, int width, int height) {
    g->width = width;
    g->height = height;
    g->buffer = buffer;
    memset(g->buffer, 0b111111, (width * height) / 2);

    Sprite DINO_SPRITE;
    DINO_SPRITE.buffer = DINO_SPRITE_ARRAY;
    DINO_SPRITE.loc = (Point){50, 360};
    DINO_SPRITE.vel = (Velocity){0, 0};
    DINO_SPRITE.height = 100;
    DINO_SPRITE.width = 100;
    DINO_SPRITE.past_loc = (Point){-1, -1};

    Sprite DINO_WALKING_LEFT_SPRITE;
    DINO_WALKING_LEFT_SPRITE.buffer = DINO_WALKING_LEFT;
    DINO_WALKING_LEFT_SPRITE.loc = (Point){50, 360};
    DINO_WALKING_LEFT_SPRITE.vel = (Velocity){0, 0};
    DINO_WALKING_LEFT_SPRITE.height = 100;
    DINO_WALKING_LEFT_SPRITE.width = 100;
    DINO_WALKING_LEFT_SPRITE.past_loc = (Point){-1, -1};
    
    Sprite DINO_WALKING_RIGHT_SPRITE;
    DINO_WALKING_RIGHT_SPRITE.buffer = DINO_WALKING_RIGHT;
    DINO_WALKING_RIGHT_SPRITE.loc = (Point){50, 360};
    DINO_WALKING_RIGHT_SPRITE.vel = (Velocity){0, 0};
    DINO_WALKING_RIGHT_SPRITE.height = 100;
    DINO_WALKING_RIGHT_SPRITE.width = 100;
    DINO_WALKING_RIGHT_SPRITE.past_loc = (Point){-1, -1};

    Sprite COOKED_DINO_SPRITE;
    COOKED_DINO_SPRITE.buffer = COOKED_DINO;
    COOKED_DINO_SPRITE.loc = DINO_SPRITE.loc;
    COOKED_DINO_SPRITE.vel = (Velocity){0, 0};
    COOKED_DINO_SPRITE.height = DINO_SPRITE.height;
    COOKED_DINO_SPRITE.width = DINO_SPRITE.width;
    COOKED_DINO_SPRITE.past_loc = (Point){-1, -1};

    Sprite ROAD_SPRITE;
    ROAD_SPRITE.buffer = ROAD;
    ROAD_SPRITE.loc = (Point){0, 460};
    ROAD_SPRITE.vel = (Velocity){-8, 0};
    ROAD_SPRITE.height = 20;
    ROAD_SPRITE.width = 704;
    ROAD_SPRITE.past_loc = (Point){-1, -1};


    CloudSprite CLOUD_SPRITE_1;
    CLOUD_SPRITE_1.buffer = CLOUD;
    CLOUD_SPRITE_1.loc = (Point){640, 20};
    CLOUD_SPRITE_1.vel = (Velocity){-2, 0};
    CLOUD_SPRITE_1.height = 50;
    CLOUD_SPRITE_1.width = 100;
    CLOUD_SPRITE_1.past_loc = (Point){-1, -1};
    CLOUD_SPRITE_1.next_cloud_dist = 200 + rand()%100;

    CloudSprite CLOUD_SPRITE_2;
    CLOUD_SPRITE_2.buffer = CLOUD;
    CLOUD_SPRITE_2.loc = (Point){640, 40};
    CLOUD_SPRITE_2.vel = (Velocity){-2, 0};
    CLOUD_SPRITE_2.height = 50;
    CLOUD_SPRITE_2.width = 100;
    CLOUD_SPRITE_2.past_loc = (Point){-1, -1};
    CLOUD_SPRITE_2.next_cloud_dist = 200 + rand()%100;
    
    CloudSprite CLOUD_SPRITE_3;
    CLOUD_SPRITE_3.buffer = SMALL_CLOUD;
    CLOUD_SPRITE_3.loc = (Point){640, 20};
    CLOUD_SPRITE_3.vel = (Velocity){-2, 0};
    CLOUD_SPRITE_3.height = 25;
    CLOUD_SPRITE_3.width = 50;
    CLOUD_SPRITE_3.past_loc = (Point){-1, -1};
    CLOUD_SPRITE_3.next_cloud_dist = 200 + rand()%100;

    CloudSprite CLOUD_SPRITE_4;
    CLOUD_SPRITE_4.buffer = SMALL_CLOUD;
    CLOUD_SPRITE_4.loc = (Point){640, 40};
    CLOUD_SPRITE_4.vel = (Velocity){-2, 0};
    CLOUD_SPRITE_4.height = 25;
    CLOUD_SPRITE_4.width = 50;
    CLOUD_SPRITE_4.past_loc = (Point){-1, -1};
    CLOUD_SPRITE_4.next_cloud_dist = 200 + rand()%100;

    ObstacleSprite CACTUS_SPRITE_1;
    CACTUS_SPRITE_1.buffer = CACTUS;
    CACTUS_SPRITE_1.width = 44;
    CACTUS_SPRITE_1.height = 80;
    CACTUS_SPRITE_1.loc = (Point){640, 480 - CACTUS_SPRITE_1.height - 20};
    CACTUS_SPRITE_1.vel = ROAD_SPRITE.vel;
    CACTUS_SPRITE_1.past_loc = (Point){-1, -1};
    CACTUS_SPRITE_1.next_obs_dist = 400 + rand()%100 + CACTUS_SPRITE_1.width;
    
    ObstacleSprite CACTUS_SPRITE_2;
    CACTUS_SPRITE_2.buffer = SMALL_CACTUS;
    CACTUS_SPRITE_2.width = 22;
    CACTUS_SPRITE_2.height = 40;
    CACTUS_SPRITE_2.loc = (Point){640, 480 - CACTUS_SPRITE_2.height - 20};
    CACTUS_SPRITE_2.vel = ROAD_SPRITE.vel;
    CACTUS_SPRITE_2.past_loc = (Point){-1, -1};
    CACTUS_SPRITE_2.next_obs_dist = 400 + rand()%100 + CACTUS_SPRITE_2.width;

    ObstacleSprite CACTUS_SPRITE_3;
    CACTUS_SPRITE_3.buffer = CACTUS_2;
    CACTUS_SPRITE_3.width = 90;
    CACTUS_SPRITE_3.height = 80;
    CACTUS_SPRITE_3.loc = (Point){640, 480 - CACTUS_SPRITE_3.height - 20};
    CACTUS_SPRITE_3.vel = ROAD_SPRITE.vel;
    CACTUS_SPRITE_3.past_loc = (Point){-1, -1};
    CACTUS_SPRITE_3.next_obs_dist = 400 + rand()%100 + CACTUS_SPRITE_3.width;

    ObstacleSprite CACTUS_SPRITE_4;
    CACTUS_SPRITE_4.buffer = SMALL_CACTUS_2;
    CACTUS_SPRITE_4.width = 45;
    CACTUS_SPRITE_4.height = 40;
    CACTUS_SPRITE_4.loc = (Point){640, 480 - CACTUS_SPRITE_4.height - 20};
    CACTUS_SPRITE_4.vel = ROAD_SPRITE.vel;
    CACTUS_SPRITE_4.past_loc = (Point){-1, -1};
    CACTUS_SPRITE_4.next_obs_dist = 400 + rand()%100 + CACTUS_SPRITE_4.width;

    ObstacleSprite CACTUS_SPRITE_5;
    CACTUS_SPRITE_5.buffer = CACTUS_3;
    CACTUS_SPRITE_5.width = 136;
    CACTUS_SPRITE_5.height = 80;
    CACTUS_SPRITE_5.loc = (Point){640, 480 - CACTUS_SPRITE_5.height - 20};
    CACTUS_SPRITE_5.vel = ROAD_SPRITE.vel;
    CACTUS_SPRITE_5.past_loc = (Point){-1, -1};
    CACTUS_SPRITE_5.next_obs_dist = 400 + rand()%100 + CACTUS_SPRITE_5.width;

    ObstacleSprite CACTUS_SPRITE_6;
    CACTUS_SPRITE_6.buffer = SMALL_CACTUS_3;
    CACTUS_SPRITE_6.width = 68;
    CACTUS_SPRITE_6.height = 40;
    CACTUS_SPRITE_6.loc = (Point){640, 480 - CACTUS_SPRITE_6.height - 20};
    CACTUS_SPRITE_6.vel = ROAD_SPRITE.vel;
    CACTUS_SPRITE_6.past_loc = (Point){-1, -1};
    CACTUS_SPRITE_6.next_obs_dist = 400 + rand()%100 + CACTUS_SPRITE_6.width;

    ObstacleSprite BIRD_SPRITE;
    BIRD_SPRITE.buffer = BIRD_WING_DOWN;
    BIRD_SPRITE.width = 99;
    BIRD_SPRITE.height = 69;
    BIRD_SPRITE.loc = (Point){640, 330};
    BIRD_SPRITE.vel = ROAD_SPRITE.vel;
    BIRD_SPRITE.past_loc = (Point){-1, -1};
    BIRD_SPRITE.next_obs_dist = 400 + rand()%100 + BIRD_SPRITE.width;
    
    ObstacleSprite BIRD_SPRITE_2;
    BIRD_SPRITE_2.buffer = BIRD_WING_UP;
    BIRD_SPRITE_2.width = 99;
    BIRD_SPRITE_2.height = 60;
    BIRD_SPRITE_2.vel = ROAD_SPRITE.vel;
    BIRD_SPRITE_2.loc = (Point){640, 316};
    BIRD_SPRITE_2.past_loc = (Point){-1, -1};
    BIRD_SPRITE_2.next_obs_dist = 400 + rand()%100 + BIRD_SPRITE_2.width;
    

    Sprite GAME_OVER_SPRITE;
    GAME_OVER_SPRITE.buffer = GAME_OVER;
    GAME_OVER_SPRITE.height = 112;
    GAME_OVER_SPRITE.width = 200;
    GAME_OVER_SPRITE.loc = (Point) {220, 184};


    g->sprite_arr[0] = DINO_SPRITE;
    g->sprite_arr[1] = DINO_WALKING_LEFT_SPRITE;
    g->sprite_arr[2] = DINO_WALKING_RIGHT_SPRITE;
    g->sprite_arr[3] = ROAD_SPRITE;
    g->sprite_arr[4] = COOKED_DINO_SPRITE;
    g->sprite_arr[5] = GAME_OVER_SPRITE;


    g->rle_buf_len[0] = 542;
    g->rle_buf_len[1] = 510;
    g->rle_buf_len[2] = 510;
    g->rle_buf_len[3] = 324;
    g->rle_buf_len[4] = 570;
    g->rle_buf_len[5] = 1882;

    g->cloud_sprite_arr[0] = CLOUD_SPRITE_1;
    g->cloud_sprite_arr[1] = CLOUD_SPRITE_2;
    g->cloud_sprite_arr[2] = CLOUD_SPRITE_3;
    g->cloud_sprite_arr[3] = CLOUD_SPRITE_4;

    g->cloud_rle_buf_len[0] = 242;
    g->cloud_rle_buf_len[1] = 242;
    g->cloud_rle_buf_len[2] = 122;
    g->cloud_rle_buf_len[3] = 122;

    for (int i = 0; i < 4; i++) {
        g->clouds_visible[i] = 0;
    }

    g->obs_sprite_arr[0] = CACTUS_SPRITE_1;
    g->obs_sprite_arr[1] = CACTUS_SPRITE_2;
    g->obs_sprite_arr[2] = CACTUS_SPRITE_3;
    g->obs_sprite_arr[3] = CACTUS_SPRITE_4;
    g->obs_sprite_arr[4] = CACTUS_SPRITE_5;
    g->obs_sprite_arr[5] = CACTUS_SPRITE_6;
    g->obs_sprite_arr[6] = BIRD_SPRITE;

    g->obs_rle_buf_len[0] = 438;
    g->obs_rle_buf_len[1] = 222;
    g->obs_rle_buf_len[2] = 950;
    g->obs_rle_buf_len[3] = 478;
    g->obs_rle_buf_len[4] = 1462;
    g->obs_rle_buf_len[5] = 734;
    g->obs_rle_buf_len[6] = 278;

    for (int i = 0; i < 7; i++) {
        g->obs_visible[i] = 0;
    }


    uint8_t first_cloud_index = rand()%4;
    g->clouds_visible[first_cloud_index] = 1;


    uint8_t first_obs_index = rand()%7;
    //don't spawn obstacle instantly, wait just like the chrome game
 //   g->obs_visible[first_obs_index] = 1;


    //replace with first cactus index once feature is added.
    StartingConditions indices = (StartingConditions) {first_cloud_index, first_obs_index};
    return indices;
}

void draw_pixel(GraphicsManager* g, Point p,  uint8_t value) {
    if (0 <= p.x && p.x < g->width && 0 <= p.y && p.y < g->height) {}
    else {
        return;
    }

    bool writeUpper;
    if ((p.x)%2 == 1) {
        writeUpper = true;
    }
    else {
        writeUpper = false;
    }
    
    int index = (p.y) * (g->width)/2 + (p.x)/2;
    if (writeUpper) {
        if (g->buffer[index] != (uint8_t)((g->buffer[index] & 0b111) | (value << 3))) {
            g->buffer[index] = (uint8_t)((g->buffer[index] & 0b111) | (value << 3));
        }
    }
    else {
        if (g->buffer[index] != (uint8_t)(g->buffer[index] & 0b111000) | value) {
            g->buffer[index] = (uint8_t)(g->buffer[index] & 0b111000) | value;
        }
    }
}

void clear_rectangle(GraphicsManager* g, Point top_left, int width, int height) {
    Point marker = top_left;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            marker.x++;
            draw_pixel(g, marker, 0b111);
        }
        marker.y++;
    }
}

uint8_t get_pixel(GraphicsManager* g, Point p) {
    if (0 <= p.x && p.x < g->width && 0 <= p.y && p.y < g->height) {}
    else {
        return 0;
    }

    bool readUpper;
    if ((p.x)%2 == 1) {
        readUpper = true;
    }
    else {
        readUpper = false;
    }
    
    int index = (p.y) * (g->width)/2 + (p.x)/2;
    if (readUpper) {
        return (g->buffer[index] >> 3) & 0b111;
    }
    else {
        return g->buffer[index] & 0b111;
    }

}

void update_screen_array(GraphicsManager* g, Point start, int sprite_num) {
    Point p;
    g->sprite_arr[sprite_num].loc = start;
    for (int x = 0; x < g->sprite_arr[sprite_num].width; x++) {
        for (int y = 0; y < g->sprite_arr[sprite_num].height; y++) {
            p.x = start.x + x;
            p.y = start.y + y;
            draw_pixel(g, p, g->sprite_arr[sprite_num].buffer[y * g->sprite_arr[sprite_num].width + x]);
        }
    }
}

Point create_point(Point* starting, int16_t x_dist, int16_t y_dist) {
    return (Point) {starting->x + x_dist, starting->y + y_dist};
}

bool check_collision(GraphicsManager* g) {
    bool collision_detected = false;
    Point dino_corners[4];
    dino_corners[0] = g->sprite_arr[0].loc;
    dino_corners[1] = (Point) {g->sprite_arr[0].loc.x + g->sprite_arr[0].width, g->sprite_arr[0].loc.y};
    dino_corners[2] = (Point) {g->sprite_arr[0].loc.x, g->sprite_arr[0].loc.y + g->sprite_arr[0].height};
    dino_corners[3] = (Point) {g->sprite_arr[0].loc.x + g->sprite_arr[0].width, g->sprite_arr[0].loc.y + g->sprite_arr[0].height};

    Point* starting = &(dino_corners[0]);

    Point hitboxes[3][4] = {
        {create_point(starting, 31, 3), create_point(starting, 69, 3), create_point(starting, 31, 97), create_point(starting, 69, 97)},
        {create_point(starting, 3, 44), create_point(starting, 30, 44), create_point(starting, 3, 85), create_point(starting, 30, 85)},
        {create_point(starting, 70, 3), create_point(starting, 97, 3), create_point(starting, 70, 29), create_point(starting, 97, 29)}
    };

    for (int i = 0; i < 7; ++i) {
        if (g->obs_visible[i] == 0) {
            continue;
        }
        Point obst_corners[4];
        obst_corners[0] = g->obs_sprite_arr[i].loc;
        obst_corners[1] = (Point) {g->obs_sprite_arr[i].loc.x + g->obs_sprite_arr[i].width, g->obs_sprite_arr[i].loc.y};
        obst_corners[2] = (Point) {g->obs_sprite_arr[i].loc.x, g->obs_sprite_arr[i].loc.y + g->obs_sprite_arr[i].height};
        obst_corners[3] = (Point) {g->obs_sprite_arr[i].loc.x + g->obs_sprite_arr[i].width, g->obs_sprite_arr[i].loc.y + g->obs_sprite_arr[i].height};
        for (int j = 0; j < 3; ++j) {
            for (int k = 0; k < 4; ++k) {
                Point dino_hb_point = hitboxes[j][k];
                bool point_in_obst = ((dino_hb_point.x >= obst_corners[0].x) && (dino_hb_point.x <= obst_corners[3].x) && (dino_hb_point.y >= obst_corners[0].y) && (dino_hb_point.y <= obst_corners[3].y));
                if (point_in_obst == true) {
                    collision_detected = true;   
                }
            }
        }
    }

    return collision_detected;
}

void update_screen_array_rle(GraphicsManager* g, int sprite_num) {    
    Point p;
    size_t rle_idx = 0;
    uint16_t width = g->sprite_arr[sprite_num].width;
    uint16_t height = g->sprite_arr[sprite_num].height;
    p.x = g->sprite_arr[sprite_num].loc.x;
    p.y = g->sprite_arr[sprite_num].loc.y;
    Point start = p;
    int x_dis = g->sprite_arr[sprite_num].loc.x - g->sprite_arr[sprite_num].past_loc.x;
    int y_dis = g->sprite_arr[sprite_num].loc.y - g->sprite_arr[sprite_num].past_loc.y;

    if (g->sprite_arr[sprite_num].past_loc.x != -1) {
        Point p2;
        p2.x = g->sprite_arr[sprite_num].past_loc.x;
        p2.y = g->sprite_arr[sprite_num].past_loc.y;
        Point p3;
        p3.x = p2.x + x_dis;
        p3.y = p2.y + y_dis;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (p2.x - p3.x >= 0 && p2.x - p3.x < width && p2.y - p3.y >= 0 && p2.y - p3.y < height) {
                    p2.x++;
                    continue;
                }
                draw_pixel(g, p2, 0b111);
                p2.x++;
            }
            p2.x = g->sprite_arr[sprite_num].past_loc.x;
            p2.y++;
        }
    }
    while (rle_idx < g->rle_buf_len[sprite_num]) {
        uint8_t value = g->sprite_arr[sprite_num].buffer[rle_idx];
        uint16_t count = g->sprite_arr[sprite_num].buffer[rle_idx+1];
        for (int i = 0; i < count; i++) {
            draw_pixel(g, p, value);
            p.x++;
            if (p.x >= (start.x + width)) {
                p.x = start.x;
                p.y++;
            }
            if (p.y >= (start.y + height)) {
                p.y = start.y;
            }
        }
        rle_idx += 2;
    }

    g->sprite_arr[sprite_num].past_loc = g->sprite_arr[sprite_num].loc;
}


void update_screen_array_rle_cloud(GraphicsManager* g, int cloud_num) {
    if (g->clouds_visible[cloud_num] == 0) {
        return;
    }

    Point p;
    size_t rle_idx = 0;
    uint16_t width = g->cloud_sprite_arr[cloud_num].width;
    uint16_t height = g->cloud_sprite_arr[cloud_num].height;
    p.x = g->cloud_sprite_arr[cloud_num].loc.x;
    p.y = g->cloud_sprite_arr[cloud_num].loc.y;
    Point start = p;
    int x_dis = g->cloud_sprite_arr[cloud_num].loc.x - g->cloud_sprite_arr[cloud_num].past_loc.x;
    int y_dis = g->cloud_sprite_arr[cloud_num].loc.y - g->cloud_sprite_arr[cloud_num].past_loc.y;

    if (p.x <= -100) {
        g->cloud_sprite_arr[cloud_num].loc.x = -100;
    }


    if (g->cloud_sprite_arr[cloud_num].past_loc.x != -1) {
        Point p2;
        p2.x = g->cloud_sprite_arr[cloud_num].past_loc.x;
        p2.y = g->cloud_sprite_arr[cloud_num].past_loc.y;
        Point p3;
        p3.x = p2.x + x_dis;
        p3.y = p2.y + y_dis;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (p2.x - p3.x >= 0 && p2.x - p3.x < width && p2.y - p3.y >= 0 && p2.y - p3.y < height) {
                    p2.x++;
                    continue;
                }
                draw_pixel(g, p2, 0b111);
                p2.x++;
            }
            p2.x = g->cloud_sprite_arr[cloud_num].past_loc.x;
            p2.y++;
        }
    }
    while (rle_idx < g->cloud_rle_buf_len[cloud_num]) {
        uint8_t value = g->cloud_sprite_arr[cloud_num].buffer[rle_idx];
        uint16_t count = g->cloud_sprite_arr[cloud_num].buffer[rle_idx+1];
        for (int i = 0; i < count; i++) {
            draw_pixel(g, p, value);
            p.x++;
            if (p.x >= (start.x + width)) {
                p.x = start.x;
                p.y++;
            }
            if (p.y >= (start.y + height)) {
                p.y = start.y;
            }
        }
        rle_idx += 2;
    }

    g->cloud_sprite_arr[cloud_num].past_loc = g->cloud_sprite_arr[cloud_num].loc;
}

void update_screen_array_rle_obstacle(GraphicsManager* g, int obs_num) {
    if (g->obs_visible[obs_num] == 0) {
        return;
    }

    Point p;
    size_t rle_idx = 0;
    uint16_t width = g->obs_sprite_arr[obs_num].width;
    uint16_t height = g->obs_sprite_arr[obs_num].height;
    p.x = g->obs_sprite_arr[obs_num].loc.x;
    p.y = g->obs_sprite_arr[obs_num].loc.y;
    Point start = p;
    int x_dis = g->obs_sprite_arr[obs_num].loc.x - g->obs_sprite_arr[obs_num].past_loc.x;
    int y_dis = g->obs_sprite_arr[obs_num].loc.y - g->obs_sprite_arr[obs_num].past_loc.y;

    if (p.x <= -1 * g->obs_sprite_arr[obs_num].width) {
        g->obs_sprite_arr[obs_num].loc.x = -1 * g->obs_sprite_arr[obs_num].width;
    }


    if (g->obs_sprite_arr[obs_num].past_loc.x != -1) {
        Point p2;
        p2.x = g->obs_sprite_arr[obs_num].past_loc.x;
        p2.y = g->obs_sprite_arr[obs_num].past_loc.y;
        Point p3;
        p3.x = p2.x + x_dis;
        p3.y = p2.y + y_dis;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (p2.x - p3.x >= 0 && p2.x - p3.x < width && p2.y - p3.y >= 0 && p2.y - p3.y < height) {
                    p2.x++;
                    continue;
                }
                draw_pixel(g, p2, 0b111);
                p2.x++;
            }
            p2.x = g->obs_sprite_arr[obs_num].past_loc.x;
            p2.y++;
        }
    }
    while (rle_idx < g->obs_rle_buf_len[obs_num]) {
        uint8_t value = g->obs_sprite_arr[obs_num].buffer[rle_idx];
        uint16_t count = g->obs_sprite_arr[obs_num].buffer[rle_idx+1];
        for (int i = 0; i < count; i++) {
            draw_pixel(g, p, value);
            p.x++;
            if (p.x >= (start.x + width)) {
                p.x = start.x;
                p.y++;
            }
            if (p.y >= (start.y + height)) {
                p.y = start.y;
            }
        }
        rle_idx += 2;
    }

    g->obs_sprite_arr[obs_num].past_loc = g->obs_sprite_arr[obs_num].loc;
}

#endif