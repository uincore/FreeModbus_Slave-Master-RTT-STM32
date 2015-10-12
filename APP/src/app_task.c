#include "app_task.h"

#ifdef __CC_ARM
extern int Image$$RW_IRAM1$$ZI$$Limit;
#elif __ICCARM__
#pragma section="HEAP"
#else
extern int __bss_end;
#endif

uint8_t CpuUsageMajor, CpuUsageMinor; //CPU使用率
USHORT  usModbusUserData[MB_PDU_SIZE_MAX];
UCHAR   ucModbusUserData[MB_PDU_SIZE_MAX];
//====================操作系统各线程优先级==================================
#define thread_SysMonitor_Prio		    	11
#define thread_ModbusSlavePoll_Prio      	10
#define thread_ModbusMasterPoll_Prio      	 9
ALIGN(RT_ALIGN_SIZE)
//====================操作系统各线程堆栈====================================
static rt_uint8_t thread_SysMonitor_stack[512];
static rt_uint8_t thread_ModbusSlavePoll_stack[1024];
static rt_uint8_t thread_ModbusMasterPoll_stack[1024];

struct rt_thread thread_SysMonitor;
struct rt_thread thread_ModbusSlavePoll;
struct rt_thread thread_ModbusMasterPoll;

//***************************系统监控线程***************************
//函数定义: void thread_entry_SysRunLed(void* parameter)
//入口参数：无
//出口参数：无
//备    注：Editor：Armink   2013-08-02   Company: BXXJS
//******************************************************************
void thread_entry_SysMonitor(void* parameter)
{
	eMBMasterReqErrCode    errorCode = MB_MRE_NO_ERR;
	uint32_t errorCount = 0;
	uint32_t sendCount = 0;
	uint16_t i;
	
//  rt_pin_mode(3, 0);
  rt_pin_mode(4, 0);
  rt_pin_mode(5, 0);
#if 0	
	while(1)
	{
    rt_pin_write(MODBUS_MASTER_RT_CONTROL_PIN_INDEX, 1);//PIN_LOW);
		rt_thread_delay(1);
    rt_pin_write(MODBUS_MASTER_RT_CONTROL_PIN_INDEX, 0);//PIN_LOW);
		rt_thread_delay(1);
	}	
#endif
	while (1)
	{
		cpu_usage_get(&CpuUsageMajor, &CpuUsageMinor);
		usSRegHoldBuf[S_HD_CPU_USAGE_MAJOR] = CpuUsageMajor;
		usSRegHoldBuf[S_HD_CPU_USAGE_MINOR] = CpuUsageMinor;
		LED_LED1_ON;
		LED_LED2_ON;
		rt_thread_delay(DELAY_SYS_RUN_LED);
		LED_LED1_OFF;
		LED_LED2_OFF;
		rt_thread_delay(DELAY_SYS_RUN_LED);
		IWDG_Feed(); //feed the dog
		//Test Modbus Master
		//usModbusUserData[0] = (USHORT)(rt_tick_get()/10);
		//usModbusUserData[1] = (USHORT)(rt_tick_get()%10);
		//usModbusUserData[2] = (USHORT)(rt_tick_get()/3);
		//usModbusUserData[3] = (USHORT)(rt_tick_get()/4);
		//usModbusUserData[4] = (USHORT)(rt_tick_get()/5);
		//usModbusUserData[5] = (USHORT)(rt_tick_get()/6);
		
		ucModbusUserData[0] = 0x1F;
//		errorCode = eMBMasterReqReadDiscreteInputs(1,3,8,RT_WAITING_FOREVER);
//		errorCode = eMBMasterReqWriteMultipleCoils(1,3,5,ucModbusUserData,RT_WAITING_FOREVER);
//		errorCode = eMBMasterReqWriteCoil(1,8,0xFF00,RT_WAITING_FOREVER);
//		errorCode = eMBMasterReqReadCoils(1,3,8,RT_WAITING_FOREVER);
//		errorCode = eMBMasterReqReadInputRegister(1,3,2,RT_WAITING_FOREVER);
//		errorCode = eMBMasterReqWriteHoldingRegister(1,3,usModbusUserData[0],RT_WAITING_FOREVER);

// BMS
//		errorCode = eMBMasterReqReadHoldingRegister(5,35000, 1,RT_WAITING_FOREVER);	// 0, BMS -> VCU OnRequest
//		errorCode = eMBMasterReqReadHoldingRegister(5,35002,22,RT_WAITING_FOREVER);	// 1, BMS -> VCU
//		errorCode = eMBMasterReqReadHoldingRegister(5,35100, 7,RT_WAITING_FOREVER);	// 2, BMS -> VCU
//		errorCode = eMBMasterReqReadHoldingRegister(5,35800, 4,RT_WAITING_FOREVER);	// 3, BMS -> VCU OnRequest

// MCU
//		errorCode = eMBMasterReqReadHoldingRegister(3,33000, 6,RT_WAITING_FOREVER);	// 4, MCU -> VCU
//		errorCode = eMBMasterReqReadHoldingRegister(3,33800, 4,RT_WAITING_FOREVER);	// 5, MCU -> VCU OnRequest

// ICL
//		errorCode = eMBMasterReqReadHoldingRegister(7,37000, 4,RT_WAITING_FOREVER);	// 6, ICL -> VCU
//		errorCode = eMBMasterReqWriteMultipleHoldingRegister(7,37100,7,usModbusUserData,RT_WAITING_FOREVER);	// 7, VCU->ICL
//		errorCode = eMBMasterReqReadHoldingRegister(7,37800, 4,RT_WAITING_FOREVER);	// 8, ICL -> VCU OnRequest


// TEST
		errorCode = eMBMasterReqWriteMultipleHoldingRegister(5,35000,5,usModbusUserData,RT_WAITING_FOREVER);
//		errorCode = eMBMasterReqReadHoldingRegister(5,35000,5,RT_WAITING_FOREVER);
//		errorCode = eMBMasterReqReadWriteMultipleHoldingRegister(1,3,2,usModbusUserData,5,2,RT_WAITING_FOREVER);

//		for ( i = 0; i < 10; i++)
//			rt_kprintf("[%04x]",usModbusUserData[i]);
//			rt_kprintf("[%d,%d%%]\n",CpuUsageMajor, CpuUsageMinor);
		sendCount++;
		
		//记录出错次数
		if (errorCode != MB_MRE_NO_ERR) {
			errorCount++;
		}
		rt_kprintf("[%02d.%02d%%]%lu\t%lu\n",CpuUsageMajor, CpuUsageMinor, errorCount, sendCount);
		//rt_kprintf("%lu %lu\n",errorCount, sendCount);
		
	}
}

