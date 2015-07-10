#ifndef _SX9512_H
#define _SX9512_H

#include <stdint.h>

#if __BYTE_ORDER == __BIG_ENDIAN
#define BIT_ORDER_BIG_ENDIAN
#endif

#define SX9512_I2C_ADDRESS  0x2b
#define SX9512B_I2C_ADDRESS 0x2d
#define SX9512_CHANNELS 8

#define SX9512_REG_NVM_AREA_START 0x07
#define SX9512_REG_NVM_AREA_END   0x62

#define SX9512_REG_I2C_SOFT_RESET 0xff

//Name,                reserved, default value
#define SX9512_REGS \
X(IRQ_SRC,                    0, 0x00) \
X(TOUCH_STATUS,               0, 0x00) \
X(PROX_STATUS,                0, 0x00) \
X(COMP_STATUS,                0, 0x00) \
X(NVM_CTRL,                   0, 0x00) \
X(R_05,                       1, 0x00) \
X(R_06,                       1, 0x00) \
X(SPO2_MODE,                  0, 0x00) \
X(PWR_KEY,                    0, 0x00) \
X(IRQ_MASK,                   0, 0x00) \
X(R_0A,                       1, 0x00) \
X(R_0B,                       1, 0x00) \
X(LED_MAP1,                   0, 0x00) \
X(LED_MAP2,                   0, 0x00) \
X(LED_PWM_FREQ,               0, 0x00) \
X(LED_MODE,                   0, 0x00) \
X(LED_IDLE,                   0, 0x00) \
X(LED_OFF_DELAY,              0, 0x00) \
X(LED1_ON,                    0, 0x00) \
X(LED1_FADE,                  0, 0x00) \
X(LED2_ON,                    0, 0x00) \
X(LED2_FADE,                  0, 0x00) \
X(LED_PWR_IDLE,               0, 0x00) \
X(LED_PWR_ON,                 0, 0x00) \
X(LED_PWR_OFF,                0, 0x00) \
X(LED_PWR_FADE,               0, 0x00) \
X(LED_PWR_ON_PW,              0, 0x00) \
X(LED_PWR_MODE,               0, 0x00) \
X(R_1C,                       1, 0x00) \
X(R_1D,                       1, 0x00) \
X(CAP_SENSE_ENABLE,           0, 0x00) \
X(CAP_SENSE_RANGE0,           0, 0x00) \
X(CAP_SENSE_RANGE1,           0, 0x00) \
X(CAP_SENSE_RANGE2,           0, 0x00) \
X(CAP_SENSE_RANGE3,           0, 0x00) \
X(CAP_SENSE_RANGE4,           0, 0x00) \
X(CAP_SENSE_RANGE5,           0, 0x00) \
X(CAP_SENSE_RANGE6,           0, 0x00) \
X(CAP_SENSE_RANGE7,           0, 0x00) \
X(CAP_SENSE_RANGE_ALL,        0, 0x00) \
X(CAP_SENSE_THRESH0,          0, 0x00) \
X(CAP_SENSE_THRESH1,          0, 0x00) \
X(CAP_SENSE_THRESH2,          0, 0x00) \
X(CAP_SENSE_THRESH3,          0, 0x00) \
X(CAP_SENSE_THRESH4,          0, 0x00) \
X(CAP_SENSE_THRESH5,          0, 0x00) \
X(CAP_SENSE_THRESH6,          0, 0x00) \
X(CAP_SENSE_THRESH7,          0, 0x00) \
X(CAP_SENSE_THRESH_COMB,      0, 0x00) \
X(CAP_SENSE_OP,               0, 0x00) \
X(CAP_SENSE_MODE,             0, 0x00) \
X(CAP_SENSE_DEBOUNCE,         0, 0x00) \
X(CAP_SENSE_NEG_COMP_THRESH,  0, 0x00) \
X(CAP_SENSE_POS_COMP_THRESH,  0, 0x00) \
X(CAP_SENSE_POS_FILT,         0, 0x00) \
X(CAP_SENSE_NEG_FILT,         0, 0x00) \
X(CAP_SENSE_STUCK,            0, 0x00) \
X(CAP_SENSE_FRAME_SKIP,       0, 0x00) \
X(CAP_SENSE_MISC,             0, 0x00) \
X(PROX_COMB_CHAN_MASK,        0, 0x00) \
X(R_3C,                       1, 0x00) \
X(R_3D,                       1, 0x00) \
X(SPO_CHAN_MAP,               0, 0x00) \
X(SPO_LEVEL_BL0,              0, 0x00) \
X(SPO_LEVEL_BL1,              0, 0x00) \
X(SPO_LEVEL_BL2,              0, 0x00) \
X(SPO_LEVEL_BL3,              0, 0x00) \
X(SPO_LEVEL_BL4,              0, 0x00) \
X(SPO_LEVEL_BL5,              0, 0x00) \
X(SPO_LEVEL_BL6,              0, 0x00) \
X(SPO_LEVEL_BL7,              0, 0x00) \
X(SPO_LEVEL_IDLE,             0, 0x00) \
X(SPO_LEVEL_PROX,             0, 0x00) \
X(R_49,                       1, 0x00) \
X(R_4A,                       1, 0x00) \
X(BUZZER_TRIGGER,             0, 0x00) \
X(BUZZER_FREQ,                0, 0x00) \
X(R_4D,                       1, 0x00) \
X(R_4E,                       1, 0x00) \
X(R_4F,                       1, 0x00) \
X(R_50,                       1, 0x00) \
X(R_51,                       1, 0x00) \
X(R_52,                       1, 0x00) \
X(R_53,                       1, 0x00) \
X(R_54,                       1, 0x00) \
X(R_55,                       1, 0x00) \
X(R_56,                       1, 0x00) \
X(R_57,                       1, 0x00) \
X(R_58,                       1, 0x00) \
X(R_59,                       1, 0x00) \
X(R_5A,                       1, 0x00) \
X(R_5B,                       1, 0x00) \
X(R_5C,                       1, 0x00) \
X(R_5D,                       1, 0x00) \
X(R_5E,                       1, 0x00) \
X(R_5F,                       1, 0x00) \
X(R_60,                       1, 0x00) \
X(R_61,                       1, 0x00) \
X(CAP_SENSE_CHAN_SELECT,      0, 0x00) \
X(CAP_SENSE_USEFUL_DATA_MSB,  0, 0x00) \
X(CAP_SENSE_USEFUL_DATA_LSB,  0, 0x00) \
X(CAP_SENSE_AVERAGE_DATA_MSB, 0, 0x00) \
X(CAP_SENSE_AVERAGE_DATA_LSB, 0, 0x00) \
X(CAP_SENSE_DIFF_DATA_MSB,    0, 0x00) \
X(CAP_SENSE_DIFF_DATA_LSB,    0, 0x00) \
X(CAP_SENSE_COMP_MSB,         0, 0x00) \
X(CAP_SENSE_COMP_LSB,         0, 0x00)


