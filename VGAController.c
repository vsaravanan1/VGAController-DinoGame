// Include standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
// Include Pico libraries
#include "pico/stdlib.h"
#include "pico/divider.h"
#include "pico/multicore.h"
// Include hardware libraries
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"
#include "hardware/pll.h"
#include "hardware/uart.h"
#include "hardware/timer.h"

#include "hsync.pio.h"
#include "vsync.pio.h"
#include "rgb.pio.h"
#include "graphics.h"
#include "pt_cornell_rp2040_v1_3.h"

uint8_t VGA_CHAR_ARRAY[153600] = {0};
uint32_t CTRL_BLOCKS[1] = {VGA_CHAR_ARRAY};
const int TX_COUNT = 153600;
static float gravity = 0.65f;
static float accum_velocity_change = 0.0f;
volatile CloudSprite* last_cloud_spawned = NULL;
volatile ObstacleSprite* last_obs_spawned = NULL;
static volatile bool gameOver = false;
static volatile bool flapWings = false;


#define DINO_TIMER_DELAY 20000
#define DINO_WALK_TIMER_DELAY 100000
#define CLOUD_UPDATE_DELAY 25000
#define FLAP_WINGS_DELAY 500000

typedef enum {
    dino = 0, 
    walking_dino_left = 1, 
    walking_dino_right = 2,
    road = 3,
    cooked_dino = 4,
    game_over = 5
} SPRITES_ENUM;


static volatile uint8_t moveDino = 0;
static volatile uint8_t executeJump = 0;
static volatile GraphicsManager g;
static volatile uint8_t moveCloud = 0;
static volatile uint8_t moveRoad = 0;
static volatile uint8_t walkDino = 0;
static volatile uint8_t flapCounter = 0;
static volatile uint8_t spawn_counter = 200;

typedef volatile struct {
    volatile uint8_t DINO_RENDER;
    volatile uint8_t WALKING_DINO_LEFT_RENDER;
    volatile uint8_t WALKING_DINO_RIGHT_RENDER;
    volatile uint8_t BACKGROUND_RENDER;
    volatile uint8_t CACTUS_RENDER;
    
} RENDER;

RENDER draw_graphics = (RENDER){0, 0, 0, 0, 0};
StartingConditions starting_indices = {-1, -1};

//function prototypes
void hsync_init(PIO pio, uint sm, uint offset, uint pin);
void vsync_init(PIO pio, uint sm, uint offset, uint pin);
void rgb_init(PIO pio, uint sm, uint offset, uint pin);
void timer_init();


void hsync_init(PIO pio, uint sm, uint offset, uint pin) {
    hsync_program_init(pio, sm, offset, pin);
    pio->txf[sm] = 655;
    pio_sm_set_enabled(pio, sm, false);

    printf("Started HSYNC");
}


void vsync_init(PIO pio, uint sm, uint offset, uint pin) {
    vsync_program_init(pio, sm, offset, pin);
    pio->txf[sm] = 479;
    pio_sm_set_enabled(pio, sm, false);
}

void dino_irq() {
    if (timer_hw->intr & 1u << 1) {
        hw_clear_bits(&timer_hw->intr, 1u << 1);
        if (!gameOver) {
            moveDino = 1;
        }
        timer_hw->alarm[1] = (uint32_t)(uint64_t)(timer_hw->timerawl + DINO_TIMER_DELAY);   
    }
    if (timer_hw->intr & 1u << 0) {;
        hw_clear_bits(&timer_hw->intr, 1u << 0);
        if (!gameOver) {
            walkDino = 1;
            flapCounter++;
            if (flapCounter%5 == 0) {
                flapWings = 1;
                flapCounter = 0;
            }
        }
        timer_hw->alarm[0] = (uint32_t)(uint64_t)(timer_hw->timerawl + DINO_WALK_TIMER_DELAY);
    }
}

void background_irq() {
    hw_clear_bits(&timer_hw->intr, 1u << 2);
    if (!gameOver) {
        moveCloud = 1;
        moveRoad = 1;
    }

    if (spawn_counter == 0) {
        last_obs_spawned = &(g.obs_sprite_arr[starting_indices.obs_idx]);
        g.obs_visible[starting_indices.obs_idx] = 1;
        spawn_counter--;
    }
    else if (spawn_counter > 0) {
        spawn_counter--;
    }

    timer_hw->alarm[2] = (uint32_t)(uint64_t)(timer_hw->timerawl + CLOUD_UPDATE_DELAY);
}


