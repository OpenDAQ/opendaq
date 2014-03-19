/*
 *  Copyright (C) 2012 INGEN10 Ingenieria SL
 *  http://www.ingen10.com
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.

 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Version:    140207
 *  Author:     JRB
 *  Revised by: AV (07/02/14)
 */


#include <avr/eeprom.h>
#include "calibration.h"

// Constructors ////////////////////////////////////////////////////////////////

/** \brief
 *  Calibration constructor
 *
 *  \return
 *  Calibration objetc created
 */
CalibrationClass::CalibrationClass(void) {
    Reset_calibration();

    my_id = 0;
}

/** \brief
 *  Reset the calibration
 *
 */
void CalibrationClass::Reset_calibration() {
    //DAC CALIBRATION DEFAULTS
    gain_m[0] = 1000;
    gain_b[0] = 0;

    //AIN CALIBRATION DEFAULTS
#if HW_VERSION==2
    for (int i = 1; i < NCAL_POS; i++) {
        gain_m[i] = 100;
    }
#else
    gain_m[1] = 37500;
    gain_m[2] = 12500;
    gain_m[3] = 6250;
    gain_m[4] = 1250;
    gain_m[5] = 125;
#endif

    for (int i = 1; i < NCAL_POS; i++) {
        gain_b[i] = 0;
    }

}


// Public Methods //////////////////////////////////////////////////////////////

///////ID-SERIAL NB STORAGE FUNCTIONS

/** \brief
 *  Get ID-SERIAL
 *
 *  \return
 *  ID-SERIAL readed
 */
uint32_t CalibrationClass::ID_Recall() {
    unsigned char p;
    uint32_t i32;
    int i;

    i32 = 0;

    p = read(IW_MARK);
    if (p != IW_IND)
        return 0;

    for (i = 0; i < 4; i++) {
        i32 <<= 8;
        p = read(CID_POS + i);
        i32 |= p;
    }

    return i32;
}

/** \brief
 *  Save ID-SERIAL
 *
 *  \param
 *  ID-SERIAL to write
 */
void CalibrationClass::ID_Save(uint32_t device_id) {
    unsigned char p;
    uint32_t i32;
    int i;

    i32 = device_id;

    write(IW_MARK, IW_IND);

    for (i = 0; i < 4; i++) {
        p = i32 & 0x000000FF;
        write(CID_POS + i, p);
        i32 >>= 8;
    }
}


// Calibration storage functions 

/** \brief
 *  Save calibration
 *
 */
void CalibrationClass::SaveCalibration() {
    long *p;
    uint8_t value;
    int i;

    p = (long*) gain_m;

    for (i = 0; i < NCAL_POS; i++) {
        value = p[i]&0xFF;
        write(CG0_POS + 2 * i, value);
        value = (p[i]&0xFF00) >> 8;
        write(CG0_POS + 2 * i + 1, value);
    }

    p = (long*) gain_b;

    for (i = 0; i < NCAL_POS; i++) {
        value = p[i]&0xFF;
        write(CG0_POS + OFFSET_POS + 2 * i, value);
        value = (p[i]&0xFF00) >> 8;
        write(CG0_POS + OFFSET_POS + 2 * i + 1, value);
    }

    write(CW_MARK, CW_IND);
}

/** \brief
 *  Get calibration
 *
 *  \return
 *  Calibration readed
 */
int CalibrationClass::RecallCalibration() {
    long *p;
    uint8_t value;
    int i;
    int a;

    a = read(CW_MARK);
    if (a != CW_IND) {
        Reset_calibration();
        return 0;
    }

    p = (long*) gain_m;

    for (i = 0; i < NCAL_POS; i++) {
        value = read(CG0_POS + 2 * i);
        p[i] = value;
        value = read(CG0_POS + 2 * i + 1);
        p[i] |= (value << 8);
    }

    p = (long*) gain_b;

    for (i = 0; i < NCAL_POS; i++) {
        value = read(CG0_POS + OFFSET_POS + 2 * i);
        p[i] = value;
        value = read(CG0_POS + OFFSET_POS + 2 * i + 1);
        p[i] |= (value << 8);
    }

    return a;
}



// Low level eeprom calls 

/** \brief
 *  Read data from eeprom
 *
 *  \param
 *  address: address to read the data
 *  \return
 *  Data readed
 */
uint8_t CalibrationClass::read(int address) {
    return eeprom_read_byte((unsigned char *) address);
}

/** \brief
 *  Write data from eeprom
 *
 *  \param
 *  value: data to write in eeprom
 */
void CalibrationClass::write(int address, uint8_t value) {
    eeprom_write_byte((unsigned char *) address, value);
}

CalibrationClass Cal;
