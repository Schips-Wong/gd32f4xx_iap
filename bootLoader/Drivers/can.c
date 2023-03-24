#include "can.h"
#include "gd32f4xx.h"

FlagStatus can0_receive_flag;
FlagStatus can1_receive_flag;
FlagStatus can0_error_flag;
FlagStatus can1_error_flag;

can_trasnmit_message_struct transmit_message;
can_receive_message_struct receive_message0;
can_receive_message_struct receive_message1;


/*!
    \brief      Recv Stand Frame Data from CAN Port X
    \param[in]  which_can : CAN0, CAN1
    \param[out] sfid      : rx_sfid
    \param[out] data      : rx data
    \retval     len for rx data ( <=8)
*/
int try_recv_can_stand(unsigned int which_can, unsigned int *sfid, char *data)
{
    int i;
    int ret = -1;

    if(which_can == CAN0) {
        if(SET == can0_error_flag){
            can0_error_flag = RESET;
            return -1;
        }
        if(can0_receive_flag == RESET) {
            return -1;
        }
        can0_receive_flag = RESET;
        if(sfid) {
            *sfid = receive_message0.rx_sfid;
        }
        if(data) {
            for(i = 0; i < receive_message0.rx_dlen; i++){
                data[i] = receive_message0.rx_data[i];
            }
            ret = receive_message0.rx_dlen;
            receive_message0.rx_dlen = 0;
        }
    } else
    if(which_can == CAN1) {
        if(SET == can1_error_flag) {
            can1_error_flag = RESET;
            return -1;
        }
        if(can1_receive_flag == RESET) {
            return -1;
        }
        can1_receive_flag = RESET;
        if(sfid) {
            *sfid = receive_message1.rx_sfid;
        }
        if(data) {
            for(i = 0; i < receive_message1.rx_dlen; i++){
                data[i] = receive_message1.rx_data[i];
            }
            ret = receive_message1.rx_dlen;
            receive_message1.rx_dlen = 0;
        }
    }
    return ret;
}

/*!
    \brief      Send Stand Frame Data To CAN Port X
    \param[in]  which_can : CAN0, CAN1
    \param[in] sfid      : rx_sfid
    \param[in] data      : rx data
    \param[in] data      : rx data len
*/
void can_send_stand_frame(unsigned int which_can, unsigned int sfid, char *data, int data_len)
{
    int i;
    uint32_t timeout = 0xFFFF;
    uint8_t transmit_mailbox = 0;

    /* initialize transmit message */
    transmit_message.tx_sfid = sfid;
    transmit_message.tx_efid = 0x00;
    transmit_message.tx_ft = CAN_FT_DATA;
    transmit_message.tx_ff = CAN_FF_STANDARD;
    transmit_message.tx_dlen = 8;
    // 不足时填0
    for(i = 0; i < transmit_message.tx_dlen; i++)
    {
        if(i < data_len)
        {
            transmit_message.tx_data[i] = data[i];
        }else
        {
            transmit_message.tx_data[i] = 0x00;
        }
    }
#if DEBUG
    printf("\r\n can %d transmit data:", can_id - CAN0);
    for(i = 0; i < transmit_message.tx_dlen; i++)
    {
        printf(" %02x", transmit_message.tx_data[i]);
    }
#endif
    transmit_mailbox = can_message_transmit(which_can, &transmit_message);
    /* waiting for transmit completed */
    while((CAN_TRANSMIT_OK != can_transmit_states(which_can, transmit_mailbox)) && (0 != timeout)){
        timeout--;
    }
}


static void _can_0_1_gpio_config(void)
{
    /* enable CAN clock */
    rcu_periph_clock_enable(RCU_CAN0);
    rcu_periph_clock_enable(RCU_CAN1);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOD);
    
    /* configure CAN1 GPIO */
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_13);
    gpio_af_set(GPIOB, GPIO_AF_9, GPIO_PIN_13);
    
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_12);
    gpio_af_set(GPIOB, GPIO_AF_9, GPIO_PIN_12);
    
    /* configure CAN0 GPIO */
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);
    gpio_mode_set(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_1);
    gpio_af_set(GPIOD, GPIO_AF_9, GPIO_PIN_1);
    
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
    gpio_mode_set(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_0);
    gpio_af_set(GPIOD, GPIO_AF_9, GPIO_PIN_0);
}

