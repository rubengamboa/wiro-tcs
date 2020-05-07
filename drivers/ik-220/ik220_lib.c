#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "ik220_if.h"
#include "Pgm220.h"

static int ik220_fd;
static char ik220_device_open = 0;
static const char DEVNAME[] = IK220_DEVNAME;

static const char LIB_VERSION[20] = "LIB: V "IK220_LIB_VERSION"(R)";

static const char TRUE = 1;
static const char FALSE = 0;

static double preset[IK220_MAX_AXIS] = {0.0, 0.0, 0.0, 0.0, 0.0, 
	                                      0.0, 0.0, 0.0, 0.0, 0.0,
	                                      0.0, 0.0, 0.0, 0.0, 0.0,
	                                      0.0};

char IK220InputW(uint16_t axis, uint16_t address, uint16_t *data);
char IK220InputL(uint16_t axis, uint16_t address, uint32_t *data);
char IK220Output(uint16_t axis, uint16_t address, uint16_t data);

char IK220RamRead(uint16_t axis, uint16_t address, uint16_t *data);
char IK220RamWrite(uint16_t axis, uint16_t address, uint16_t data);

char IK220DownLoad(uint16_t axis, const uint16_t *prog_data, uint32_t prog_size);

char IK220SetEnClock(uint16_t axis, char state, uint16_t *status);
char IK220SetEnData(uint16_t axis, char state, uint16_t *status);

char IK220ReadEnData(uint16_t axis, char *state);

static char DownLoad(uint16_t axis, const uint16_t *prog_data, uint32_t prog_size);
static char OutputW(uint16_t axis, uint16_t address, uint16_t data);
static char InputW(uint16_t axis, uint16_t address, uint16_t *data);
static char InputL(uint16_t axis, uint16_t address, uint32_t *data);
static char IKFind(uint32_t *buffer);
static char GetVers(uint16_t axis, char *version_card, char *version_driver);
static char OutCmd(uint16_t axis, uint16_t cmd);
static char ik220_open_device(void);




static char ik220_open_device(void)
{
  if(ik220_device_open) return TRUE;

  if((ik220_fd = open(DEVNAME, O_RDWR)) < 0)
  {
     return FALSE;
  }
  else
  {
     ik220_device_open = 1;
  }

	return TRUE;
}



static char LatchInt(uint16_t card)
{
	// generate latch pulse for internal synchronous latch
	
	uint16_t seppi = card;
	long              rc;
  struct ik220_data output_struct;

	if (card >= IK220_MAX_CARDS)
		return FALSE;

  output_struct.axis = card;
  output_struct.address = card;
  output_struct.size = 0;
  output_struct.data = 0;

  if ((rc = ioctl(ik220_fd, 
				          _IOC(_IOC_WRITE, IK220_IOCTL_TYPE, IK220_IOCTL_LATCHI, 0), 
								  (unsigned long) &output_struct)) < 0)
		return FALSE;

	return TRUE;
} 



static char LatchExt(uint16_t card)
{
	// generate latch pulse for external synchronous latch
	
	long              rc;
  struct ik220_data output_struct;

	if (card >= IK220_MAX_CARDS)
		return FALSE;

  output_struct.axis = card;
  output_struct.address = card;
  output_struct.size = 0;
  output_struct.data = 0;

  if ((rc = ioctl(ik220_fd, 
				          _IOC(_IOC_WRITE, IK220_IOCTL_TYPE, IK220_IOCTL_LATCHE, 0), 
								  (unsigned long) &output_struct)) < 0)
		return FALSE;

	return TRUE;
}



static char OutputW(uint16_t axis, uint16_t address, uint16_t data)
{
   struct ik220_data output_struct;
   long rc;

   output_struct.axis = axis;
   output_struct.address = address;
   output_struct.size = sizeof data;

   output_struct.data = (unsigned long)&data;

   if((rc = ioctl(ik220_fd, _IOC(_IOC_WRITE, IK220_IOCTL_TYPE, IK220_IOCTL_OUTPUT, 0), (unsigned long)&output_struct)) < 0)
   {
      return FALSE;
   }
  
	return TRUE;
}



