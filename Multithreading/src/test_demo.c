/*****************************************************************/ /**
* @file test_demo.c
* @brief
* @author lysander.li@quectel.com
* @date 2026-04-08
*
* </table>
**********************************************************************/
#include "qosa_sys.h"
#include "qosa_def.h"
#include "qosa_log.h"
#include <stdio.h>

/*===========================================================================
 *  Macro Definition
 ===========================================================================*/

#define QOS_LOG_TAG   LOG_TAG_DEMO

#define QOS_LOG_TAG   LOG_TAG_DEMO

#define UniRTOS_TEST_DEMO_TASK_STACK_SIZE 1024  // Task stack size 1KB

#define UniRTOS_TEST_DEMO_TASK_PRIO QOSA_PRIORITY_NORMAL // Normal priority

static qosa_task_t UniRTOS_TASK_A = QOSA_NULL, UniRTOS_TASK_B = QOSA_NULL;


void task_A_handler(void *argv)
{
    while (1) 
    {
        QLOGI("[TEST Demo][TASK A] TASK A is running... ");
        qosa_task_sleep_ms(2000);
    }
}

void task_B_handler(void *argv)
{
    while (1) 
    {
        QLOGI("[TEST Demo][TASK B] TASK B is running... ");
        qosa_task_sleep_ms(2000);
    }
}

int unir_test_demo_init(void)
{
    qosa_int32_t status;
    int ret;
    QLOGI("[TEST Demo]enter UniRTOS TEST DEMO !!!");
    if (UniRTOS_TASK_A == QOSA_NULL && UniRTOS_TASK_B == QOSA_NULL)
    {
        qosa_task_create(&UniRTOS_TASK_A, UniRTOS_TEST_DEMO_TASK_STACK_SIZE, UniRTOS_TEST_DEMO_TASK_PRIO, "taskA", task_A_handler, QOSA_NULL);
        qosa_task_create(&UniRTOS_TASK_B, UniRTOS_TEST_DEMO_TASK_STACK_SIZE, UniRTOS_TEST_DEMO_TASK_PRIO, "taskB", task_B_handler, QOSA_NULL);
    }
    qosa_task_sleep_sec(20);
    qosa_task_get_status(UniRTOS_TASK_A, &status);
    if (status)
    {
        ret = qosa_task_delete(UniRTOS_TASK_A);
        if (ret != QOSA_OK)
        {
            QLOGE("[TEST Demo] Failed to delete task A");
        }
        else
        {
            QLOGI("[TEST Demo] Task A deleted successfully");
        }
    }
    qosa_task_get_status(UniRTOS_TASK_B, &status);
    if (status)
    {
        ret = qosa_task_delete(UniRTOS_TASK_B);
        if (ret != QOSA_OK)
        {
            QLOGE("[TEST Demo] Failed to delete task B");
        }
        else
        {
            QLOGI("[TEST Demo] Task B deleted successfully");
        }
    }
    return 0;
}