void timer_init() {
    hw_set_bits(&timer_hw->inte, 0b111);
    irq_add_shared_handler(TIMER_IRQ_1, dino_irq, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
    irq_set_enabled(TIMER_IRQ_1, true);
    irq_add_shared_handler(TIMER_IRQ_0, dino_irq, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
    irq_set_enabled(TIMER_IRQ_0, true);
    irq_set_exclusive_handler(TIMER_IRQ_2, background_irq);
    irq_set_enabled(TIMER_IRQ_2, true);

    timer_hw->alarm[0] = (uint32_t)(uint64_t)(timer_hw->timerawl + DINO_WALK_TIMER_DELAY);    
    timer_hw->alarm[1] = (uint32_t)(uint64_t)(timer_hw->timerawl + DINO_TIMER_DELAY);
    timer_hw->alarm[2] = (uint32_t)(uint64_t)(timer_hw->timerawl + CLOUD_UPDATE_DELAY);
    
}


void rgb_init(PIO pio, uint sm, uint offset, uint pin) {
    rgb_program_init(pio, sm, offset, pin);
    pio->txf[sm] = 319;

    int ctrl_chan = 0;
    int data_chan = 1;

    dma_channel_claim(ctrl_chan);
    dma_channel_claim(data_chan);

    dma_channel_config c = dma_channel_get_default_config(ctrl_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, false);
    channel_config_set_chain_to(&c, data_chan);
    
    dma_channel_configure(
        ctrl_chan,
        &c,
        &dma_hw->ch[data_chan].al1_read_addr,
        CTRL_BLOCKS,
        1,
        false
    );
    
    dma_channel_config c_data = dma_channel_get_default_config(data_chan);
    channel_config_set_transfer_data_size(&c_data, DMA_SIZE_8);
    channel_config_set_read_increment(&c_data, true);
    channel_config_set_write_increment(&c_data, false);
    channel_config_set_dreq(&c_data, DREQ_PIO0_TX2);
    channel_config_set_chain_to(&c_data, ctrl_chan);

    dma_channel_configure(
        data_chan,
        &c_data,
        &pio0->txf[sm],
        &VGA_CHAR_ARRAY,
        TX_COUNT,
        false
    );
    
    dma_start_channel_mask(1u << ctrl_chan | 1u << data_chan);
    pio_sm_set_enabled(pio, sm, false);
}



static PT_THREAD (dino_jump_thread(struct pt* pt)) {
    PT_BEGIN(pt);
    
    while (1) {
        if (moveDino == 1) {
            accum_velocity_change += gravity;
            accum_velocity_change = accum_velocity_change > 1.0f ? 1.0f : accum_velocity_change;
            g.sprite_arr[dino].loc.x += g.sprite_arr[dino].vel.vx;
            g.sprite_arr[dino].loc.y += g.sprite_arr[dino].vel.vy;       


            g.sprite_arr[dino].vel.vy += (int)accum_velocity_change;
            if (accum_velocity_change >= 1.0f) {
                accum_velocity_change = 0.0f;
            }

            if (g.sprite_arr[dino].loc.y >= 360) {
                g.sprite_arr[dino].loc.y = 360;
                if (g.sprite_arr[dino].vel.vy > 0) {
                    executeJump = 0;
                    g.sprite_arr[dino].vel.vy = 0;
                }
                else if (g.sprite_arr[dino].vel.vy == 0) {
                    g.sprite_arr[dino].loc.y = 360;
                    g.sprite_arr[dino].vel.vy = -25;
                }
            }
            else if (g.sprite_arr[dino].loc.y <= 100) {
                g.sprite_arr[dino].loc.y = 100;
                if (g.sprite_arr[dino].vel.vy < 0) {
                    g.sprite_arr[dino].vel.vy = 0;
                }
            }


        //  update_screen_array_rle(&g, dino);
            draw_graphics.DINO_RENDER = 1;
            moveDino = 0;
        }
        PT_YIELD_UNTIL(pt, moveDino == 1 && executeJump == 1 && !gameOver);
    }
    PT_END(pt);
}

static PT_THREAD (dino_walking_thread(struct pt* pt)) {
    PT_BEGIN(pt);
    static uint8_t counter = 0;
    while (1) {
        if (walkDino == 1) {
            if (counter == 0 && executeJump == 0) {
                draw_graphics.WALKING_DINO_LEFT_RENDER = 1;
            //    update_screen_array_rle(&g, walking_dino_left);
            }
            else if (counter == 1 && executeJump == 0) {
                draw_graphics.WALKING_DINO_RIGHT_RENDER = 1;
            //    update_screen_array_rle(&g, walking_dino_right);
            }
            counter++;
            counter = counter%2;
            walkDino = 0;
        }

        PT_YIELD_UNTIL(pt, walkDino == 1 && executeJump == 0 && !gameOver);
    }
    PT_END(pt);
}

static PT_THREAD(background_thread(struct pt* pt)) {
    PT_BEGIN(pt);
    while (1) {
        if (moveCloud == 1) {
            static uint8_t spawn_new_cloud = false;
            static uint8_t num_invisible = 0;
            static uint8_t invisible_indices[4];
            for (int i = 0; i < 4; i++) {
                if (g.clouds_visible[i] == 1) {
                    g.cloud_sprite_arr[i].loc.x += g.cloud_sprite_arr[i].vel.vx;
                    g.cloud_sprite_arr[i].loc.y += g.cloud_sprite_arr[i].vel.vy;
                    if (g.cloud_sprite_arr[i].loc.x <= -1 * g.cloud_sprite_arr[i].width) {
                        g.cloud_sprite_arr[i].loc.x = 640;
                        g.clouds_visible[i] = 0;
                    }
                    if (g.cloud_sprite_arr[i].loc.x <= (640 - g.cloud_sprite_arr[i].next_cloud_dist)) {
                        if (last_cloud_spawned != NULL) {
                            if ((last_cloud_spawned->loc).x <= (640 - 150)) {
                                spawn_new_cloud = true;
                            }
                        }
                    }
                }
                else {
                    invisible_indices[num_invisible] = i;
                    num_invisible++;  
                }
            }

            if (spawn_new_cloud == true && num_invisible > 0) {
                srand(42);
                uint8_t random_index = invisible_indices[rand()%num_invisible];
                g.cloud_sprite_arr[random_index].next_cloud_dist = 200 + rand()%100;
                g.cloud_sprite_arr[random_index].loc.x = 640;
                g.clouds_visible[random_index] = 1;
                spawn_new_cloud = false;
                last_cloud_spawned = &(g.cloud_sprite_arr[random_index]);
            }

            num_invisible = 0;
        }
        if (moveRoad == 1) {
            static uint8_t spawn_new_obs = false;
            static uint8_t num_obs_invisible = 0;
            static uint8_t invisible_obs_indices[7];

            for (int i = 0; i < 7; i++) {
                if (g.obs_visible[i] == 1) {
                    if (g.obs_sprite_arr[i].loc.x <= -1 * g.obs_sprite_arr[i].width) {
                        g.obs_sprite_arr[i].loc.x = 640;
                        g.obs_visible[i] = 0;
                    }
                    if (g.obs_sprite_arr[i].loc.x <= (640 - g.obs_sprite_arr[i].next_obs_dist)) {
                        if (last_obs_spawned != NULL) {
                            if ((last_obs_spawned->loc).x <= (640 - 400)) {
                                spawn_new_obs = true;
                            }
                        }
                    }
                    g.obs_sprite_arr[i].loc.x += g.obs_sprite_arr[i].vel.vx;
                    g.obs_sprite_arr[i].loc.y += g.obs_sprite_arr[i].vel.vy;
                }
                else {
                    invisible_obs_indices[num_obs_invisible] = i;
                    num_obs_invisible++;  
                }
            }

            if (g.sprite_arr[road].loc.x <= -1 * (64 - g.sprite_arr[road].vel.vx)) {
                g.sprite_arr[road].loc.x = 0;
            }
            else {
                g.sprite_arr[road].loc.x += g.sprite_arr[road].vel.vx;
                g.sprite_arr[road].loc.y += g.sprite_arr[road].vel.vy;
            }

            if (spawn_new_obs == true && num_obs_invisible > 0) {
                uint8_t random_index = invisible_obs_indices[rand()%num_obs_invisible];
                g.obs_sprite_arr[random_index].next_obs_dist = 400 + rand()%100 + g.obs_sprite_arr[random_index].width;
                g.obs_sprite_arr[random_index].loc.x = 640;
                g.obs_visible[random_index] = 1;
                spawn_new_obs = false;
                last_obs_spawned = &(g.obs_sprite_arr[random_index]);
            }

            num_obs_invisible = 0;
            
        }
        moveCloud = 0;
        moveRoad = 0;
        draw_graphics.BACKGROUND_RENDER = 1;
        PT_YIELD_UNTIL(pt, moveCloud == 1 && moveRoad == 1 && !gameOver);
    }

    PT_END(pt);
}

static PT_THREAD(flap_wings_thread(struct pt* pt)) {
    PT_BEGIN(pt);
    static int static_counter = 0;
    while (1) {
        if (flapWings == 1) {
            if (static_counter == 0) {
                clear_rectangle(&g, g.obs_sprite_arr[6].loc, g.obs_sprite_arr[6].width, 69);
                g.obs_sprite_arr[6].buffer = BIRD_WING_UP;
                g.obs_sprite_arr[6].height = 60;
                g.obs_sprite_arr[6].width = 99;
         //       g.obs_sprite_arr[6].loc.y -= 14;
                g.obs_sprite_arr[6].past_loc = g.obs_sprite_arr[6].loc;
                g.obs_rle_buf_len[6] = 318;
            }
            else if (static_counter == 1) {
                clear_rectangle(&g, g.obs_sprite_arr[6].loc, g.obs_sprite_arr[6].width, 60);
                g.obs_sprite_arr[6].buffer = BIRD_WING_DOWN;
                g.obs_sprite_arr[6].height = 69;
                g.obs_sprite_arr[6].width = 99;
         //       g.obs_sprite_arr[6].loc.y += 14;
                g.obs_sprite_arr[6].past_loc = g.obs_sprite_arr[6].loc;
                g.obs_rle_buf_len[6] = 278;
            }
            flapWings = 0;
            static_counter++;
            static_counter %= 2;
        }
        PT_YIELD_UNTIL(pt, flapWings == 1 && gameOver == 0);
    }
    PT_END(pt);
}

static PT_THREAD (render_thread(struct pt* pt)) {
    PT_BEGIN(pt);
    while(1) {
        if (check_collision(&g) && !gameOver) {
            gameOver = true;
            update_screen_array_rle(&g, dino);
            g.sprite_arr[cooked_dino].loc = g.sprite_arr[dino].loc;
            update_screen_array_rle(&g, cooked_dino);
            update_screen_array_rle(&g, game_over);
        }
        if (draw_graphics.BACKGROUND_RENDER == 1) {
            draw_graphics.BACKGROUND_RENDER = 0;
            for (int i = 0; i < 4; i++) {
                update_screen_array_rle_cloud(&g, i);
            }
            for (int i = 0; i < 7; i++) {
              update_screen_array_rle_obstacle(&g, i);
            }
            update_screen_array_rle(&g, road);
        }
        if (draw_graphics.DINO_RENDER == 1) {
            draw_graphics.DINO_RENDER = 0;
            update_screen_array_rle(&g, dino);
        }
        if (draw_graphics.WALKING_DINO_LEFT_RENDER == 1) {
            draw_graphics.WALKING_DINO_LEFT_RENDER = 0;
            update_screen_array_rle(&g, walking_dino_left);
        }
        if (draw_graphics.WALKING_DINO_RIGHT_RENDER == 1) {
            draw_graphics.WALKING_DINO_RIGHT_RENDER = 0;
            update_screen_array_rle(&g, walking_dino_right);
        }
        
        PT_YIELD_usec(50);
    }

    PT_END(pt);
}

static PT_THREAD(usb_control_thread(struct pt* pt)) {
    PT_BEGIN(pt);

    while (1) {
        char c = stdio_getchar_timeout_us(1);
        if (c != PICO_ERROR_TIMEOUT && c == 'u') {
            executeJump = 1;
        }
        PT_YIELD_usec(500);
    } 

    PT_END(pt);
}




int main()
{
    stdio_init_all();
    timer_init();

    starting_indices = graphics_init(&g, VGA_CHAR_ARRAY, 640, 480);
    last_cloud_spawned = &(g.cloud_sprite_arr[starting_indices.cloud_idx]);
    update_screen_array_rle(&g, dino);
    update_screen_array_rle(&g, road);

    // PIO 
    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &hsync_program);
    uint offset2 = pio_add_program(pio, &vsync_program);
    uint offset3 = pio_add_program(pio, &rgb_program);
    printf("%d", (int)offset);

    hsync_init(pio, sm, offset, 3);
    vsync_init(pio, sm+1, offset2, 4);
    rgb_init(pio, sm+2, offset3, 5);

    pio_enable_sm_mask_in_sync(pio, 1u << 0 | 1u << 1 | 1u << 2);

    
    pt_add_thread(render_thread);
    pt_add_thread(flap_wings_thread);
    pt_add_thread(background_thread);
    pt_add_thread(dino_jump_thread);
    pt_add_thread(usb_control_thread);
    pt_add_thread(dino_walking_thread);


    pt_sched_method = SCHED_ROUND_ROBIN;
    pt_schedule_start;
}