#define X(name, reserved, default) SX9512_REG_##name,
typedef enum { SX9512_REGS SX9512_REGS_AMOUNT } sx9512_reg_t;
#undef X


struct sx9512_reg_data {
	const char *name;
	uint8_t reserved, default_value;
};


//! Interrupt source

//! The Irq Source register will indicate that the specified event has occurred since the last read of this register. If the
//! NIRQ function is selected for SPO2 then it will indicate the occurrence of any of these events that are not masked
//! out in register 0x09.
//! The Irq mask in register 0x09 will prevent an Irq from being indicated by the NIRQ pin but it will not prevent the
//! IRQ from being noted in this register.
struct sx9512_reg_irq_src {
#ifdef BIT_ORDER_BIG_ENDIAN
	uint8_t reset:1;
	uint8_t touch:1;
	uint8_t release:1;
	uint8_t prox_near:1;         //!< proximity on
	uint8_t prox_far:1;          //!< proximity off
	uint8_t compensation_done:1; //!< write 1 to trigger compensation on all channels
	uint8_t conversion_done:1;
	uint8_t :1;
#else
	uint8_t :1;
	uint8_t conversion_done:1;
	uint8_t compensation_done:1; //!< write 1 to trigger compensation on all channels
	uint8_t prox_far:1;          //!< proximity off
	uint8_t prox_near:1;         //!< proximity on
	uint8_t release:1;
	uint8_t touch:1;
	uint8_t reset:1;
#endif
} __attribute__((__packed__));


