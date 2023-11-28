/**
	�����FreeRTOSʵ��
	ʹ�ÿ�����Ұ���STM32F429ϵ�п�����
	ʵ��5-ʹ�ö�����ģ�⻥����
	ʱ�䣺23/11/28
  ******************************************************************************
  */
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "bsp_led.h"
#include "bsp_key.h" 
#include "bsp_debug_usart.h"

/*************************����������********************************/
/*������ƿ�ָ�봴��*/
TaskHandle_t AppTaskCreate_Handle = NULL;
TaskHandle_t Receive_Task_Handle = NULL;
TaskHandle_t Send_Task_Handle = NULL;

/*************************�ں˾������********************************/
QueueHandle_t Test_Queue_Handle = NULL;

/****************************��������********************************/
static void AppTaskCreate(void);
static void Receive_Test_Task(void * param);
static void Send_Test_Task(void * param);

static void GetUARTLock(void);
static void PutUARTLock(void);
/******************************�����ڱ������ĺ궨��*******************************/
/*���еĳ��ȣ����ɰ������ٸ���Ϣ*/
#define  QUEUE_LEN    4   
/*������ÿ����Ϣ��С*/
#define  QUEUE_SIZE   4   

/**
  * @brief  ������
  * @param  ��
  * @retval ��
  */
int main(void)
{
	/*������Ϣ��ֵ֤����ʼĬ��ΪpdPASS*/
	BaseType_t xReturn = pdPASS;
	/*LED �˿ڳ�ʼ��*/
	LED_GPIO_Config();
	/*���ڳ�ʼ��*/
	Debug_USART_Config();
	/*��ʼ������*/
	Key_GPIO_Config();
	
	LED_RGBOFF;
	
	printf("FreeRTOSʵ��3,����ʵ�飬���߼���\n");
	
	//���������������������
	xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,			/* ������ں��� */
						(const char*	)"AppTaskCreate",			/* �������� */
						(uint16_t		)256,   					/* ����ջ��С */
						(void*			)NULL,						/* ������ں������� */
						(UBaseType_t	)1,	    					/* ��������ȼ� */
						(TaskHandle_t*	)&AppTaskCreate_Handle);	/* ������ƿ�ָ�� */
	
	/*�����������*/
	if(xReturn == pdPASS)
	{
		/*�������������*/
		vTaskStartScheduler();
	}
	else
	{
		printf("�������ʧ�ܣ��������\n");
		return -1;
	}
	
	/*������˵����ִ�е�����*/
	while(1);
}

/**************************************************************************************/
static void AppTaskCreate(void)
{
	uint32_t val = 1;
	/*������Ϣ��ֵ֤����ʼĬ��ΪpdPASS*/
	BaseType_t xReturn = pdPASS;
	
	/*�����ٽ�α���*/
	taskENTER_CRITICAL();
	
	/* ����Test_Queue_Handle */
	Test_Queue_Handle = xQueueCreate((UBaseType_t ) QUEUE_LEN,/* ��Ϣ���еĳ��� */
							(UBaseType_t ) QUEUE_SIZE);/* ��Ϣ�Ĵ�С */
	if(NULL != Test_Queue_Handle)
	{
		printf("����Test_Queue_Handle��Ϣ���гɹ�!\r\n");
	}

	/*�Զ��н���ռ��*/
	xQueueSend(Test_Queue_Handle, &val, portMAX_DELAY);
	
	//������������
	xReturn = xTaskCreate((TaskFunction_t )Receive_Test_Task,	/* ������ں��� */
						(const char*	)"Receive_Test_Task",	/* �������� */
						(uint16_t		)512,					/* ����ջ��С */
						(void*			)"�������н��ܺ���",	/* ������ں������� */
						(UBaseType_t	)2,						/* ��������ȼ� */
						(TaskHandle_t*	)&Receive_Task_Handle);	/* ������ƿ�ָ�� */
	
	/*�鿴�����Ƿ񴴽��ɹ�*/
	if(xReturn == pdPASS)
	{
		printf("�������񴴽��ɹ�\n");
	}
	
	//������������
	xReturn = xTaskCreate((TaskFunction_t )Send_Test_Task, 		/* ������ں��� */
						(const char*	)"Send_Test_Task",		/* �������� */
						(uint16_t		)512,					/* ����ջ��С */
						(void*			)"�������з��ͺ���",	/* ������ں������� */
						(UBaseType_t	)3,						/* ��������ȼ� */
						(TaskHandle_t*	)&Send_Task_Handle);	/* ������ƿ�ָ�� */
							
	/*�鿴�����Ƿ񴴽��ɹ�*/
	if(xReturn == pdPASS)
	{
		printf("�������񴴽��ɹ�\n");
	}
	
	/*ɾ������*/
	/*���������ҳ�����ֱ������NULL��ɾ���������������ϾͻῨ����prvTaskExitError��������У��ܹ�*/
	vTaskDelete(AppTaskCreate_Handle);
	
	taskEXIT_CRITICAL();
}

static void Receive_Test_Task(void * param)
{
	while(1)
	{
		/*��ȡ�䵱�������Ķ���*/
		GetUARTLock();
		/*��������*/
		printf("%s\r\n", (char *)param);
		/*�ͷų䵱�������Ķ���*/
		PutUARTLock(); 
		/*��ʱ20��tick*/
		vTaskDelay(20);
	}
}

void Send_Test_Task(void * param)
{
	while(1)
	{
		/*��ȡ�䵱�������Ķ���*/
		GetUARTLock();
		/*��������*/
		printf("%s\r\n", (char *)param);
		/*�ͷų䵱�������Ķ���*/
		PutUARTLock(); 
		/*��ʱ20��tick*/
		vTaskDelay(20);
	}
}

static void GetUARTLock(void)
{	
	/*������Ϣ��ֵ֤����ʼĬ��ΪpdPASS*/
	BaseType_t xReturn = pdPASS;
	int val = 1;
	xQueueReceive(Test_Queue_Handle, &val, portMAX_DELAY);
	if(pdPASS == xReturn)
	{
		printf("�����ͷųɹ�\r\n");
	}
}

static void PutUARTLock(void)
{	
	/*������Ϣ��ֵ֤����ʼĬ��ΪpdPASS*/
	BaseType_t xReturn = pdPASS;
	int val = 0;
	xQueueSend(Test_Queue_Handle, &val, portMAX_DELAY);
	if(pdPASS == xReturn)
	{
		printf("�������»�ȡ\r\n");
	}
}




/*********************************************END OF FILE**********************/