//************************ Modbus从机轮训线程***************************
//函数定义: void thread_entry_ModbusSlavePoll(void* parameter)
//入口参数：无
//出口参数：无
//备    注：Editor：Armink   2013-08-02    Company: BXXJS
//******************************************************************
void thread_entry_ModbusSlavePoll(void* parameter)
{
	eMBInit(MB_RTU, 0x01, 1, 115200,  MB_PAR_NONE);//MB_PAR_EVEN);
	eMBEnable();
	while (1)
	{
		eMBPoll();
	}
}

//************************ Modbus主机轮训线程***************************
//函数定义: void thread_entry_ModbusMasterPoll(void* parameter)
//入口参数：无
//出口参数：无
//备    注：Editor：Armink   2013-08-28    Company: BXXJS
//******************************************************************
void thread_entry_ModbusMasterPoll(void* parameter)
{
	eMBMasterInit(MB_RTU, 1, 115200,  MB_PAR_EVEN);//MB_PAR_NONE);
	eMBMasterEnable();
	while (1)
	{
		eMBMasterPoll();
	}
}

//**********************系统初始化函数********************************
//函数定义: int rt_application_init(void)
//入口参数：无
//出口参数：无
//备    注：Editor：Liuqiuhu   2013-1-31   Company: BXXJS
//********************************************************************
int rt_application_init(void)
{
#if 1
	rt_thread_init(&thread_SysMonitor, "SysMonitor", thread_entry_SysMonitor,
			RT_NULL, thread_SysMonitor_stack, sizeof(thread_SysMonitor_stack),
			thread_SysMonitor_Prio, 5);
	rt_thread_startup(&thread_SysMonitor);
#endif
	
#if 0
	rt_thread_init(&thread_ModbusSlavePoll, "MBSlavePoll",
			thread_entry_ModbusSlavePoll, RT_NULL, thread_ModbusSlavePoll_stack,
			sizeof(thread_ModbusSlavePoll_stack), thread_ModbusSlavePoll_Prio,
			5);
	rt_thread_startup(&thread_ModbusSlavePoll);
#endif
	
#if 1
	rt_thread_init(&thread_ModbusMasterPoll, "MBMasterPoll",
			thread_entry_ModbusMasterPoll, RT_NULL, thread_ModbusMasterPoll_stack,
			sizeof(thread_ModbusMasterPoll_stack), thread_ModbusMasterPoll_Prio,
			5);
	rt_thread_startup(&thread_ModbusMasterPoll);
#endif
	return 0;
}

//**************************初始化RT-Thread函数*************************************
//函数定义: void rtthread_startup(void)
//入口参数：无
//出口参数：无
//备    注：Editor：Armink 2011-04-04    Company: BXXJS
//**********************************************************************************
void rtthread_startup(void)
{
	/* init board */
	rt_hw_board_init();


	/* init tick */
	rt_system_tick_init();

	/* init kernel object */
	rt_system_object_init();

	/* init timer system */
	rt_system_timer_init();

#ifdef RT_USING_HEAP
#ifdef __CC_ARM
	rt_system_heap_init((void*)&Image$$RW_IRAM1$$ZI$$Limit, (void*)STM32_SRAM_END);
#elif __ICCARM__
	rt_system_heap_init(__segment_end("HEAP"), (void*)STM32_SRAM_END);
#else
	/* init memory system */
	rt_system_heap_init((void*)&__bss_end, (void*)STM32_SRAM_END);
#endif
#endif

#ifdef RT_USING_CONSOLE
	rt_console_set_device(CONSOLE_DEVICE);
#endif	
	
	/* show version */
	rt_show_version();
	
	/* init scheduler system */
	rt_system_scheduler_init();

	/* init all device */
	rt_device_init_all();

	/* init application */
	rt_application_init();

#ifdef RT_USING_FINSH
	/* init finsh */
	finsh_system_init();
	finsh_set_device("uart6");
#endif

	/* init timer thread */
	rt_system_timer_thread_init();

	/* init idle thread */
	rt_thread_idle_init();

	/* Add CPU usage to system */
	cpu_usage_init();

	/* start scheduler */
	rt_system_scheduler_start();

	/* never reach here */
	return;
}

