#ifndef _INVERTER_DFS_H_
#define _INVERTER_DFS_H_

#define ON (1)
#define OFF (0)

#define BATTERY_VOLTAGE (270)
#define MINIMUM_INPUT_VOLTAGE (50)

// ID
#define EV_ECU1_ID (0x301)
#define MG_ECU1_ID (0x311)
#define MG_ECU2_ID (0x321)

// PWM
#define SHORT_CIRCUIT (0b000)
#define FREE_WHEEL (0b001)
#define PWM_RUN (0b010)

// Working Status
#define INIT (0b000)
#define PRECHARGE (0b001)
#define STANDBY (0b010)
#define TORQUE_CONTROL (0b011)
#define RAPID_DISCHARGE (0b111)

// Failure Status
#define NO_ERROR (0b000)
#define DERATING (0b001)
#define WARNING (0b010)
#define ERROR (0b100)
#define CRITICAL_ERROR (0b101)

#define ECU_DISABLE_MOTOR_SPEED (20) // Motor Speed 20 rpm 未満でECU実行要求OFFにしないとCritical Errorが発生する

#endif