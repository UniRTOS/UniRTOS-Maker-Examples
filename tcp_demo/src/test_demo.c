/*****************************************************************/ /**
* @file qcm_socket_block_demo.c
* @brief
* @author harry.li@quectel.com
* @date 2025-05-7
*
* @copyright Copyright (c) 2023 Quectel Wireless Solution, Co., Ltd.
* All Rights Reserved. Quectel Wireless Solution Proprietary and Confidential.
*
* @par EDIT HISTORY FOR MODULE
* <table>
* <tr><th>Date <th>Version <th>Author <th>Description
* <tr><td>2025-05-7 <td>1.0 <td>harry.li <td> Init
* </table>
**********************************************************************/
#include "qosa_def.h"
#include "qosa_sys.h"
#include "qosa_log.h"
#include "qcm_socket_adp.h"
#include "qosa_asyn_dns.h"

#include "qosa_datacall.h"

#define QOS_LOG_TAG                       LOG_TAG

#define SOCKET_BLOCK_DEMO_TASK_STACK_SIZE 4096
#define SOCKET_BLOCK_DEMO_TASK_PRIO       QOSA_PRIORITY_NORMAL

#define SOCKET_BLOCK_CONNECT_SERVER_ADDR  "101.37.104.185"
#define SOCKET_BLOCK_CONNECT_SERVER_PORT  46329
#define SOCKET_BLOCK_CONNECT_SERVER_NAME  "101.37.104.185"
#define SOCKET_BLOCK_BUFF_MAX_LEN         1024

#define SOCKET_BLOCK_DEMO_SIMID           0
#define SOCKET_BLOCK_DEMO_PDPID           1
#define SOCKET_BLOCK_DEMO_ACTIVE_TIMEOUT  30  // PDP activation timeout 30s

/**
 * @brief Check and activate PDP
 *
 * This function is used to check the data connection status of the specified SIM card and PDP context,
 * and perform synchronous activation if not activated.
 * Uses predefined SIMID and PDPID parameters to establish data connection.
 *
 * @return int Execution result
 * @retval 0  Successfully activated data connection or connection is already active
 * @retval -1 Failed to activate data connection
 */
static int qcm_socket_app_datacall_active(void)
{
    /* Define data connection related variables */
    qosa_datacall_conn_t    conn = 0;
    qosa_datacall_ip_info_t info = {0};
    qosa_datacall_errno_e   ret = 0;

    /* Create new data connection object */
    conn = qosa_datacall_conn_new(SOCKET_BLOCK_DEMO_SIMID, SOCKET_BLOCK_DEMO_PDPID, QOSA_DATACALL_CONN_TCPIP);

    /* Check data connection information to determine if PDP is activated */
    if (QOSA_DATACALL_ERR_NO_ACTIVE == qosa_datacall_get_ip_info(conn, &info))
    {
        QLOGI("[TEST DEMO]sim_id=%d,pdp_id=%d", SOCKET_BLOCK_DEMO_SIMID, SOCKET_BLOCK_DEMO_PDPID);

        /* If PDP is not activated, start synchronous activation process */
        ret = qosa_datacall_start(conn, SOCKET_BLOCK_DEMO_ACTIVE_TIMEOUT);
        if (QOSA_DATACALL_OK != ret)
        {
            QLOGE("datacall ret=%x", ret);
            return -1;
        }
    }
    return 0;
}

/**
 * @brief Blocking socket application processing function for executing TCP client communication flow.
 *
 * This function first waits for a period of time for network registration to complete, then activates data connection (such as PDP),
 * then obtains server address through DNS resolution, creates blocking TCP socket and attempts to connect to server.
 * After successful connection, loops to send and receive data, and finally closes the socket.
 *
 * @param argv Unused parameter, reserved to comply with function pointer interface requirements.
 * @return No return value.
 */