//! Proximity status
struct sx9512_reg_prox_status {
#ifdef BIT_ORDER_BIG_ENDIAN
	uint8_t prox_bl0:1;                //!< proximity detected on BL0
	uint8_t prox_multi:1;              //!< proximity detected on combined channels
	uint8_t prox_multi_comp_pending:1; //!< compensation pending for combined channel prox sensing
	uint8_t :5;
#else
	uint8_t :5;
	uint8_t prox_multi_comp_pending:1; //!< compensation pending for combined channel prox sensing
	uint8_t prox_multi:1;              //!< proximity detected on combined channels
	uint8_t prox_bl0:1;                //!< proximity detected on BL0
#endif
} __attribute__((__packed__));


//! NVM Control

//! The NVM Area field indicates which of the user NVM areas are currently programmed and active (1, 2 or 3). The
//! NVM Read bit gives the ability to manually request that the contents of the NVM be transferred to the registers
//! and NVM Burn field gives the ability to burn the current registers to the next available NVM area.
//! Normally, the transfer of data from the NVM to the registers is done automatically on power up and upon a reset
//! but occasionally a user might want to force a read manually.
//! Registers 0x07 through 0x62 are stored to NVM and loaded from NVM.
//! After writing 0xA0 (ie NVM burn), wait for at least 500ms and then power off-on the IC for changes to be effective.
//! Caution, there are only three user areas and attempts to burn values beyond user area 3 will be ignored. 
struct sx9512_reg_nvm_ctrl {
#ifdef BIT_ORDER_BIG_ENDIAN
	uint8_t nvm_burn:4; //!< write 0x50 followed by 0xA0 to initiate transfer of reg 0x07-0x62 to NVM
	uint8_t nvm_read:1; //!< trigger NVM read to registers
	uint8_t nvm_area:3; //!< indicates current active NVM area
#else
	uint8_t nvm_area:3; //!< indicates current active NVM area
	uint8_t nvm_read:1; //!< trigger NVM read to registers
	uint8_t nvm_burn:4; //!< write 0x50 followed by 0xA0 to initiate transfer of reg 0x07-0x62 to NVM
#endif
} __attribute__((__packed__));


//! SPO2 Mode Control

//! The SPO2 Config field will specify the functionality of the SPO pin. When selected as NIRQ, the open drain output
//! will go low whenever a non-masked Irq occurs and the NIRQ will go back high after a register 0x00 is read over
//! the I2C. When selected as Buzzer, the SPO2 pin will drive a 2 phase 2 frequency signal onto an external buzzer
//! for each specified event (see Buzzer section). When selected as SPO2, pin operates as an analog output similar
//! to SPO1 (see SPO section). If selected as TV power state, the pin is driven from the system PMIC with a high
//! (SPO2 = SVDD) indicating that the system power is on and a low (SPO2 = GND) when the system power is off.
//! The TV Power State bit reads back the current state of SPO2 if SPO2 is selected for TV power state, otherwise
//! the system should write to this bit to indicate the current system power state. The SX9512/12B/13/13B needs to
//! know the current state in able to correctly process some of the LED modes for the Power Button (see LED
//! modes). 
struct sx9512_reg_spo2_mode {
	#ifdef BIT_ORDER_BIG_ENDIAN
	uint8_t :1;
	uint8_t spo2_config:2;    //!< set function of SPO2 pin
	uint8_t tv_power_state:1; //!< if SPO2 set to TV power state input then TV power state indicated by this bit.
	uint8_t :4;
#else
	uint8_t :4;
	uint8_t tv_power_state:1; //!< if SPO2 set to TV power state input then TV power state indicated by this bit.
	uint8_t spo2_config:2;    //!< set function of SPO2 pin
	uint8_t :1;
#endif
} __attribute__((__packed__));


//! Interrupt Request Mask

