#ifndef IK220_IF_H
#define IK220_IF_H

#define IK220_IOCTL_TYPE      0xCE

/* IK220-IOCTLs */

#define IK220_IOCTL_INPUT     0x01
#define IK220_IOCTL_OUTPUT    0x02
#define IK220_IOCTL_STATUS    0x03
#define IK220_IOCTL_DOWNLOAD  0x04
#define IK220_IOCTL_UPLOAD    0x05
#define IK220_IOCTL_BURSTRAM  0x06
#define IK220_IOCTL_VERSION   0x07
#define IK220_IOCTL_LATCHI    0x08
#define IK220_IOCTL_LATCHE    0x09
#define IK220_IOCTL_SYSLED    0x0a

#define IK220_MAX_IOCTL       0x0a

#define IK220_MAX_CARDS   8
#define IK220_MAX_AXIS    16

#define IK220_MAX_INPUT_SIZE  16
#define IK220_MAX_OUTPUT_SIZE 16
#define IK220_MAX_PROG_SIZE   (65536 * 2)

#define IK220_VERSION_SIZE  33
#define IK220_LIB_VERSION   "0.20"

#define IK220_DEVNAME       "/dev/ik220"

#define IK220_G28REGISTER_0	(uint16_t) 0x0000
#define IK220_G28REGISTER_1	(uint16_t) 0x0001
#define IK220_G28REGISTER_2	(uint16_t) 0x0002
#define IK220_G28REGISTER_3	(uint16_t) 0x0003
#define IK220_G28REGISTER_4	(uint16_t) 0x0004
#define IK220_G28REGISTER_5	(uint16_t) 0x0005
#define IK220_G28REGISTER_6	(uint16_t) 0x0006
#define IK220_G28REGISTER_7	(uint16_t) 0x0007
#define IK220_G28REGISTER_8	(uint16_t) 0x0008
#define IK220_G28REGISTER_9	(uint16_t) 0x0009
#define IK220_G28REGISTER_10	(uint16_t) 0x000a

#define IK220_G28SEM_0		(uint16_t)0x0001
#define IK220_G28SEM_1		(uint16_t)0x0002
#define IK220_G28SEM_2		(uint16_t)0x0004
#define IK220_G28SEM_3		(uint16_t)0x0008
#define IK220_G28SEM_4		(uint16_t)0x0010
#define IK220_G28SEM_5		(uint16_t)0x0020
#define IK220_G28SEM_6		(uint16_t)0x0040
#define IK220_G28SEM_7		(uint16_t)0x0080
#define IK220_G28SEM_8		(uint16_t)0x0100
#define IK220_G28SEM_9		(uint16_t)0x0200
#define IK220_G28SEM_10		(uint16_t)0x0400

#define IK220_SET_FLAG_0_REGISTER (uint16_t)0x000b
#define IK220_CONTROLREGISTER	  (uint16_t)0x000c

#define IK220_CLEAR_FLAG_1_REGISTER (uint16_t)0x000b
#define IK220_STATUSREGISTER	    (uint16_t)0x000c
#define IK220_FLAG_0_REGISTER	    (uint16_t)0x000d
#define IK220_FLAG_1_REGISTER	    (uint16_t)0x000e
#define IK220_CODEREGISTER          (uint16_t)0x000f

#define	IK220_WAITSTATES	(uint16_t)0x0000 
#define IK220_RUNMODE		(uint16_t)(0x0000 | IK220_WAITSTATES)
#define IK220_BOOTMODE		(uint16_t)(0x0001 | IK220_WAITSTATES)
#define IK220_SINGLEMODE	(uint16_t)(0x0002 | IK220_WAITSTATES)
#define IK220_DEBUGMODE		(uint16_t)(0x0003 | IK220_WAITSTATES)
#define IK220_READ_RAM_MODE	(uint16_t)(0x0004 | IK220_BOOTMODE)
#define IK220_WRITE_RAM_MODE	(uint16_t)(0x0008 | IK220_BOOTMODE)

#define IK220_DOSTEPMODE	(uint16_t)(0x0004 | IK220_SINGLEMODE)