static void _can_0_1_config()
{
    can_parameter_struct            can_parameter;
    can_filter_parameter_struct     can_filter;
    can_struct_para_init(CAN_INIT_STRUCT, &can_parameter);
    can_struct_para_init(CAN_FILTER_STRUCT, &can_filter);
    /* initialize CAN register */
    can_deinit(CAN0);
    can_deinit(CAN1);
    
    /* initialize CAN parameters */
    can_parameter.time_triggered = DISABLE;
    can_parameter.auto_bus_off_recovery = ENABLE;
    can_parameter.auto_wake_up = DISABLE;
    can_parameter.auto_retrans = ENABLE;
    can_parameter.rec_fifo_overwrite = DISABLE;
    can_parameter.trans_fifo_order = DISABLE;
    can_parameter.working_mode = CAN_NORMAL_MODE;
    can_parameter.resync_jump_width = CAN_BT_SJW_1TQ;
    can_parameter.time_segment_1 = CAN_BT_BS1_7TQ;
    can_parameter.time_segment_2 = CAN_BT_BS2_2TQ;
    
    /* 1MBps */
#if CAN_BAUDRATE == 1000
    can_parameter.prescaler = 5;
    /* 500KBps */
#elif CAN_BAUDRATE == 500
    can_parameter.prescaler = 10;
    /* 250KBps */
#elif CAN_BAUDRATE == 250
    can_parameter.prescaler = 20;
    /* 125KBps */
#elif CAN_BAUDRATE == 125
    can_parameter.prescaler = 40;
    /* 100KBps */
#elif  CAN_BAUDRATE == 100
    can_parameter.prescaler = 50;
    /* 50KBps */
#elif  CAN_BAUDRATE == 50
    can_parameter.prescaler = 100;
    /* 20KBps */
#elif  CAN_BAUDRATE == 20
    can_parameter.prescaler = 250;
#else
    #error "please select list can baudrate"
#endif  
    /* initialize CAN */
    can_init(CAN0, &can_parameter);
    can_init(CAN1, &can_parameter);
    
    /* initialize filter */ 
    can_filter.filter_number=0;
    can_filter.filter_mode = CAN_FILTERMODE_MASK;
    can_filter.filter_bits = CAN_FILTERBITS_32BIT;
    can_filter.filter_list_high = 0x0000;
    can_filter.filter_list_low = 0x0000;
    can_filter.filter_mask_high = 0x0000;
    can_filter.filter_mask_low = 0x0000;
    can_filter.filter_fifo_number = CAN_FIFO0;
    can_filter.filter_enable = ENABLE;
    
    can_filter_init(&can_filter);
    
    /* CAN1 filter number */
    can_filter.filter_number = 15;
    can_filter_init(&can_filter);
    
    /*** configure the nested vectored interrupt controller ***/
    /* configure CAN0 NVIC */
    nvic_irq_enable(CAN0_RX0_IRQn,0,0);

    /* configure CAN1 NVIC */
    nvic_irq_enable(CAN1_RX0_IRQn,1,1);
    
    /* enable can receive FIFO0 not empty interrupt */
    can_interrupt_enable(CAN0, CAN_INT_RFNE0);
    can_interrupt_enable(CAN1, CAN_INT_RFNE0);
}

/*!
    \brief      can_config
    \param[in]  none
    \param[out] none
    \retval     none
*/
void can_config(void)
{
    can0_receive_flag = RESET;
    can1_receive_flag = RESET;
    can0_error_flag = RESET;
    can1_error_flag = RESET;
    
    /* configure GPIO */
    _can_0_1_gpio_config();
    
    /* initialize CAN and filter */
    _can_0_1_config();
}

/*!
    \brief      this function handles CAN0 RX0 exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void CAN0_RX0_IRQHandler(void)
{
    /* check the receive message */
    can_message_receive(CAN0, CAN_FIFO0, &receive_message0);
    can0_error_flag = RESET;

    if((CAN_FF_STANDARD == receive_message0.rx_ff)&&(8 == receive_message0.rx_dlen)) // (0x7ab == receive_message.rx_sfid)
    {
        can0_receive_flag = SET; 
    }else{
        can0_error_flag = SET; 
    }
}
/*!
    \brief      this function handles CAN1 RX0 exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void CAN1_RX0_IRQHandler(void)
{
    /* check the receive message */
    can_message_receive(CAN1, CAN_FIFO0, &receive_message1);
    can1_error_flag = RESET;

    if((CAN_FF_STANDARD == receive_message1.rx_ff)&&(8 == receive_message1.rx_dlen)) // 0x7ab == receive_message.rx_sfid)
    {
        can1_receive_flag = SET; 
    }else{
        can1_error_flag = SET; 
    }
}
