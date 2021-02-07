/*
 * amp_eQEP.c
 *
 *  Created on: May 14, 2019
 *      Author: Tommy Krause
 */

#include "amp_eQEP.h"

#define AXIS_D 8
#define WHEEL_D 10
#define MOTOR_D 2
#define INCH_TO_METER 0.0254
#define PRESCALE 128
#define PI 3.14159
#define SYSCLK 200000000


//eQEP Variables
extern long     prd_count;      //number of periods of QCLK
extern float    prd_time;       //prd_count in time (secs)
extern float    motor_speed;    //angular speed of motor shaft (revs / sec)
extern float    wheel_speed;    //angular speed of rear axis (wheel) (revs / sec)
extern float    cart_speed;     //translatioinal speed of cart (meteres / sec)
extern float    spd_meas;

/* FUNCTION ---------------------------------------------------------------
 * amp_err_code_t amp_eQEP_initialize()
 *
 * Initializes the eQEP module to the specified settings
 */
amp_err_code_t amp_eQEP_initialize() {

    /*  Quadrature Decoder Control
     *      -Quadrature count mode
     *      -Count Rising and Falling edge
     *      -Not swapped
     *      -No polarity changes
     */
    EQep1Regs.QDECCTL.all = 0x0000;

    /*  Quadrature Capture Control
     *      -Enable eQEP capture unit
     *      -CAPCLK = SYSCLKOUT/128
     *      -UPEVNT = QCLK/1
     */
    EQep1Regs.QCAPCTL.all = 0x8070;

    /* QEP Control Register
     *      -FREE_SOFT = 2
     *      -Enable eQEP peripheral
     */
    EQep1Regs.QEPCTL.all = 0x8008;

    /*
     * I don't know if this will work, setting up the interrupt for direction change
     * Things to do to set up interrupts
     * -Enable in registers
     * -assign in Vector Table
     * -write ISR
     */
    //WAIT JUST READ THE CAPTURE DIRECTION FLAG IN THE QEPSTS REGISTER

    return AMP_ERROR_NONE;
}

/* FUNCTION ---------------------------------------------------------------
 * amp_err_code_t amp_eQEP_serviceSpeed()
 *
 * This function checks to see if an upevent has occured. If one has, the
 * function updates the measured speed variable
 */
amp_err_code_t amp_eQEP_serviceSpeed() {
    //Check if UPEVENT flag has been raised
    if(EQep1Regs.QEPSTS.bit.UPEVNT == 1) {
        if(EQep1Regs.QEPSTS.bit.COEF == 0) {
            //No Capture overflow, set speed normally
            prd_count = (unsigned long) EQep1Regs.QCPRD;
        } else {
            // Capture overflow has occurred, hardcode result
            prd_count = 0xFFFF;
        }

        //Calculate Speed from QCPRD value
        prd_time = prd_count * (1.0 / (SYSCLK / PRESCALE));
        motor_speed = 1 / (4 * prd_time); //Revolutions per second
        wheel_speed = ((float) MOTOR_D / AXIS_D) * motor_speed; //Revolutions per second
        cart_speed = wheel_speed * PI * INCH_TO_METER * WHEEL_D;

        spd_meas = cart_speed;

        if(spd_meas > 18)
        {
            spd_meas = 1.189;
        }

        // Clear Unit position event flag, Clear overflow error flag
        EQep1Regs.QEPSTS.all = 0x88;
    }
    return AMP_ERROR_NONE;
}
