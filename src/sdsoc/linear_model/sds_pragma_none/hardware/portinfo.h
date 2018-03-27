#ifndef _SDS_PORTINFO_H
#define _SDS_PORTINFO_H
/* File: /home/seanf/workspace/linearFloat/Release/_sds/p0/.cf_work/portinfo.h */
#ifdef __cplusplus
extern "C" {
#endif

struct _p0_swblk_LinReg {
  cf_port_send_t cmd_LinReg;
  cf_port_send_t x_PORTA;
  cf_port_send_t a_PORTA;
  cf_port_send_t b;
  cf_port_receive_t output_r_PORTA;
  cf_port_send_t datalen;
};

extern struct _p0_swblk_LinReg _p0_swinst_LinReg_1;
void _p0_cf_framework_open(int);
void _p0_cf_framework_close(int);

#ifdef __cplusplus
};
#endif
#ifdef __cplusplus
extern "C" {
#endif
void switch_to_next_partition(int);
void init_first_partition();
void close_last_partition();
#ifdef __cplusplus
};
#endif /* extern "C" */
#endif /* _SDS_PORTINFO_H_ */
