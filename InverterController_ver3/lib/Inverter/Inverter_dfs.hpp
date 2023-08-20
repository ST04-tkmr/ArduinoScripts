#ifndef _INVERTER_DFS_H_
#define _INVERTER_DFS_H_

#define MINIMUM_BATTERY_VOLTAGE (250)
#define MAXIMUM_BATTERY_VOLTAGE (400)
#define MINIMUM_INPUT_VOLTAGE (50)

// ID
#define EV_ECU1_ID (0x301)
#define MG_ECU1_ID (0x311)
#define MG_ECU2_ID (0x321)

// PWM
#define PWM_SHORT_CIRCUIT (0b00)
#define PWM_FREE_WHEEL (0b01)
#define PWM_RUN (0b10)
#define PWM_RESERVED (0b11)

// Working Status
#define WORKING_INIT (0b000)
#define WORKING_PRECHARGE (0b001)
#define WORKING_STANDBY (0b010)
#define WORKING_TORQUE_CONTROL (0b011)
#define WORKING_RAPID_DISCHARGE (0b111)

// Failure Status
#define FAILURE_NO_ERROR (0b000)
#define FAILURE_DERATING (0b001)
#define FAILURE_WARNING (0b010)
#define FAILURE_ERROR (0b100)
#define FAILURE_CRITICAL_ERROR (0b101)

#define ECU_DISABLE_MOTOR_SPEED (20) // Motor Speed 20 rpm 未満でECU実行要求OFFにしないとCritical Errorが発生する

#endif