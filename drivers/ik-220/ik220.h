/********************************************************************
* ik220.h                                                           *
*********************************************************************
* Kernel-Module for Heidenhain IK220-Card                           *
*                                                                   *
* Author: HEIDENHAIN /                                              *
*********************************************************************/

#ifndef IK220_H
#define IK220_H

#define DRV_NAME        "ik220"
#define DRV_VERSION     "0.20"

#define IK220_VENDOR_ID    0x10B5
#define IK220_DEVICE_ID    0x9050
#define IK220_SUBVENDOR_ID IK220_VENDOR_ID
#define IK220_SUBDEVICE_ID 0x1172

#define IK220_DRV_NAME         DRV_NAME
#define IK220_DRIVER_VERSION   DRV_VERSION

#include "ik220_if.h"

#endif