//! Set which Irqs will trigger an NIRQ (if enabled on SPO2) and report in reg 0x00
//! 0=Disable IRQ
//! 1=Enable IRQ
struct sx9512_reg_irq_mask {
#ifdef BIT_ORDER_BIG_ENDIAN
	uint8_t reset:1;
	uint8_t touch:1;
	uint8_t release:1;
	uint8_t prox_near:1; //!< proximity on
	uint8_t prox_far:1;  //!< proximity off
	uint8_t compensation_done:1;
	uint8_t :2;
#else
	uint8_t :2;
	uint8_t compensation_done:1;
	uint8_t prox_far:1;  //!< proximity off
	uint8_t prox_near:1; //!< proximity on
	uint8_t release:1;
	uint8_t touch:1;
	uint8_t reset:1;
#endif
} __attribute__((__packed__));


//! LED Mode
struct sx9512_reg_led_mode {
#ifdef BIT_ORDER_BIG_ENDIAN
	uint8_t led_fade_repeat:4;
	uint8_t :1;
	uint8_t led_fading:1;
	uint8_t led_mode:2;
#else
	uint8_t led_mode:2;
	uint8_t led_fading:1;
	uint8_t :1;
	uint8_t led_fade_repeat:4;
#endif
} __attribute__((__packed__));


//! LED off delay
struct sx9512_reg_led_off_delay {
#ifdef BIT_ORDER_BIG_ENDIAN
	uint8_t led_engine1_delay_off_time:4;
	uint8_t led_engine2_delay_off_time:4;
#else
	uint8_t led_engine2_delay_off_time:4;
	uint8_t led_engine1_delay_off_time:4;
#endif
} __attribute__((__packed__));


//! LED Engine fade in/out timing
struct sx9512_reg_led_fade {
#ifdef BIT_ORDER_BIG_ENDIAN
	uint8_t led_engine_fade_in_time:4;
	uint8_t led_engine_fade_out_time:4;
#else
	uint8_t led_engine_fade_out_time:4;
	uint8_t led_engine_fade_in_time:4;
#endif
} __attribute__((__packed__));


//! LED power button mode
struct sx9512_reg_led_pwr_mode {
#ifdef BIT_ORDER_BIG_ENDIAN
	uint8_t power_led_off_mode:1;
	uint8_t power_led_max_level:1;
	uint8_t power_led_breath_max:1;
	uint8_t power_led_waveform:1;
	uint8_t :2;
	uint8_t power_button_enable:1;
	uint8_t led_touch_poarity_invert:1;
#else
	uint8_t led_touch_poarity_invert:1;
	uint8_t power_button_enable:1;
	uint8_t :2;
	uint8_t power_led_waveform:1;
	uint8_t power_led_breath_max:1;
	uint8_t power_led_max_level:1;
	uint8_t power_led_off_mode:1;
#endif
} __attribute__((__packed__));


//! CapSense delta Cin range and LS control
struct sx9512_reg_cap_sense_range {
#ifdef BIT_ORDER_BIG_ENDIAN
	uint8_t ls_control:2;
	uint8_t :4;
	uint8_t delta_cin_range:2;
#else
	uint8_t delta_cin_range:2;
	uint8_t :4;
	uint8_t ls_control:2;
#endif
} __attribute__((__packed__));


//! CapSense auto compensation, proximity on BL0 and combined channel proximity
struct sx9512_reg_cap_sense_op {
#ifdef BIT_ORDER_BIG_ENDIAN
	uint8_t auto_compensation:1;
	uint8_t proximity_bl0:1;
	uint8_t proximity_combined_channels:1;
	uint8_t set_to_0x14:5;
#else
	uint8_t set_to_0x14:5;
	uint8_t proximity_combined_channels:1;
	uint8_t proximity_bl0:1;
	uint8_t auto_compensation:1;
#endif
} __attribute__((__packed__));


//! CapSense raw data filter coef, digital gain, I2C touch reporting and CapSense
struct sx9512_reg_cap_sense_mode {
#ifdef BIT_ORDER_BIG_ENDIAN
	uint8_t raw_filter:3;
	uint8_t touch_reporting:1;
	uint8_t cap_sense_digital_gain:2;
	uint8_t cap_sense_report_mode:2;
#else
	uint8_t cap_sense_report_mode:2;
	uint8_t cap_sense_digital_gain:2;
	uint8_t touch_reporting:1;
	uint8_t raw_filter:3;
#endif
} __attribute__((__packed__));