#define IK220_READREGS		(uint16_t)(0x0004 | IK220_DEBUGMODE)
#define IK220_WRITEREGS		(uint16_t)(0x0008 | IK220_DEBUGMODE)

#define IK220_CPUHALT		(uint16_t)0x0001
#define IK220_STATUS_PIPE_EMPTY	(uint16_t)0x0002

#define IK220_CMDREGISTER     IK220_G28REGISTER_0
#define IK220_DATAREGISTER_0  IK220_G28REGISTER_0
#define IK220_DATAREGISTER_1  IK220_G28REGISTER_1
#define IK220_DATAREGISTER_2  IK220_G28REGISTER_2
#define IK220_DATAREGISTER_3  IK220_G28REGISTER_3
#define IK220_DATAREGISTER_4  IK220_G28REGISTER_4
#define IK220_DATAREGISTER_5  IK220_G28REGISTER_5
#define IK220_DATAREGISTER_6  IK220_G28REGISTER_6
#define IK220_DATAREGISTER_7  IK220_G28REGISTER_7
#define IK220_DATAREGISTER_8  IK220_G28REGISTER_8
#define IK220_DATAREGISTER_9  IK220_G28REGISTER_9
#define IK220_STAREGISTER     IK220_G28REGISTER_10

#define IK220_CONFREG_CNTRL   (0x50>>2)

#define IK220_CMD_RESET	    (uint16_t)0x0101
#define IK220_CMD_START	    (uint16_t)0x0102
#define IK220_CMD_STOP	    (uint16_t)0x0103
#define IK220_CMD_CLEAR_ERR (uint16_t)0x0104

#define IK220_CMD_LATCH_0   (uint16_t)0x0105
#define IK220_CMD_LATCH_1   (uint16_t)0x0106
#define IK220_CMD_LATCH_2   (uint16_t)0x0107

#define IK220_CMD_RESET_REF   (uint16_t)0x0201
#define IK220_CMD_START_REF   (uint16_t)0x0202
#define IK220_CMD_STOP_REF    (uint16_t)0x0203
#define IK220_CMD_LATCH_REF_2 (uint16_t)0x0204
#define IK220_CMD_TRAVERSE_REF (uint16_t)0x0205
#define IK220_CMD_CANCEL_REF  (uint16_t)0x0206

#define IK220_CMD_POSITION_REF   (uint16_t)0x0207
#define IK220_CMD_POSITION_REF_2 (uint16_t)0x0208
#define IK220_CMD_POSITION_REF_3 (uint16_t)0x0209

#define IK220_CMD_READ_CNT_0	 (uint16_t)0x0301
#define IK220_CMD_READ_CNT_1     (uint16_t)0x0302
#define IK220_CMD_GET_CNT_0      (uint16_t)0x0303
#define IK220_CMD_GET_CNT_1      (uint16_t)0x0304
#define IK220_CMD_GET_CNT_2      (uint16_t)0x0305

#define IK220_CMD_GET_REF_STAT  (uint16_t)0x0401
#define IK220_CMD_GET_SIG_STAT  (uint16_t)0x0402

#define IK220_CMD_GET_CORR_A    (uint16_t)0x0403
#define IK220_CMD_GET_CORR_B    (uint16_t)0x0404
#define IK220_CMD_LOAD_CORR_A   (uint16_t)0x0405

#define IK220_CMD_GET_OKT_CNT   (uint16_t)0x0406

#define IK220_CMD_GET_VERSION   (uint16_t)0x0407
#define IK220_CMD_GET_PORT      (uint16_t)0x0408

#define IK220_CMD_GET_CHK_SUM_PAR (uint16_t)0x0409
#define IK220_CMD_GET_CHK_SUM_PRG (uint16_t)0x040A

#define IK220_CMD_GET_CNT_STATUS (uint16_t)0x040B


#define IK220_CMD_WRITE_PAR      (uint16_t)0x0501
#define IK220_CMD_READ_PAR       (uint16_t)0x0502

