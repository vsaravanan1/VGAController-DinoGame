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
static float gravity = 0.5f;
static float accum_velocity_change = 0.0f;

#define DINO_TIMER_DELAY 20000

typedef enum {dino = 0, cloud = 1, walking_dino_1 = 2, walking_dino_2 = 3} SPRITES_ENUM;

static volatile int moveDino = 0;
static volatile int executeJump = 0;
static volatile GraphicsManager g;

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
    hw_clear_bits(&timer_hw->intr, 1u << 1);
    moveDino = 1;
    timer_hw->alarm[1] = (uint32_t)(uint64_t)(timer_hw->timerawl + DINO_TIMER_DELAY);
}

void timer_init() {
    hw_set_bits(&timer_hw->inte, 1u << 1);
    irq_set_exclusive_handler(TIMER_IRQ_1, dino_irq);
    irq_set_enabled(TIMER_IRQ_1, true);
    timer_hw->alarm[1] = (uint32_t)(uint64_t)(timer_hw->timerawl + DINO_TIMER_DELAY);
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

            printf("Entered\n");

            if (g.sprite_arr[dino].loc.y >= 380) {
                g.sprite_arr[dino].loc.y = 380;
                if ((g.sprite_arr[dino].vel.vy > 0)) {
                    executeJump = 0;
                    g.sprite_arr[dino].vel.vy = 0;
                }
                else if (g.sprite_arr[dino].vel.vy == 0) {
                    g.sprite_arr[dino].loc.y = 380;
                    g.sprite_arr[dino].vel.vy = -20;
                }
            }
            else if (g.sprite_arr[dino].loc.y <= 100) {
                g.sprite_arr[dino].loc.y = 100;
                if (g.sprite_arr[dino].vel.vy < 0) {
                    g.sprite_arr[dino].vel.vy = 0;
                }
            }

            g.sprite_arr[dino].vel.vy += (int)accum_velocity_change;
            if (accum_velocity_change >= 1.0f) {
                accum_velocity_change = 0.0f;
            }
            update_screen_array_rle(&g, dino);
            moveDino = 0;
        }
        PT_YIELD_UNTIL(pt, moveDino == 1 && executeJump == 1);
    }
    PT_END(pt);
} 

static PT_THREAD(usb_control_thread(struct pt* pt)) {
    PT_BEGIN(pt);

    while (1) {
        char c = stdio_getchar_timeout_us(1);
        if (c != PICO_ERROR_TIMEOUT && c == 'u') {
            executeJump = 1;
            printf("Debug info: executeJump: %d, dino_vel_y: %d, dino_loc_y: %d\n", executeJump, g.sprite_arr[dino].vel.vy, g.sprite_arr[dino].loc.y);
        }
        PT_YIELD_usec(5000);
    } 

    PT_END(pt);
}




int main()
{
    stdio_init_all();
    timer_init();

    graphics_init(&g, VGA_CHAR_ARRAY, 640, 480);
    update_screen_array_rle(&g, dino);

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

    
    pt_add_thread(usb_control_thread);
    pt_add_thread(dino_jump_thread);

    pt_sched_method = SCHED_ROUND_ROBIN;
    pt_schedule_start;
}
