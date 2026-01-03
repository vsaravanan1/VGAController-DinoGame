#ifndef GRAPHICS
#define GRAPHICS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sprites.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    int vx;
    int vy;
} Velocity;

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
    uint8_t* buffer;
    int width;
    int height;
    Sprite sprite_arr[10];
} GraphicsManager;

void graphics_init(GraphicsManager* g, uint8_t* buffer, int width, int height) {
    g->width = width;
    g->height = height;
    g->buffer = buffer;
    memset(g->buffer, 0b111111, (width * height) / 2);

    Sprite DINO_SPRITE;
    DINO_SPRITE.buffer = DINO_SPRITE_ARRAY;
    DINO_SPRITE.loc = (Point){50, 380};
    DINO_SPRITE.vel = (Velocity){0, 0};
    DINO_SPRITE.height = 100;
    DINO_SPRITE.width = 100;
    DINO_SPRITE.past_loc = (Point){-1, -1};

    g->sprite_arr[0] = DINO_SPRITE;
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
        g->buffer[index] = (uint8_t)((g->buffer[index] & 0b111) | (value << 3));
    }
    else {
        g->buffer[index] = (uint8_t)(g->buffer[index] & 0b111000) | value;
    }
}

void get_pixel(GraphicsManager* g, Point p) {
    if (0 <= p.x && p.x < g->width && 0 <= p.y && p.y < g->height) {}
    else {
        return;
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
    while (rle_idx < (542)) {
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


#endif