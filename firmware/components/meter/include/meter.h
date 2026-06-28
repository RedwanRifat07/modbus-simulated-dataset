/* SPDX-License-Identifier: MIT */
/* Public API for all field-device readers and simulators.
 * Each device: one _sample_t struct + meter_read_*() + meter_sim_*(). */
#pragma once

#include <stdint.h>
#include "driver/uart.h"
#include "esp_err.h"

/* ----------------------------------------------------------------- MFM384 */
/* Eastron SDM384 — 3-phase multifunction energy meter (style exemplar).
 * Registers: FC04 input regs, float32 CDAB (low-word first).               */
typedef struct {
    float v1, v2, v3;          /* Phase-N voltages [V]      */
    float i1, i2, i3;          /* Phase currents [A]        */
    float p1, p2, p3;          /* Active power per phase [kW] */
    float total_kwh;           /* Import active energy [kWh]  */
    float total_kvarh;         /* Import reactive energy [kvarh] */
    float freq;                /* System frequency [Hz]      */
    float avg_pf;              /* Average power factor       */
} mfm384_sample_t;

esp_err_t meter_read_mfm384(uart_port_t port, uint8_t unit, mfm384_sample_t *out);
void      meter_sim_mfm384(uint32_t tick, mfm384_sample_t *out);

/* ----------------------------------------------------------------- EEM400 */
/* Honeywell EEM400C-D-MO — 3-phase energy meter with Modbus RTU.
 * Bus: 19200 baud, 8E1, FC03 (holding registers), default slave 0x01.
 * Source: honeywell_data_read_only_with_rak4630.ino + datasheet P+P26/595.
 *
 * All fields are engineering units decoded from raw registers:
 *   Energy : kWh   (0.1 kWh resolution — meter multiplier 10^-1)
 *   Voltage: V     (1 V   resolution — raw register = volts directly)
 *   Current: A     (0.1 A resolution — assumes 5/5 CT; see conflict note)
 *   Power  : kW/kvar (0.1 kW resolution — datasheet multiplier 10^-1)
 *   PF     : –     (0.01 resolution — datasheet multiplier 10^-2)         */
typedef struct {
    float t1_total_kwh;  /* Tariff-1 total energy [kWh]    R28-29, ×0.1   */
    float t1_part_kwh;   /* Tariff-1 partial energy [kWh]  R30-31, ×0.1   */
    float t2_total_kwh;  /* Tariff-2 total energy [kWh]    R32-33, ×0.1   */
    float v1;            /* Phase-1 RMS voltage [V]         R36             */
    float v2;            /* Phase-2 RMS voltage [V]         R41             */
    float v3;            /* Phase-3 RMS voltage [V]         R46             */
    float i1;            /* Phase-1 RMS current [A]         R37, ×0.1      */
    float i2;            /* Phase-2 RMS current [A]         R42, ×0.1      */
    float i3;            /* Phase-3 RMS current [A]         R47, ×0.1      */
    float p1;            /* Phase-1 active power [kW]       R38, ×0.1      */
    float p2;            /* Phase-2 active power [kW]       R43, ×0.1      */
    float p3;            /* Phase-3 active power [kW]       R48, ×0.1      */
    float q1;            /* Phase-1 reactive power [kvar]   R39, ×0.1      */
    float q2;            /* Phase-2 reactive power [kvar]   R44, ×0.1      */
    float q3;            /* Phase-3 reactive power [kvar]   R49, ×0.1      */
    float p_total;       /* Total active power [kW]         R51, ×0.1      */
    float q_total;       /* Total reactive power [kvar]     R52, ×0.1      */
    float cos1;          /* Power factor phase 1            R40, ×0.01     */
    float cos2;          /* Power factor phase 2            R45, ×0.01     */
    float cos3;          /* Power factor phase 3            R50, ×0.01     */
} eem400_sample_t;

esp_err_t meter_read_eem400(uart_port_t port, uint8_t unit, eem400_sample_t *out);
void      meter_sim_eem400(uint32_t tick, eem400_sample_t *out);

/* ------------------------------------------------------------------- DSE */
/* Deep Sea Electronics generator controller (e.g. DSE7320 / DSE8610).
 * Bus: 115200 baud, 8N2, FC03 (holding registers), slave configurable.
 * Source: Deep-Sea-code.ino + datasheet (deep-sea-register-addredss.pdf).
 *
 * NOTE: Arduino register labels are wrong for many voltage channels
 * (Mains↔Generator mixups, wrong phase numbers).  Register ADDRESSES are
 * correct and proven.  Field names below follow the datasheet.
 *
 * All fields are engineering units:
 *   Fuel      : %    (1 % resolution, raw = %)
 *   Voltage   : V    (0.1 V resolution, raw × 0.1 = V)
 *   Current   : —    (no current registers in Arduino code)
 *   Frequency : Hz   (0.1 Hz resolution)
 *   Power     : W    (1 W resolution, signed — generator can export/import)
 *   PF        : —    (0.1 resolution, signed: –1.0 … +1.0)
 *   Time      : s    (1 s resolution)                                      */
typedef struct {
    float fuel_pct;       /* Fuel level [%]              R1027, ×1         */
    float batt_v;         /* Engine battery voltage [V]  R1029, ×0.1       */
    float engine_rpm;     /* Engine speed [RPM]          R1030, ×1         */
    float gen_freq_hz;    /* Generator frequency [Hz]    R1031, ×0.1       */
    float gen_l1n_v;      /* Generator L1-N voltage [V]  R1032-33, ×0.1   */
    float gen_l2n_v;      /* Generator L2-N voltage [V]  R1034-35, ×0.1   */
    float gen_l3n_v;      /* Generator L3-N voltage [V]  R1036-37, ×0.1   */
    float gen_l1l2_v;     /* Generator L1-L2 voltage [V] R1038-39, ×0.1   */
    float gen_l2l3_v;     /* Generator L2-L3 voltage [V] R1040-41, ×0.1   */
    float gen_l3l1_v;     /* Generator L3-L1 voltage [V] R1042-43, ×0.1   */
    float mains_freq_hz;  /* Mains frequency [Hz]        R1059, ×0.1       */
    float mains_l1n_v;    /* Mains L1-N voltage [V]      R1060-61, ×0.1   */
    float mains_l2n_v;    /* Mains L2-N voltage [V]      R1062-63, ×0.1   */
    float mains_l3n_v;    /* Mains L3-N voltage [V]      R1064-65, ×0.1   */
    float mains_l1l2_v;   /* Mains L1-L2 voltage [V]     R1066-67, ×0.1   */
    float mains_l2l3_v;   /* Mains L2-L3 voltage [V]     R1068-69, ×0.1   */
    float mains_l3l1_v;   /* Mains L3-L1 voltage [V]     R1070-71, ×0.1   */
    float gen_total_w;    /* Generator total watts [W]   R1536-37, signed  */
    float gen_avg_pf;     /* Generator avg PF            R1557, ×0.1 signed*/
    float gen_pct_power;  /* Generator % of full power   R1630, ×0.1 signed*/
    float engine_run_s;   /* Engine run time [s]         R1798-99, ×1      */
} dse_sample_t;

esp_err_t meter_read_dse(uart_port_t port, uint8_t unit, dse_sample_t *out);
void      meter_sim_dse(uint32_t tick, dse_sample_t *out);