//! CapSense debounce
struct sx9512_reg_cap_sense_debounce {
#ifdef BIT_ORDER_BIG_ENDIAN
	uint8_t cap_sense_prox_near_debounce:2;
	uint8_t cap_sense_prox_far_debounce:2;
	uint8_t cap_sense_touch_debounce:2;
	uint8_t cap_sense_release_debounce:2;
#else
	uint8_t cap_sense_release_debounce:2;
	uint8_t cap_sense_touch_debounce:2;
	uint8_t cap_sense_prox_far_debounce:2;
	uint8_t cap_sense_prox_near_debounce:2;
#endif
} __attribute__((__packed__));


//! CapSense positive filter coef, positive auto compensation debounce and proximity hyst
struct sx9512_reg_cap_sense_pos_filt {
#ifdef BIT_ORDER_BIG_ENDIAN
	uint8_t cap_sense_prox_hyst:3;
	uint8_t cap_sense_pos_comp_debounce:2;
	uint8_t cap_sense_avg_pos_filt_coef:3;
#else
	uint8_t cap_sense_avg_pos_filt_coef:3;
	uint8_t cap_sense_pos_comp_debounce:2;
	uint8_t cap_sense_prox_hyst:3;
#endif
} __attribute__((__packed__));


//! CapSense negative filter coef, negative auto compensation debounce and touch hyst
struct sx9512_reg_cap_sense_neg_filt {
#ifdef BIT_ORDER_BIG_ENDIAN
	uint8_t cap_sense_touch_hyst:3;
	uint8_t cap_sense_neg_comp_debounce:2;
	uint8_t cap_sense_avg_neg_filt_coef:3;
#else
	uint8_t cap_sense_avg_neg_filt_coef:3;
	uint8_t cap_sense_neg_comp_debounce:2;
	uint8_t cap_sense_touch_hyst:3;
#endif
} __attribute__((__packed__));


//! CapSense stuck-at timer and periodic compensation timer
struct sx9512_reg_cap_sense_stuck {
#ifdef BIT_ORDER_BIG_ENDIAN
	uint8_t cap_sense_stuck_at_timer:4;
	uint8_t cap_sense_periodic_comp:4;
#else
	uint8_t cap_sense_periodic_comp:4;
	uint8_t cap_sense_stuck_at_timer:4;
#endif
} __attribute__((__packed__));


//! CapSense frame skip setting from active and sleep
struct sx9512_reg_cap_sense_frame_skip {
#ifdef BIT_ORDER_BIG_ENDIAN
	uint8_t cap_sense_active_frame_skip:4;
	uint8_t cap_sense_sleep_frame_skip:4;
#else
	uint8_t cap_sense_sleep_frame_skip:4;
	uint8_t cap_sense_active_frame_skip:4;
#endif
} __attribute__((__packed__));


//! CapSense sleep enable, auto compensation channels threshold, inactive BL control
struct sx9512_reg_cap_sense_misc {
#ifdef BIT_ORDER_BIG_ENDIAN
	uint8_t :2;
	uint8_t comp_chan_num_thresh:2;
	uint8_t cap_sense_sleep_mode_enable:1;
	uint8_t :1;
	uint8_t cap_sense_inactive_bl_mode:2;
#else
	uint8_t cap_sense_inactive_bl_mode:2;
	uint8_t :1;
	uint8_t cap_sense_sleep_mode_enable:1;
	uint8_t comp_chan_num_thresh:2;
	uint8_t :2;
#endif
} __attribute__((__packed__));


//! SPO analog output for proximity
struct sx9512_reg_spo_level_prox {
#ifdef BIT_ORDER_BIG_ENDIAN
	uint8_t spo_report_prox:1;
	uint8_t spo_prox_channel_mapping:1;
	uint8_t spo_level_prox:6;
#else
	uint8_t spo_level_prox:6;
	uint8_t spo_prox_channel_mapping:1;
	uint8_t spo_report_prox:1;
#endif
} __attribute__((__packed__));


