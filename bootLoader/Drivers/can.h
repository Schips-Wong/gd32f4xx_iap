#ifndef __CAN_H__
#define __CAN_H__

/* select CAN baudrate */
#define CAN_BAUDRATE  500 // 500K

/* 1MBps */
/* #define CAN_BAUDRATE  1000 */

/* 500kBps */
/* #define CAN_BAUDRATE  500 */

/* 250kBps */
/* #define CAN_BAUDRATE  250 */

/* 125kBps */
/* #define CAN_BAUDRATE  125 */

/* 100kBps */ 
/* #define CAN_BAUDRATE  100 */

/* 50kBps */ 
/* #define CAN_BAUDRATE  50 */

/* 20kBps */ 
/* #define CAN_BAUDRATE  20 */

void can_config(void);

/*!
    \brief      Recv Stand Frame Data from CAN Port X
    \param[in]  which_can : CAN0, CAN1
    \param[out] sfid      : rx_sfid
    \param[out] data      : rx data
    \retval     len for rx data ( <=8)
*/
int try_recv_can_stand(unsigned int which_can, unsigned int *sfid, char *data);

/*!
    \brief      Send Stand Frame Data To CAN Port X
    \param[in]  which_can : CAN0, CAN1
    \param[in] sfid      : rx_sfid
    \param[in] data      : rx data
    \param[in] data      : rx data len
*/
void can_send_stand_frame(unsigned int which_can, unsigned int sfid, char *data, int data_len);


#endif /*__CAN_H__*/