static char InputW(uint16_t axis, uint16_t address, uint16_t *data)
{
   long              rc;
   struct ik220_data output_struct;

   output_struct.axis = axis;
   output_struct.address = address;
   output_struct.size = sizeof *data;

   output_struct.data = (unsigned long)data;

   if((rc = ioctl(ik220_fd, _IOC(_IOC_READ, IK220_IOCTL_TYPE, IK220_IOCTL_INPUT, 0), (unsigned long)&output_struct)) < 0)
   {
      return FALSE;
   }
  
return TRUE;
}



static char InputL(uint16_t axis, uint16_t address, uint32_t *data)
{
   struct ik220_data output_struct;
   long rc;

   output_struct.axis = axis;
   output_struct.address = address;
   output_struct.size = sizeof *data;

   output_struct.data = (unsigned long)data;

   if((rc = ioctl(ik220_fd, _IOC(_IOC_READ, IK220_IOCTL_TYPE, IK220_IOCTL_INPUT, 0), (unsigned long)&output_struct)) < 0)
   {
      return FALSE;
   }
  
return TRUE;
}



static char OutCmd(uint16_t axis, uint16_t cmd)
{
   uint16_t reg_word;

   if (!OutputW (axis, IK220_CMDREGISTER, cmd)) return FALSE;

   do
   {
      if(!InputW(axis, IK220_FLAG_1_REGISTER, &reg_word)) return FALSE;
   }while(!(reg_word & IK220_G28SEM_0)); /* FIXME: Do some timeout thingy */
   
   if(!InputW(axis, IK220_DATAREGISTER_0, &reg_word)) return FALSE;
   
   if(reg_word != cmd) return FALSE;
   
return TRUE;
}



static char IKFind(uint32_t *buffer)
{
   long rc;
   unsigned long *buffer2;

   if(!ik220_device_open)
   {
     fprintf(stderr,"Opening IK....\n");
     if(ik220_open_device() == FALSE)
       {
	 fprintf(stderr,"NO Driver found!\nPlease check:\n1. Do you set an /dev/ik220 with mknod?\n2. Correct Permissons on Driver?\n");
	 return FALSE;
       }
   } 

   if(buffer == NULL)
   {
   /* FIXME: Do some status stuff */
   }
   buffer2 = malloc(IK220_MAX_AXIS * 4);
   //fprintf(stderr, "Vor ioctl buffer:%lX\n", buffer);
   if((rc = ioctl(ik220_fd, _IOC(_IOC_READ, IK220_IOCTL_TYPE, IK220_IOCTL_STATUS, 1), (unsigned long)buffer2)) < 0)
   {
      return FALSE;
   }

   for(rc = 0; rc < IK220_MAX_AXIS; ++rc)
      buffer[rc] = buffer2[rc];

   free(buffer2);

return TRUE;
}



static char GetVers(uint16_t axis, char *version_card, char *version_driver)
{
   char *buffer;
   struct ik220_data version_struct;
   long rc;
   int i, j;

   version_struct.axis = axis;
   version_struct.size = IK220_VERSION_SIZE;
   buffer = malloc(IK220_VERSION_SIZE);
   version_struct.data = (unsigned long) buffer;
   
//   fprintf(stderr, "Wo? Na in %lX natuerlich!\n", buffer);  
 
   if((rc = ioctl(ik220_fd, _IOC(_IOC_READ, IK220_IOCTL_TYPE, IK220_IOCTL_VERSION, 0), (unsigned long)&version_struct)) < 0)
   {
      return FALSE;
   }

/* FIXME: Just for now! */
   i = j = 0;
   while(i < IK220_VERSION_SIZE)
      if(buffer[i++] == '\0')
         ++j;
         
   if(j >= 2)
   {
      strcpy(version_card, buffer);
      strcpy(version_driver, buffer + (strlen(buffer) + 1));
   }
   else
   {
      return FALSE;
   }
   free(buffer);

return TRUE;
}


   
char DownLoad(uint16_t axis, const uint16_t *prog_data, uint32_t prog_size)
{
   struct ik220_data prog_struct;
   long rc;

   if(axis > IK220_MAX_AXIS) return FALSE;

   if((prog_size < 2) | (prog_size>(65536)*2) ) return FALSE;

   prog_struct.axis = axis;
   prog_struct.address = 0;
   prog_struct.size = prog_size; 
   prog_struct.data = (unsigned long) prog_data;

   if((rc = ioctl(ik220_fd, _IOC(_IOC_WRITE, IK220_IOCTL_TYPE, IK220_IOCTL_DOWNLOAD, 0), (unsigned long)&prog_struct)) < 0)
   {
      //fprintf(stderr, "RetVal was %ld!\n", rc);
      return FALSE;
   }

return TRUE;
}



