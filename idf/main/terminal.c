#include "terminal.h"
#include "config.h"
#include "freertos/idf_additions.h"
#include "ssd1306.h"
#include "state.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static SSD1306_t disp;

#define SPIKE_W 3
#define GAP 2

#if CONFIG_SSD1306_128x64
#define DISP_ROWS 64
#define DISP_COLS 128
#endif // CONFIG_SSD1306_128x64
#if CONFIG_SSD1306_128x32
#define DISP_ROWS 32
#define DISP_COLS 128
#endif // CONFIG_SSD1306_128x32

// TODO: this font is too large
#define FONT_COLS 8
#define FONT_ROWS 8
#define COL_CHARS (DISP_COLS / FONT_COLS)

#define LEFT_COL 0
#define RIGHT_COL (DISP_COLS / 2)

#define MAX_BUF ((DISP_COLS / FONT_COLS) + 1)

static int scroll_offset = 0;

void draw_box_with_fill(SSD1306_t *dev, int x, int y, int w, int h, bool invert,
                        bool fill) {
  if (w <= 0 || h <= 0)
    return;

  int right = x + w - 1;
  int bottom = y + h - 1;

  if (fill) {
    for (int j = y; j <= bottom; j++) {
      for (int i = x; i <= right; i++) {
        _ssd1306_pixel(dev, i, j, invert);
      }
    }
  } else {
    for (int i = x; i <= right; i++) {
      _ssd1306_pixel(dev, i, y, invert);
      _ssd1306_pixel(dev, i, bottom, invert);
    }
    for (int j = y; j <= bottom; j++) {
      _ssd1306_pixel(dev, x, j, invert);
      _ssd1306_pixel(dev, right, j, invert);
    }
  }
}

void update_scroll_window(void) {
  const int spikes = (DISP_COLS + GAP) / (SPIKE_W + GAP);

  const int selected_ch = system_state_get_selected_ch();

  if (selected_ch < scroll_offset) {
    scroll_offset = selected_ch;
  } else if (selected_ch >= scroll_offset + spikes) {
    scroll_offset = selected_ch - (spikes - 1);
  }

  const int max_offset = CHANNELS - spikes;
  if (scroll_offset > max_offset)
    scroll_offset = max_offset;
  if (scroll_offset < 0)
    scroll_offset = 0;
}

void terminal_redraw_spectrum(void) {
  const int x = SPECTRUM_X;
  const int y = SPECTRUM_Y;
  const int w = DISP_COLS - x;
  const int h = DISP_ROWS - y;

  update_scroll_window();

  const int spikes = (w + GAP) / (SPIKE_W + GAP);

  int start = scroll_offset, end = start + spikes;
  if (end > CHANNELS)
    end = CHANNELS;

  const int selected_ch = system_state_get_selected_ch();
  for (int i = start; i < end; ++i) {
    int visible_index = i - start;

    bool active = system_state_get_spectrum_data(i);
    int spike_h = active ? h : 0;
    int spike_x = x + visible_index * (SPIKE_W + GAP);
    bool is_selected = (selected_ch == i);

    // clear the column before drawing by drawing a black rectangle
    draw_box_with_fill(&disp, spike_x, y, SPIKE_W, h, true,
                       true); // invert=true means black pixels

    if (active) {
      draw_box_with_fill(&disp, spike_x, y + (h - spike_h), SPIKE_W, spike_h,
                         false, is_selected);
    } else {
      if (is_selected) {
        const int selected_box_height = 3;
        draw_box_with_fill(&disp, spike_x, y + h - selected_box_height, SPIKE_W,
                           selected_box_height, false, true);
      } else {
        int ly = y + h - 1;
        _ssd1306_line(&disp, spike_x, ly, spike_x + SPIKE_W - 1, ly, false);
      }
    }
  }

  ssd1306_show_buffer(&disp);
}

void terminal_init(void) {
#if CONFIG_I2C_INTERFACE
  printf("display interface is i2c\n");
  printf("CONFIG_SDA_GPIO=%d\n", CONFIG_SDA_GPIO);
  printf("CONFIG_SCL_GPIO=%d\n", CONFIG_SCL_GPIO);
  i2c_master_init(&disp, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
#endif

#if CONFIG_SPI_INTERFACE
  printf("display interface is spi\n");
  spi_master_init(&dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO,
                  CONFIG_DC_GPIO, CONFIG_RESET_GPIO);
#endif

  printf("display is %dx%d\n", DISP_ROWS, DISP_COLS);
  ssd1306_init(&disp, DISP_COLS, DISP_ROWS);
  ssd1306_contrast(&disp, 0xff);

  ssd1306_clear_screen(&disp, false);
  terminal_redraw_info();
  terminal_redraw_spectrum();
}

void terminal_redraw_info(void) {
  // clear the first 3 rows at full width with black pixels
  draw_box_with_fill(&disp, 0, 0, DISP_COLS, 3 * FONT_ROWS, true, true);

  char buffers[OPT_MAX][MAX_BUF];
  int lengths[OPT_MAX];

  const int selected_ch = system_state_get_selected_ch();
  const int tx_power = system_state_get_tx_power();
  const int tx_speed = system_state_get_tx_speed();

  lengths[OPT_CHANNEL] =
      snprintf(buffers[OPT_CHANNEL], MAX_BUF, "CH=%d", selected_ch);
  lengths[OPT_POWER] = snprintf(buffers[OPT_POWER], MAX_BUF, "TX=%ddBm",
                                tx_power_to_dbm[tx_power]);
  lengths[OPT_SPEED] =
      snprintf(buffers[OPT_SPEED], MAX_BUF, "S=%.1fMbps(%dMHz)",
               tx_speed_to_mbps[tx_speed], tx_speed_to_bw_mhz[tx_speed]);

  const int hl = system_state_get_highlighted();

  ssd1306_display_text_box1(&disp, 0, 0, buffers[OPT_CHANNEL],
                            lengths[OPT_CHANNEL], lengths[OPT_CHANNEL],
                            hl == OPT_CHANNEL, 0);
  ssd1306_display_text_box1(&disp, 0, (lengths[OPT_CHANNEL] + 1) * FONT_COLS,
                            buffers[OPT_POWER], lengths[OPT_POWER],
                            lengths[OPT_POWER], hl == OPT_POWER, 0);
  ssd1306_display_text_box1(&disp, 1, 0, buffers[OPT_SPEED], lengths[OPT_SPEED],
                            lengths[OPT_SPEED], hl == OPT_SPEED, 0);

  lengths[OPT_TX_BTN] = snprintf(buffers[OPT_TX_BTN], MAX_BUF,
                                 system_state_get_jamming() ? "[~]" : "[T]");
  ssd1306_display_text_box1(&disp, 2, 0, buffers[OPT_TX_BTN],
                            lengths[OPT_TX_BTN], lengths[OPT_TX_BTN],
                            hl == OPT_TX_BTN, 0);
}
