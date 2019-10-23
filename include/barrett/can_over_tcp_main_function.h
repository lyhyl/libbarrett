/**
 *	Copyright 2019-2119 Wingkou <wingkou@outlook.com>
 */

/**
 * @file can_over_tcp_main_function.h
 * @date 10/22/2019
 * @author Wingkou
 */

#ifndef CAN_OVER_TCP_MAIN_FUNCTION_H_
#define CAN_OVER_TCP_MAIN_FUNCTION_H_

#include <barrett/exception.h>
#include <barrett/products/product_manager.h>
#include <barrett/bus/bus_manager.h>
#include <barrett/systems/wam.h>

#include "can_over_tcp_socket.h"

#ifdef BARRETT_SMF_VALIDATE_ARGS
bool validate_args(int argc, char **argv);
#endif

#ifdef BARRETT_SMF_CONFIGURE_PM
bool configure_pm(int argc, char **argv, ::barrett::ProductManager &pm);
#endif

#ifndef BARRETT_SMF_NO_DECLARE
template <size_t DOF>
int wam_main(int argc, char **argv, ::barrett::ProductManager &pm, ::barrett::systems::Wam<DOF> &wam);
#endif

#ifndef BARRETT_SMF_DONT_WAIT_FOR_SHIFT_ACTIVATE
#define BARRETT_SMF_WAIT_FOR_SHIFT_ACTIVATE true
#else
#define BARRETT_SMF_WAIT_FOR_SHIFT_ACTIVATE false
#endif

#ifndef BARRETT_SMF_DONT_PROMPT_ON_ZEROING
#define BARRETT_SMF_PROMPT_ON_ZEROING true
#else
#define BARRETT_SMF_PROMPT_ON_ZEROING false
#endif

#ifndef BARRETT_SMF_WAM_CONFIG_PATH
#define BARRETT_SMF_WAM_CONFIG_PATH NULL
#endif

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cerr << "Usage: your_program_name <host> <port> [other options]\n";
        return 1;
    }

    // Give us pretty stack-traces when things die
    ::barrett::installExceptionHandler();

    // #ifdef BARRETT_SMF_VALIDATE_ARGS
    //     if (!validate_args(argc, argv))
    //     {
    //         return 1;
    //     }
    // #endif

    ::barrett::ProductManager pm(NULL, new ::barrett::bus::BusManager(new ::barrett::bus::CANOverTCPSocket(argv[1], argv[2])));
    // #ifdef BARRETT_SMF_CONFIGURE_PM
    // 	if ( !configure_pm(argc, argv, pm) ) {
    // 		return 1;
    // 	}
    // #endif

    pm.waitForWam(BARRETT_SMF_PROMPT_ON_ZEROING);
    pm.wakeAllPucks();
    // TODO(JH): Rehab Update implement and test
    if (pm.foundWam3())
    {
        return wam_main(argc, argv, pm, *pm.getWam3(BARRETT_SMF_WAIT_FOR_SHIFT_ACTIVATE, BARRETT_SMF_WAM_CONFIG_PATH));
    }
    else if (pm.foundWam4())
    {
        return wam_main(argc, argv, pm, *pm.getWam4(BARRETT_SMF_WAIT_FOR_SHIFT_ACTIVATE, BARRETT_SMF_WAM_CONFIG_PATH));
    }
    else if (pm.foundWam7())
    {
        return wam_main(argc, argv, pm, *pm.getWam7(BARRETT_SMF_WAIT_FOR_SHIFT_ACTIVATE, BARRETT_SMF_WAM_CONFIG_PATH));
    }
    else
    {
        printf(">>> ERROR: No WAM was found. Perhaps you have found a bug in ProductManager::waitForWam().\n");
        return 1;
    }
}

#endif /* BARRETT_STANDARD_MAIN_FUNCTION_H_ */