char IK220Find(uint32_t *buffer)
{
   int i;

   if(IKFind(buffer) == FALSE)
      return FALSE;

   for(i = 0; i < IK220_MAX_AXIS; ++i)
   {
      if(buffer[i]) return TRUE;
   }

return FALSE;
}


char IK220Init(uint16_t axis)
{
   uint16_t reg_word;
   
   if(axis > IK220_MAX_AXIS) return FALSE;

   if(!DownLoad(axis, Pgm220, sizeof Pgm220)) return FALSE;

   if(!InputW(axis, IK220_CLEAR_FLAG_1_REGISTER, &reg_word)) return FALSE;
   //fprintf(stderr, "%d Nr 1 gefunden\n", reg_word);
   if(!OutputW(axis, IK220_CONTROLREGISTER, IK220_RUNMODE)) return FALSE;
   /* fprintf(stderr, "Habs hinter mir!\n"); */
   do
   {
      if(!InputW(axis, IK220_FLAG_1_REGISTER, &reg_word)) return FALSE;
      //fprintf(stderr, "%d gefunden\n", reg_word);
      sleep(1);
   }while(!(reg_word & IK220_G28SEM_10)); /* FIXME: Do some timeout thingy */

   if(!InputW(axis, IK220_CLEAR_FLAG_1_REGISTER, &reg_word)) return FALSE;
   fprintf(stderr, "Init abgeschlossen!\n");
return TRUE;
}

char IK220Version(uint16_t axis, char *vers_card, char *vers_drv, char *vers_lib)
{

   if(axis > IK220_MAX_AXIS) return FALSE;
   
   if(!GetVers(axis, vers_card, vers_drv)) return FALSE;

   strcpy(vers_lib, LIB_VERSION);

return TRUE;
}


char IK220Reset(uint16_t axis)
{
   
   if(axis > IK220_MAX_AXIS) return FALSE;
   
   if (!OutCmd (axis, (uint16_t)(IK220_CMD_RESET))) return FALSE;
   
return TRUE;
}

char IK220Start(uint16_t axis)
{
   if(axis > IK220_MAX_AXIS) return FALSE;
   
   if (!OutCmd (axis, (uint16_t)(IK220_CMD_START))) return FALSE;

return TRUE;
}

char IK220Stop(uint16_t axis)
{
   if(axis > IK220_MAX_AXIS) return FALSE;
   
   if (!OutCmd (axis, (uint16_t)(IK220_CMD_STOP))) return FALSE;

return TRUE;
}

char IK220ClearErr(uint16_t axis)
{
   if(axis > IK220_MAX_AXIS) return FALSE;
   
   if (!OutCmd (axis, (uint16_t)(IK220_CMD_CLEAR_ERR))) return FALSE;

return TRUE;
}


char IK220Latch(uint16_t axis, uint16_t latch)
{
   if(axis > IK220_MAX_AXIS) return FALSE;
   if(latch > 1) return FALSE;

   if (!OutCmd (axis, (uint16_t)(IK220_CMD_READ_CNT_0 + latch))) return FALSE;

return TRUE;
}



char IK220LatchInt(uint16_t card)
{
	// generates internal synchronous latch
	
	if (card >= IK220_MAX_CARDS)
		return FALSE;

	if (!LatchInt(card))
		return FALSE;
	
	return TRUE;
}



char IK220LatchExt(uint16_t card)
{
	// generates external synchronous latch
	
	if (card >= IK220_MAX_CARDS)
		return FALSE;

	if (!LatchExt(card))
		return FALSE;
	
	return TRUE;
}



char IK220ResetRef(uint16_t axis)
{
   if(axis > IK220_MAX_AXIS) return FALSE;
   
   if (!OutCmd (axis, (uint16_t)(IK220_CMD_RESET_REF))) return FALSE;

return TRUE;
}

char IK220StartRef(uint16_t axis)
{
   if(axis > IK220_MAX_AXIS) return FALSE;
   
   if (!OutCmd (axis, (uint16_t)(IK220_CMD_START_REF))) return FALSE;

return TRUE;
}

char IK220StopRef(uint16_t axis)
{
   if(axis > IK220_MAX_AXIS) return FALSE;
   
   if (!OutCmd (axis, (uint16_t)(IK220_CMD_STOP_REF))) return FALSE;

return TRUE;
}

