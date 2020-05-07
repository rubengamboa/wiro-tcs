#ifndef IK220_LIB_H
#define IK220_LIB_H

#ifdef __cplusplus
extern "C"{
#endif


extern char IK220Find(uint32_t *buffer);
extern char IK220Init(uint16_t axis);
extern char IK220Version(uint16_t axis, char *vers_card, char *vers_drv, char *vers_lib);

extern char IK220Reset(uint16_t axis);
extern char IK220Start(uint16_t axis);
extern char IK220Stop(uint16_t axis);
extern char IK220ClearErr(uint16_t axis);

extern char IK220Latch(uint16_t axis, uint16_t latch);
extern char IK220LatchInt(uint16_t card);
extern char IK220LatchExt(uint16_t card);
extern char IK220ResetRef(uint16_t axis);
extern char IK220StartRef(uint16_t axis);
extern char IK220StopRef(uint16_t axis);
extern char IK220LatchRef(uint16_t axis);
extern char IK220Latched(uint16_t axis, uint16_t latch, char *status);
extern char IK220WaitLatch(uint16_t axis, uint16_t latch);

extern char IK220SetTimeOut(uint32_t timeout);

extern char IK220Set(uint16_t axis, double value);
extern char IK220SetPreset(uint16_t axis, double value);
extern char IK220GetPreset(uint16_t axis, double value);

extern char IK220Read32(uint16_t axis, uint16_t latch, int32_t *data);
extern char IK220Read48(uint16_t axis, uint16_t latch, double *data);
extern char IK220Get32(uint16_t axis, uint16_t latch, int32_t *data);
extern char IK220Get48(uint16_t axis, uint16_t latch, double *data);

extern char IK220CntStatus(uint16_t axis, uint16_t latch, uint16_t *ref_status,
                           int16_t *korr00, int16_t *korr90,
                           int16_t *n_korr00, int16_t *n_korr90,
                           uint16_t *sam_cnt);

extern char IK220DoRef(uint16_t axis);
extern char IK220CancelRef(uint16_t axis);
extern char IK220RefActive(uint16_t axis, char *status);
extern char IK220WaitRef(uint16_t axis);
extern char IK220PositionRef(uint16_t axis, double *data, int32_t *period, uint16_t *intpol);
extern char IK220PositionRef2(uint16_t axis, double *data, int32_t *period, uint16_t *intpol);

extern char IK220Status(uint16_t axis, uint32_t *status);
extern char IK220LibStatus(uint32_t *status, uint32_t *info);
#define IK220DllStatus IK220LibStatus

extern char IK220RefStatus(uint16_t axis, int32_t *ref1, int32_t *ref2, int32_t *diff,
                           int32_t *code, uint16_t *flag);

extern char IK220SignalStatus(uint16_t axis, uint16_t *freq, uint16_t *amin, uint16_t *aact, uint16_t *amax);

extern char IK220GetCorrA(uint16_t axis, int16_t *offs0, int16_t *offs90, int16_t *pha0, int16_t *pha90,
                          int16_t *sym0, int16_t *sym90, uint16_t *flag1, uint16_t *flag2);
extern char IK220GetCorrB(uint16_t axis, int16_t *offs0, int16_t *offs90, int16_t *pha0, int16_t *pha90,
                          int16_t *sym0, int16_t *sym90, uint16_t *flag1, uint16_t *flag2);
extern char IK220LoadCorrA(uint16_t axis, int16_t offs0, int16_t offs90, int16_t pha0, int16_t pha90,
                           int16_t sym0, int16_t sym90);

extern char IK220OctStatus(uint16_t axis, uint16_t *oct0, uint16_t *oct1, uint16_t *oct2, uint16_t *oct3,
                           uint16_t *oct4, uint16_t *oct5, uint16_t *oct6, uint16_t *oct7, uint16_t *sam_cnt);

extern char IK220ChkSumPar(uint16_t axis, uint16_t *chksum);
extern char IK220ChkSumPrg(uint16_t axis, uint16_t *chksum1, uint16_t *chksum2);

extern char IK220WritePar(uint16_t axis, uint16_t par_num, uint32_t par_val);
extern char IK220ReadPar(uint16_t axis, uint16_t par_num, uint32_t *par_val);

extern char IK220ResetEn(uint16_t axis, uint16_t *status);
extern char IK220ConfigEn(uint16_t axis, uint16_t *status, uint16_t *type, uint32_t *period, uint32_t *step,
                          uint16_t *turns, uint16_t *ref_dist, uint16_t *cnt_dir);

extern char IK220ReadEn(uint16_t axis, uint16_t *status, double *data, uint16_t *alarm);
extern char IK220ReadEnInc(uint16_t axis, uint16_t latch, uint16_t *status, double *data_en,
                           uint16_t *alarm, double *data_inc);

extern char IK220ModeEnCont(uint16_t axis, uint16_t *latch, uint16_t mode, uint16_t *status);
extern char IK220ReadEnIncCont(uint16_t axis, uint16_t *status, double *data_en,
                               uint16_t *alarm, double *data_inc, uint16_t *sig_stat);
extern char IK220AlarmEn(uint16_t axis, uint16_t *alarm);
extern char IK220WarnEn(uint16_t axis, uint16_t *warn);

extern char IK220ReadMemEn(uint16_t axis, uint16_t range, uint16_t mem_addr, uint16_t *data, uint16_t *status);
extern char IK220WriteMemEn(uint16_t axis, uint16_t range, uint16_t mem_addr, uint16_t data, uint16_t *status);

extern char IK220ReadSSI(uint16_t axis, uint16_t *status, double *data);
extern char IK220ReadSsiInc(uint16_t axis, uint16_t latch, uint16_t *status, double *data_ssi, double *data_inc);

extern char IK220SetTimer(uint16_t axis, uint32_t value, uint32_t *tim_value);
extern char IK220ModeTimer(uint16_t axis, uint16_t mode);

extern char IK220ModeRam(uint16_t axis, uint16_t mode);
extern char IK220ResetRam(uint16_t axis);
extern char IK220GetRam(uint16_t axis, double *data, uint16_t *read, uint16_t *write, uint16_t *status);
extern char IK220BurstRam(uint16_t axis, uint16_t max_count, double *data, uint16_t *count, uint16_t *status);

extern char IK220GetSig(uint16_t axis, uint16_t *period, int16_t *amp0, int16_t *amp90,
                        uint16_t *read, uint16_t *write, uint16_t *status);
extern char IK220BurstSig(uint16_t axis, uint16_t max_count, uint16_t *period, int16_t *amp0,
                          int16_t *amp90, uint16_t *count, uint16_t *status);

extern char IK220Led(uint16_t axis, uint16_t mode);
extern char IK220SysLed(uint16_t card, uint16_t mode);

extern char IK220GetPort(uint16_t axis, uint16_t *port_info, uint16_t *rising, uint16_t *falling);

extern char IK220RefEval(uint16_t axis, uint16_t mode);
extern char IK220SetBw(uint16_t axis, uint16_t mode);

#ifdef __cplusplus
}
#endif
#endif /* #ifndef IK220_LIB_H */