//! Buzzer trigger event selection
struct sx9512_reg_buzzer_trigger {
#ifdef BIT_ORDER_BIG_ENDIAN
	uint8_t :3;
	uint8_t buzzer_near:1;
	uint8_t buzzer_far:1;
	uint8_t buzzer_touch:1;
	uint8_t buzzer_release:1;
	uint8_t buzzer_idle_level:1;
#else
	uint8_t buzzer_idle_level:1;
	uint8_t buzzer_release:1;
	uint8_t buzzer_touch:1;
	uint8_t buzzer_far:1;
	uint8_t buzzer_near:1;
	uint8_t :3;
#endif
} __attribute__((__packed__));


//! Buzzer duration frequency
struct sx9512_reg_buzzer_freq {
#ifdef BIT_ORDER_BIG_ENDIAN
	uint8_t buzzer_phase1_duration:2;
	uint8_t buzzer_phase1_freq:2;
	uint8_t buzzer_phase2_duration:2;
	uint8_t buzzer_phase2_freq:2;
#else
	uint8_t buzzer_phase2_freq:2;
	uint8_t buzzer_phase2_duration:2;
	uint8_t buzzer_phase1_freq:2;
	uint8_t buzzer_phase1_duration:2;
#endif
} __attribute__((__packed__));


struct sx9512_reg_nvm {
	struct sx9512_reg_spo2_mode       spo2_mode;            //!< SPO2 mode control
	uint8_t                           pwr_key;              //!< Power key control
	struct sx9512_reg_irq_mask        irq_mask;             //!< Interrupt request mask
	uint8_t                           reserved_0a[2];
	uint8_t                           led_map[2];           //!< LED map for engine 1/2
	uint8_t                           led_pwm_freq;         //!< LED PWM frequency
	struct sx9512_reg_led_mode        led_mode;             //!< LED mode
	uint8_t                           led_idle;             //!< LED idle level
	struct sx9512_reg_led_off_delay   led_off_delay;        //!< LED off delay
	uint8_t                           led1_on;              //!< LED engine 1 on level
	struct sx9512_reg_led_fade        led1_fade;            //!< LED engine 1 fade in/out time
	uint8_t                           led2_on;              //!< LED engine 2 on level
	struct sx9512_reg_led_fade        led2_fade;            //!< LED engine 2 fade in/out time
	uint8_t                           led_pwr_idle;         //!< LED power button idle level
	uint8_t                           led_pwr_on;           //!< LED power button on level
	uint8_t                           led_pwr_off;          //!< LED power button off level
	uint8_t                           led_pwr_fade;         //!< LED power button fade in/out time
	uint8_t                           led_pwr_on_pw;        //!< LED power on pulse width
	struct sx9512_reg_led_pwr_mode    led_pwr_mode;         //!< LED power button mode
	uint8_t                           reserved_1c[2];
	uint8_t                           cap_sense_enable;     //!< CapSense enable
	struct sx9512_reg_cap_sense_range cap_sense_range[9];   //!< CapSense delta Cin range and LS control chan 0-7
	//struct sx9512_reg_cap_sense_range cap_sense_range_all;  //!< CapSense delta Cin range and LS control for prox sensor (combined channels)
	uint8_t                           cap_sense_thresh[9];  //!< CapSense detection threshold for chan 0-7
	//uint8_t                           cap_sense_thesh_comb; //!< CapSense detection threshold for prox sensor (combined channels)
	struct sx9512_reg_cap_sense_op    cap_sense_op;         //!< CapSense auto compensation, proximity on BL0 and combined channel proximity enable
	struct sx9512_reg_cap_sense_mode  cap_sense_mode;       //!< CapSense raw data filter coef, digital gain, I2C touch reporting and CapSense reporting
	struct sx9512_reg_cap_sense_debounce cap_sense_debounce; //!< CapSense debounce
	uint8_t                           cap_sense_neg_comp_thresh; //!< CapSense negative auto compensation threshold
	uint8_t                           cap_sense_pos_comp_thresh; //!< CapSense positive auto compensation threshold
	struct sx9512_reg_cap_sense_pos_filt cap_sense_pos_filt; //!< CapSense positive filter coef, positive auto compensation debounce and proximity hyst
	struct sx9512_reg_cap_sense_neg_filt cap_sense_neg_filt; //!< CapSense negative filter coef, negative auto compensation debounce and touch hyst
	struct sx9512_reg_cap_sense_stuck cap_sense_stuck;      //!< CapSense stuck-at timer and periodic compensation timer
	struct sx9512_reg_cap_sense_frame_skip cap_sense_frame_skip; //!< CapSense frame skip setting from active and sleep
	struct sx9512_reg_cap_sense_misc  cap_sense_misc;       //!< CapSense sleep enable, auto compensation channels threshold, inactive BL control
	uint8_t                           prox_comb_chan_mask;  //!< Proximity combined channel mode channel mapping
	uint8_t                           reserved_3c[2];
	uint8_t                           spo_chan_map;         //!< SPO channel mapping
	uint8_t                           spo_level_bl[8];      //!< SPO analog output levels
	uint8_t                           spo_level_idle;       //!< SPO analog output level for idle
	struct sx9512_reg_spo_level_prox  spo_level_prox;       //!< SPO analog output for proximity
	uint8_t                           reserved_49[2];
	struct sx9512_reg_buzzer_trigger  buzzer_trigger;       //!< Buzzer trigger event selection
	struct sx9512_reg_buzzer_freq     buzzer_freq;          //!< Buzzer duration frequency
} __attribute__((__packed__));