char IK220LatchRef(uint16_t axis)
{
   if(axis > IK220_MAX_AXIS) return FALSE;
   
   if (!OutCmd (axis, (uint16_t)(IK220_CMD_LATCH_REF_2))) return FALSE;

return TRUE;
}

char IK220Latched(uint16_t axis, uint16_t latch, char *status)
{
return 0;
}

char IK220WaitLatch(uint16_t axis, uint16_t latch)
{
return 0;
}


char IK220SetTimeOut(uint32_t timeout)
{
return 0;
}


char IK220Set(uint16_t axis, double value)
{
return 0;
}

char IK220SetPreset(uint16_t axis, double value)
{
return 0;
}

char IK220GetPreset(uint16_t axis, double value)
{
return 0;
}


char IK220Read32(uint16_t axis, uint16_t latch, int32_t *data)
{
   uint32_t register_dword;

   if(latch > 1) return FALSE;
   if(axis > IK220_MAX_AXIS) return FALSE;
   if(data == NULL) return FALSE;

   if (!OutCmd (axis, (uint16_t)(IK220_CMD_READ_CNT_0 + latch))) return FALSE;

   if (!InputL (axis, IK220_DATAREGISTER_1, &register_dword)) return FALSE;

   *data = register_dword;
   
return TRUE;
}

char IK220Read48(uint16_t axis, uint16_t latch, double *data)
{
   unsigned long long count = 0;
   uint32_t register_dword;
   uint16_t register_word;

   if(latch > 1) return FALSE;
   if(axis > IK220_MAX_AXIS) return FALSE;

   if (!OutCmd (axis, (uint16_t)(IK220_CMD_READ_CNT_0 + latch))) return FALSE;

   if (!InputL (axis, IK220_DATAREGISTER_1, &register_dword)) return FALSE;
   count += ((unsigned long long) register_dword)<<16;

   if (!InputW (axis, IK220_DATAREGISTER_3, &register_word)) return FALSE;
   count += ((uint32_t) register_word)<<8;
   
   if(register_word & 0x8000)
      count += 0xffff;

   *data = (double) count / 65536.0; /* FIXME: Add Preset */

return TRUE;
}

char IK220Get32(uint16_t axis, uint16_t latch, int32_t *data)
{
return 0;
}

char IK220Get48(uint16_t axis, uint16_t latch, double *data)
{
return 0;
}


char IK220CntStatus(uint16_t axis, uint16_t latch, uint16_t *ref_status,
                           int16_t *korr00, int16_t *korr90,
                           int16_t *n_korr00, int16_t *n_korr90,
                           uint16_t *sam_cnt)
{
return 0;
}


char IK220DoRef(uint16_t axis)
{
   if(axis > IK220_MAX_AXIS) return FALSE;
   
   if (!OutCmd (axis, (uint16_t)(IK220_CMD_TRAVERSE_REF))) return FALSE;

return TRUE;
}

char IK220CancelRef(uint16_t axis)
{
   if(axis > IK220_MAX_AXIS) return FALSE;
   
   if (!OutCmd (axis, (uint16_t)(IK220_CMD_CANCEL_REF))) return FALSE;

return TRUE;
}

char IK220RefActive(uint16_t axis, char *status)
{
return 0;
}

char IK220WaitRef(uint16_t axis)
{
return 0;
}

char IK220PositionRef(uint16_t axis, double *data, int32_t *period, uint16_t *intpol)
{
return 0;
}

char IK220PositionRef2(uint16_t axis, double *data, int32_t *period, uint16_t *intpol)
{
return 0;
}


char IK220Status(uint16_t axis, uint32_t *status)
{
return 0;
}

char IK220LibStatus(uint32_t *status, uint32_t *info)
{
return 0;
}


char IK220RefStatus(uint16_t axis, int32_t *ref1, int32_t *ref2, int32_t *diff,
                           int32_t *code, uint16_t *flag)
{
return 0;
}


char IK220SignalStatus(uint16_t axis, uint16_t *freq, uint16_t *amin, uint16_t *aact, uint16_t *amax)
{
return 0;
}


char IK220GetCorrA(uint16_t axis, int16_t *offs0, int16_t *offs90, int16_t *pha0, int16_t *pha90,
                          int16_t *sym0, int16_t *sym90, uint16_t *flag1, uint16_t *flag2)
{
return 0;
}