#define IK220_CMD_RESET_EN       (uint16_t)0x0601
#define IK220_CMD_CONFIG_EN      (uint16_t)0x0602
#define IK220_CMD_READ_EN        (uint16_t)0x0603

#define IK220_CMD_READ_ALARM_EN  (uint16_t)0x0604
#define IK220_CMD_READ_WARN_EN   (uint16_t)0x0605
#define IK220_CMD_CLEAR_ALARM_EN (uint16_t)0x0606
#define IK220_CMD_CLEAR_WARN_EN  (uint16_t)0x0607

#define IK220_CMD_COMM_L_EN      (uint16_t)0x0608

#define IK220_CMD_COMM_S_EN      (uint16_t)0x0609

#define IK220_CMD_READ_EN_INC_0 (uint16_t)0x060A	
#define IK220_CMD_READ_EN_INC_1 (uint16_t)0x060B

#define IK220_CMD_EN_CONT_MODE_0 (uint16_t)0x060C
#define IK220_CMD_EN_CONT_MODE_1 (uint16_t)0x060D
#define IK220_CMD_EN_CONT_MODE_2 (uint16_t)0x060E

#define IK220_CMD_READ_EN_INC_CONT (uint16_t)0x060F

#define IK220_CMD_TIMER_MODE     (uint16_t)0x0701

#define IK220_CMD_RAM_MODE       (uint16_t)0x0801
#define IK220_CMD_RESET_RAM      (uint16_t)0x0802
#define IK220_CMD_GET_RAM        (uint16_t)0x0803

#define IK220_CMD_READ_SSI       (uint16_t)0x0901
#define IK220_CMD_CONFIG_SSI     (uint16_t)0x0902
#define IK220_CMD_READ_SSI_INC_0 (uint16_t)0x0903
#define IK220_CMD_READ_SSI_INC_1 (uint16_t)0x0904

#define IK220_CMD_WRITE_PORT     (uint16_t)0x1001
#define IK220_CMD_READ_PORT      (uint16_t)0x1002
#define IK220_CMD_WRITE_RAM      (uint16_t)0x1003
#define IK220_CMD_READ_RAM       (uint16_t)0x1004
#define IK220_CMD_SET_BW         (uint16_t)0x1005
#define IK220_CMD_SET_TRM        (uint16_t)0x1006
#define IK220_CMD_REF_EVAL       (uint16_t)0x1007

#define IK220_CMD_LED_OFF       (uint16_t)0x1010
#define IK220_CMD_LED_ON        (uint16_t)0x1011
#define IK220_CMD_LED_FLASH     (uint16_t)0x1012

#define IK220_CMD_RESET_EN_CLOCK (uint16_t)0x1020
#define IK220_CMD_SET_EN_CLOCK   (uint16_t)0x1021

#define IK220_CMD_RESET_EN_DATA (uint16_t)0x1022
#define IK220_CMD_SET_EN_DATA   (uint16_t)0x1023

#define IK220_CMD_READ_EN_DATA  (uint16_t)0x1024

#define IK220_CMD_SET_OP_MODE   (uint16_t)0x1030
#define IK220_CMD_GET_OP_MODE   (uint16_t)0x1031

#define IK220_CMD_WRONG_COMMAND (uint16_t)0xFEDC
#define IK220_CMD_WRONG_COMMAND_EN (uint16_t)0xFEDD

#define IK220_MODE_SEL_RANGE_EN (uint16_t)0x0E
#define IK220_MODE_WRITE_MEM_EN (uint16_t)0X1C
#define IK220_MODE_READ_MEM_EN  (uint16_t)0X23

#define IK220_STATUS_BUFFER_OVERFLOW (uint16_t)0x0001
#define IK220_STATUS_BUFFER_NO_VAL   (uint16_t)0x0002
#define IK220_STATUS_BUFFER_EMPTY    (uint16_t)0x0004
#define IK220_STATUS_BUFFER_FAILED   (uint16_t)0x8000

#define IK220_PAR_TIMER_VALUE	11
#define IK220_PAR_PRE_SCALER	12

struct ik220_data
{
   int axis;
   unsigned short address;
   int size;
   unsigned long data;
};
#endif