struct sx9512_reg {
	struct sx9512_reg_irq_src     irq_src;                //!< Interrupt source
	uint8_t                       touch_status;           //!< Touch status
	struct sx9512_reg_prox_status prox_status;            //!< Proximity status
	uint8_t                       comp_status;            //!< Compensation status
	struct sx9512_reg_nvm_ctrl    nvm_ctrl;               //!< NVM control
	struct sx9512_reg_nvm         nvm;                    //!< Non-volatile memory
	uint8_t                       cap_sense_chan_select;  //!< CapSense channel select for readback
	uint16_t                      cap_sense_useful_data;  //!< CapSense useful data
	uint16_t                      cap_sense_average_data; //!< CapSense average data
	uint16_t                      cap_sense_diff_data;    //!< CapSense diff data
	uint16_t                      cap_sense_comp;         //!< CapSense compensation DAC value
} __attribute__((__packed__));


struct sx9512_reg_status {
	struct sx9512_reg_irq_src     irq_src;                //!< Interrupt source
	uint8_t                       touch_status;           //!< Touch status
	struct sx9512_reg_prox_status prox_status;            //!< Proximity status
} __attribute__((__packed__));


struct sx9512_touch_state {
	uint8_t state;
	uint8_t touched;
	uint8_t released;
};


int sx9512_init(const char *dev, int addr, struct sx9512_reg_nvm *nvm);
const char *sx9512_reg_name(sx9512_reg_t reg);
int sx9512_reg_reserved(sx9512_reg_t reg);
int sx9512_reset(int fd);
int sx9512_reset_restore_led_state(int fd);
int sx9512_read_interrupt(int fd);
int sx9512_read_buttons(int fd);
int sx9512_read_proximity(int fd);
int sx9512_read_status(int fd, struct sx9512_reg_status *status);
int sx9512_read_status_cached(int fd, struct sx9512_touch_state *touch_state);
int sx9512_reg_nvm_load(int fd);
int sx9512_reg_nvm_store(int fd);
int sx9512_reg_nvm_read(int fd, struct sx9512_reg_nvm *p);
int sx9512_reg_nvm_write(int fd, struct sx9512_reg_nvm *p);
void sx9512_reg_nvm_init(struct sx9512_reg_nvm *p);
void sx9512_reg_nvm_init_defaults(struct sx9512_reg_nvm *p, uint8_t capsense_channels, uint8_t led_channels);
void sx9512_reg_nvm_init_cg300(struct sx9512_reg_nvm *p);
void sx9512_reg_nvm_init_cg301(struct sx9512_reg_nvm *p);
void sx9512_reg_nvm_init_eg300(struct sx9512_reg_nvm *p);
void sx9512_reg_nvm_init_dg400(struct sx9512_reg_nvm *p);

#endif