char IK220GetCorrB(uint16_t axis, int16_t *offs0, int16_t *offs90, int16_t *pha0, int16_t *pha90,
                          int16_t *sym0, int16_t *sym90, uint16_t *flag1, uint16_t *flag2)
{
return 0;
}

char IK220LoadCorrA(uint16_t axis, int16_t offs0, int16_t offs90, int16_t pha0, int16_t pha90,
                           int16_t sym0, int16_t sym90)
{
return 0;
}


char IK220OctStatus(uint16_t axis, uint16_t *oct0, uint16_t *oct1, uint16_t *oct2, uint16_t *oct3,
                           uint16_t *oct4, uint16_t *oct5, uint16_t *oct6, uint16_t *oct7, uint16_t *sam_cnt)
{
return 0;
}


char IK220ChkSumPar(uint16_t axis, uint16_t *chksum)
{
return 0;
}

char IK220ChkSumPrg(uint16_t axis, uint16_t *chksum1, uint16_t *chksum2)
{
return 0;
}


char IK220WritePar(uint16_t axis, uint16_t par_num, uint32_t par_val)
{
uint16_t check;

   if(axis > IK220_MAX_AXIS) return FALSE;

   if(!OutputW(axis, IK220_DATAREGISTER_1, par_num)) return FALSE;
   if(!OutputW(axis, IK220_DATAREGISTER_2, (uint16_t)(par_val & 0xffff))) return FALSE;
   if(!OutputW(axis, IK220_DATAREGISTER_3, (uint16_t)(par_val>>16))) return FALSE;
   if(!OutCmd(axis, IK220_CMD_WRITE_PAR)) return FALSE;

   if(!InputW(axis, IK220_DATAREGISTER_2, &check)) return FALSE;
   if(check != 0) return FALSE;
   
return TRUE;
}

char IK220ReadPar(uint16_t axis, uint16_t par_num, uint32_t *par_val)
{
return 0;
}

char IK220ResetEn(uint16_t axis, uint16_t *status)
{
   if(axis > IK220_MAX_AXIS) return FALSE;
   if(!OutCmd(axis, IK220_CMD_RESET_EN)) return FALSE;

   if(!InputW(axis, IK220_DATAREGISTER_1, status)) return FALSE;

   if(*status != 0) return FALSE;

return TRUE;
}

char IK220ConfigEn(uint16_t axis, uint16_t *status, uint16_t *type, uint32_t *period, uint32_t *step,
                          uint16_t *turns, uint16_t *ref_dist, uint16_t *cnt_dir)
{
   uint16_t memread_dummy = 0xffff;
	
   if(axis > IK220_MAX_AXIS) return FALSE;

   if(!OutCmd(axis, IK220_CMD_CONFIG_EN)) return FALSE;

   if(!InputW(axis, IK220_DATAREGISTER_1, status)) return FALSE;
   if(!InputW(axis, IK220_DATAREGISTER_2, type)) return FALSE;

   if(!InputL(axis, IK220_DATAREGISTER_3, period)) return FALSE;
   if(!InputL(axis, IK220_DATAREGISTER_5, step)) return FALSE;
   
   if(!InputW(axis, IK220_DATAREGISTER_7, turns)) return FALSE;
   if(!InputW(axis, IK220_DATAREGISTER_8, ref_dist)) return FALSE;
   if(!InputW(axis, IK220_DATAREGISTER_9, cnt_dir)) return FALSE;

   if(*status!= 0) return FALSE;

   if((*type==0x4001) || (*type==0x6001))
   {
	   //FIXME:Something missink hier!
   }   
   else if((*type==0xc001) || (*type==0xe001))
   {
	   //FIXME:Something missink hier!
   }   

//   if(!IK220ReadMemEn(axis, 1, 9, &memread_dummy, status)) return FALSE;
//   if(!IK220ReadMemEn(axis, 1, 10, &memread_dummy, status)) return FALSE;
//   if(!IK220ReadMemEn(axis, 1, 11, &memread_dummy, status)) return FALSE;
//   if(!IK220ReadMemEn(axis, 1, 12, &memread_dummy, status)) return FALSE;
   
return TRUE;
}