static void qcm_socket_app_block_process(void *argv)
{
    QOSA_UNUSED(argv);
    int                    sockfd = -1;
    int                    ret = 0;
    char                   buff[SOCKET_BLOCK_BUFF_MAX_LEN] = {0};
    struct qosa_addrinfo_s hints, *rp, *result;
    qosa_ip_addr_t         remote_ip = {0};

    hints.ai_family = QCM_AF_INET;

    // Wait for a period of time before starting demo, convenient for network registration and log capture
    qosa_task_sleep_sec(10);

    // Activate data connection (such as PDP), exit if failed
    if (qcm_socket_app_datacall_active() != 0)
    {
        QLOGE("[TEST DEMO]pdp error");
        return;
    }

    // Perform DNS resolution to get server address information
    if (qosa_dns_syn_getaddrinfo(SOCKET_BLOCK_DEMO_SIMID, SOCKET_BLOCK_DEMO_PDPID, SOCKET_BLOCK_CONNECT_SERVER_NAME, &hints, &result) != QOSA_DNS_RESULT_OK)
    {
        QLOGE("[TEST DEMO]dns_syn_getaddrinfo error");
        return;
    }
    else
    {
        QLOGI("[TEST DEMO]dns_syn_getaddrinfo success");

        // Traverse DNS returned address list, attempt to establish TCP connection
        for (rp = result; rp != QOSA_NULL; rp = rp->ai_next)
        {
            // Use qcm_socket_create to create blocking socket
            sockfd = qcm_socket_create(0, 1, rp->ai_family, QCM_SOCK_STREAM, QCM_TCP_PROTOCOL, 0, QOSA_TRUE);
            if (sockfd < 0)
            {
                QLOGE("qcm_socket_create error");
                qosa_dns_result_free(result);
                return;
            }

            qosa_memset(&remote_ip, 0, sizeof(qosa_ip_addr_t));
            if (hints.ai_family == QCM_AF_INET)
            {
                // Handle IPv4 address
                inet_pton(AF_INET, rp->ip_addr, &remote_ip.addr.ipv4_addr);
                remote_ip.ip_vsn = QOSA_PDP_IPV4;
            }
            QLOGI("[TEST DEMO]ip--->%s port=%d", rp->ip_addr, SOCKET_BLOCK_CONNECT_SERVER_PORT);

            // Use blocking method to establish TCP connection
            ret = qcm_socket_connect(sockfd, &remote_ip, SOCKET_BLOCK_CONNECT_SERVER_PORT);
            if (ret == 0)
            {
                QLOGD("[TEST DEMO]qcm_socket_connect success");
                break;
            }
            else
            {
                qcm_socket_close(sockfd);
                sockfd = -1;
            }
        }

        // Free DNS resolution result memory
        qosa_dns_result_free(result);
    }

    QLOGD("[TEST DEMO]sockfd=%d", sockfd);

    // Send and receive data, execute up to 20 times
    while (1)
    {
        static int i = 0;
        i++;
        if (i > 20)
        {
            QLOGD("i = %d", i);
            break;
        }

        // Construct send buffer content
        qosa_memset(buff, i, SOCKET_BLOCK_BUFF_MAX_LEN);
        qosa_snprintf(buff, SOCKET_BLOCK_BUFF_MAX_LEN, "%s,%d", "abcdefg:", i);

        // Send data
        ret = qcm_socket_send(sockfd, buff, qosa_strlen(buff));
        if (ret <= 0)
        {
            QLOGE("[TEST DEMO]qcm_socket_send error");
            break;
        }

        // Receive server response data, can set up own TCP server for send/receive data testing
        ret = qcm_socket_read(sockfd, buff, SOCKET_BLOCK_BUFF_MAX_LEN);
        if (ret <= 0)
        {
            QLOGE("[TEST DEMO]qcm_socket_read error");
            break;
        }

        QLOGI("[TEST DEMO]ret = %d buff=%s", ret, buff);
    }

    // Close socket connection
    qcm_socket_close(sockfd);
}

/**
 * @brief Verify socket connection using blocking socket
 */
void unir_test_demo_init(void)
{
    int         err = 0;
    qosa_task_t app_task = QOSA_NULL;

    err = qosa_task_create(&app_task, SOCKET_BLOCK_DEMO_TASK_STACK_SIZE, SOCKET_BLOCK_DEMO_TASK_PRIO, "app_block", qcm_socket_app_block_process, QOSA_NULL);
    if (err != QOSA_OK)
    {
        QLOGE("[TEST DEMO]app_task task create error");
        return;
    }
}
