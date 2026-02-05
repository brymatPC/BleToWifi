#include "LedStripDriver.h"
#include <utility/DebugLog.h>

#define RMT_LED_STRIP_RESOLUTION_HZ 10000000 // 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define RMT_LED_STRIP_GPIO_NUM      21

#define EXAMPLE_LED_NUMBERS         1

static uint8_t led_strip_pixels[EXAMPLE_LED_NUMBERS * 3];

static const rmt_symbol_word_t ws2812_zero = {
    .duration0 = (uint16_t) (0.3 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000), // T0H=0.3us
    .level0 = 1,
    .duration1 = (uint16_t) (0.9 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000), // T0L=0.9us
    .level1 = 0,
};

static const rmt_symbol_word_t ws2812_one = {
    .duration0 = (uint16_t) (0.9 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000), // T1H=0.9us
    .level0 = 1,
    .duration1 = (uint16_t) (0.3 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000), // T1L=0.3us
    .level1 = 0,
};

//reset defaults to 50uS
static const rmt_symbol_word_t ws2812_reset = {
    .duration0 = (uint16_t) (RMT_LED_STRIP_RESOLUTION_HZ / 1000000 * 50 / 2),
    .level0 = 1,
    .duration1 = (uint16_t) (RMT_LED_STRIP_RESOLUTION_HZ / 1000000 * 50 / 2),
    .level1 = 0,
};

static size_t encoder_callback(const void *data, size_t data_size,
                               size_t symbols_written, size_t symbols_free,
                               rmt_symbol_word_t *symbols, bool *done, void *arg)
{
    // We need a minimum of 8 symbol spaces to encode a byte. We only
    // need one to encode a reset, but it's simpler to simply demand that
    // there are 8 symbol spaces free to write anything.
    if (symbols_free < 8) {
        return 0;
    }

    // We can calculate where in the data we are from the symbol pos.
    // Alternatively, we could use some counter referenced by the arg
    // parameter to keep track of this.
    size_t data_pos = symbols_written / 8;
    uint8_t *data_bytes = (uint8_t*)data;
    if (data_pos < data_size) {
        // Encode a byte
        size_t symbol_pos = 0;
        for (int bitmask = 0x80; bitmask != 0; bitmask >>= 1) {
            if (data_bytes[data_pos]&bitmask) {
                symbols[symbol_pos++] = ws2812_one;
            } else {
                symbols[symbol_pos++] = ws2812_zero;
            }
        }
        // We're done; we should have written 8 symbols.
        return symbol_pos;
    } else {
        //All bytes already are encoded.
        //Encode the reset, and we're done.
        symbols[0] = ws2812_reset;
        *done = 1; //Indicate end of the transaction.
        return 1; //we only wrote one symbol
    }
}

LedStripDriver::LedStripDriver() {

}

LedStripDriver::~LedStripDriver() {
    
}

void LedStripDriver::setup(DebugLog *log) {
    esp_err_t err;
    m_log = log;

    rmt_tx_channel_config_t tx_chan_config = {
        .gpio_num = (gpio_num_t) RMT_LED_STRIP_GPIO_NUM,
        .clk_src = RMT_CLK_SRC_DEFAULT, // select source clock
        .resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ,
        .mem_block_symbols = 64, // increase the block size can make the LED less flickering
        .trans_queue_depth = 4, // set the number of transactions that can be pending in the background
    };
    err = rmt_new_tx_channel(&tx_chan_config, &m_ledChan);

    if(err != ESP_OK && m_log) {
        m_log->print( __FILE__, __LINE__, 1, err, "LedStripDriver::setup - new tx chan, error");
    }

    const rmt_simple_encoder_config_t simple_encoder_cfg = {
        .callback = encoder_callback
        //Note we don't set min_chunk_size here as the default of 64 is good enough.
    };
    err = rmt_new_simple_encoder(&simple_encoder_cfg, &m_encoder);

    if(err != ESP_OK && m_log) {
        m_log->print( __FILE__, __LINE__, 1, err, "LedStripDriver::setup - new encoder, error");
    }

    err = rmt_enable(m_ledChan);

    if(err != ESP_OK && m_log) {
        m_log->print( __FILE__, __LINE__, 1, err, "LedStripDriver::setup - enable, error");
    }
}

void LedStripDriver::setLed(uint32_t pixelVal)
{
    rmt_transmit_config_t tx_config = {
        .loop_count = 0, // no transfer loop
    };
    led_strip_pixels[0] = (uint8_t) (pixelVal & 0x000000FF);
    led_strip_pixels[1] = (uint8_t) ((pixelVal & 0x0000FF00) >> 8);
    led_strip_pixels[2] = (uint8_t) ((pixelVal & 0x00FF0000) >> 16);
    esp_err_t err = rmt_transmit(m_ledChan, m_encoder, led_strip_pixels, sizeof(led_strip_pixels), &tx_config);
    
    if(err != ESP_OK && m_log) {
        m_log->print( __FILE__, __LINE__, 1, err, "LedStripDriver::setLed - error");
    }

    // Flush RGB values to LEDs
    //ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));

}