char IK220ReadEn(uint16_t axis, uint16_t *status, double *data, uint16_t *alarm)
{
   long long  count = 0;
   uint16_t *p_count = (uint16_t *) &count;
   
   if(axis > IK220_MAX_AXIS) return FALSE;
   
   if(!InputW(axis, IK220_DATAREGISTER_1, status)) return FALSE;
   // fprintf(stderr, "Status vor Cmd: %d ", status);
   if(!OutCmd(axis, IK220_CMD_READ_EN)) return FALSE;

   if(!InputW(axis, IK220_DATAREGISTER_1, status)) return FALSE;
   if(!InputL(axis, IK220_DATAREGISTER_2, (uint32_t*)p_count)) return FALSE;
   p_count += 2;
   if(!InputW(axis, IK220_DATAREGISTER_4, p_count)) return FALSE;
   if(!InputW(axis, IK220_DATAREGISTER_5, alarm)) return FALSE;

   if(*p_count & 0x8000)
   {
      p_count += 1;
      *p_count = 0xffff;
   }

   *data = count;

return TRUE;
}

char IK220ReadEnInc(uint16_t axis, uint16_t latch, uint16_t *status, double *data_en,
                           uint16_t *alarm, double *data_inc)
{
return 0;
}


char IK220ModeEnCont(uint16_t axis, uint16_t *latch, uint16_t mode, uint16_t *status)
{
return 0;
}

char IK220ReadEnIncCont(uint16_t axis, uint16_t *status, double *data_en,
                               uint16_t *alarm, double *data_inc, uint16_t *sig_stat)
{
return 0;
}

char IK220AlarmEn(uint16_t axis, uint16_t *alarm)
{
return 0;
}

char IK220WarnEn(uint16_t axis, uint16_t *warn)
{
return 0;
}


char IK220ReadMemEn(uint16_t axis, uint16_t range, uint16_t mem_addr, uint16_t *data, uint16_t *status)
{
/*   uint16_t mrs_code_en = 0;
   uint16_t mem_data = 0;

   if(axis > IK220_MAX_AXIS) return FALSE;

   switch(range)
   {
      case 0: if(mem_addr > 3) return FALSE;
              mrs_code_en = 0xb9;
	      break;
      case 1: if((mem_addr < 4) || (mem_addr > 47)) return FALSE;
              mrs_code_en = 0xa1 + ((mem_addr >> 3)<<1);
	      mem_addr &= 0x0f;
	      break;
      case 2: if(mem_addr > 15) return FALSE;
              mrs_code_en = 0xa7;
	      break;
      case 3: mrs_code_en = 0xa9;
	      if(!FindEnRange(axis, &mem_addr, &mrs_code_en)) return FALSE;
	      break;
      case 4: mrs_code_en = 0xb1;
	      if(!FindEnRange(axis, &mem_addr, &mrs_code_en)) return FALSE;
	      break;
      default: return FALSE;
   }       	       
    
   if(!OutputW(axis, IK220_DATAREGISTER_1, IK220_MODE_SEL_RANGE_EN)) return FALSE;
   if(!OutputW(axis, IK220_DATAREGISTER_2, mrs_code_en)) return FALSE;

   if(!OutCmd(axis, IK220_CMD_COMM_L_EN)) return FALSE;
   
   if(!InputW(axis, IK220_DATAREGISTER_1, status)) return FALSE;
   if(*status != 0)
      return FALSE;

   if(!OutputW(axis, IK220_DATAREGISTER_1, IK220_MODE_READ_MEM_EN)) return FALSE;
   if(!OutputW(axis, IK220_DATAREGISTER_2, mem_addr)) return FALSE;
   if(!OutCmd(axis, IK220_CMD_COMM_L_EN)) return FALSE;
   
   if(!InputW(axis, IK220_DATAREGISTER_1, status)) return FALSE;
   if(!InputW(axis, IK220_DATAREGISTER_3, &mem_data)) return FALSE;
   if(*status != 0)
      return FALSE;



return TRUE;*/
}

char IK220WriteMemEn(uint16_t axis, uint16_t range, uint16_t mem_addr, uint16_t data, uint16_t *status)
{
return 0;
}


char IK220ReadSSI(uint16_t axis, uint16_t *status, double *data)
{
return 0;
}

char IK220ReadSsiInc(uint16_t axis, uint16_t latch, uint16_t *status, double *data_ssi, double *data_inc)
{
return 0;
}


char IK220SetTimer(uint16_t axis, uint32_t value, uint32_t *tim_value)
{
return 0;
}



char IK220ModeTimer(uint16_t axis, uint16_t mode)
{
	// sets timer mode
	
	if (axis > IK220_MAX_AXIS) 
		return FALSE;

	if (mode > 1)
		return FALSE;

	if (!OutputW (axis, IK220_DATAREGISTER_1, mode))
		return FALSE;

	if (!OutCmd  (axis, IK220_CMD_TIMER_MODE))
		return FALSE;

	return TRUE;
}



char IK220ModeRam(uint16_t axis, uint16_t mode)
{
	// sets mode for RAM buffer
	
	uint16_t status;
	
	if (axis > IK220_MAX_AXIS) 
		return FALSE;

	if ((mode & 0x000F) > 4)
		return FALSE;

	if ((mode >> 4) > 2)
		return FALSE;

	if (!OutputW (axis, IK220_DATAREGISTER_1, mode))
		return FALSE;

	if (!OutCmd(axis, IK220_CMD_RAM_MODE))   // command: set RAM mode
		return FALSE;
	
	if (!InputW(axis, IK220_DATAREGISTER_1, &status))   // get status
		return FALSE;

	if (status != 0) 
		return FALSE;

	return TRUE;
}



char IK220ResetRam(uint16_t axis)
{
return 0;
}

char IK220GetRam(uint16_t axis, double *data, uint16_t *read, uint16_t *write, uint16_t *status)
{
return 0;
}



char IK220BurstRam(uint16_t axis, uint16_t max_count, double *data, uint16_t *count, uint16_t *status)
{
	// read up to max_count values from RAM buffer to data

	uint16_t  i;
	uint16_t  buffer[1+8192*3];       // buffer for status and counter values
	uint16_t  *pbuffer = &buffer[1];  // pointer to buffer

	uint64_t  value;                  // 64 bit
	uint16_t  pvalue;
	
  long      rc;
	
	struct ik220_data output_struct;

	long long  val = 0LL;

	*count = 0;

	if (axis > IK220_MAX_AXIS) 
		return FALSE;

	if ((max_count < 1) || (max_count > 8191))
		return FALSE;

	output_struct.axis = axis;
	output_struct.address = 0;
	output_struct.size = ((max_count * 3) + 1) * 2;
	output_struct.data = (unsigned long) buffer;

  if ((rc = ioctl(ik220_fd, 
					        _IOC(_IOC_READ, IK220_IOCTL_TYPE, IK220_IOCTL_BURSTRAM, 0), 
									(unsigned long) &output_struct)) < 0)
		 return FALSE;

	if ( rc > 2)
	{
		for ( i = 0; i < ( ( ( rc / 2 ) - 1 ) / 3 ); i++)
		{
			val |= (((long long)(*pbuffer)) << 48);
			pbuffer++;
			val |= (((long long)(*pbuffer)) << 32);
			pbuffer++;
			val |= ((long long)(*pbuffer)) << 16;

			if (*pbuffer & 0x8000)
			{
				val |= 0xffff;
			}

			*data++ = preset[axis] + (double) val / 65535.0;
		}

		*count = i;
		*status = buffer[0];
		return TRUE;
	}
	else
	{
		if ( buffer[0] & 0x8000 )
		{
			*status = 0x8000;
			if (( buffer[0] & 0x0001 ) || ( buffer[0] & 0x0002))
			{
				return FALSE;
			}
		}
		else
		{
			*count = 0;
			*status = buffer[0];
			return TRUE;
		}
	}

	return FALSE;
}



char IK220GetSig(uint16_t axis, uint16_t *period, int16_t *amp0, int16_t *amp90,
                        uint16_t *read, uint16_t *write, uint16_t *status)
{
return 0;
}

char IK220BurstSig(uint16_t axis, uint16_t max_count, uint16_t *period, int16_t *amp0,
                          int16_t *amp90, uint16_t *count, uint16_t *status)
{
return 0;
}


char IK220Led(uint16_t axis, uint16_t mode)
{
return 0;
}

char IK220SysLed(uint16_t card, uint16_t mode)
{
return 0;
}


char IK220GetPort(uint16_t axis, uint16_t *port_info, uint16_t *rising, uint16_t *falling)
{
return 0;
}


char IK220RefEval(uint16_t axis, uint16_t mode)
{
return 0;
}

char IK220SetBw(uint16_t axis, uint16_t mode)
{
return 0;
}


