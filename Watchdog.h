/*
 * Copyright (c) 2018 Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MBED_WATCHDOG_H
#define MBED_WATCHDOG_H

#include "mbed_api.h"


class Watchdog   {
public:

    /** Get a reference to the single Watchdog instance in the system.
     *
     * @return A reference to the single Watchdog instance present in the system.
     */
    static Watchdog &get_instance()
    {
        // Use this implementation of singleton (Meyer's) rather than the one that allocates
        // the instance on the heap because it ensures destruction at program end (preventing warnings
        // from memory checking tools, such as valgrind).
        static Watchdog instance;
        return instance;
    }

    /** Start the Watchdog timer with the maximum timeout value available for
     * the target.
     *
     * @note The timeout is set to a value returned by Watchdog::get_max_timeout.
     *
     * If the Watchdog timer is already running, this function does nothing.
     *
     * @return true if the Watchdog timer was started successfully;
     *         false if the Watchdog timer was not started or if the Watchdog
     *         timer is already running.
     */
    bool start(){
    	return false;
    }

    /** Start the Watchdog timer.
     *
     * @note Asset that the timeout param is supported by the target
     * (0 < timeout <= Watchdog::get_max_timeout).
     *
     * @param timeout Watchdog timeout in milliseconds.
     *
     * @return true if the Watchdog timer was started successfully;
     *         false if Watchdog timer was not started or if the Watchdog
     *         timer is already running.
     */
    bool start(uint32_t timeout){
    	return false;
    }

    /** Stop the Watchdog timer.
     *
     * Calling this function disables a running Watchdog
     * peripheral if the platform supports it.
     *
     * @return true if the Watchdog timer was successfully stopped;
     *         false if the Watchdog timer cannot be disabled on this platform
     *         or if the Watchdog timer has not been started.
     */
    bool stop(){
    	return false;
    }

    /** Get the Watchdog timer refresh value.
     *
     * This function returns the refresh timeout of the watchdog peripheral.
     *
     * @return Reload value for the Watchdog timer in milliseconds.
     */
    uint32_t get_timeout() const{
    	return 0;
    }

    /** Get the maximum Watchdog refresh value for this platform.
     *
     * @return Maximum reload value supported by the Watchdog timer for this
     *         platform in milliseconds.
     */
    uint32_t get_max_timeout() const{
    	return 0;
    }

    /** Check if the Watchdog timer is already running.
     *
     * @return true if the Watchdog timer is running and
     *         false otherwise.
     */
    bool is_running() const{
    	return false;
    }

    /** Refresh the Watchdog timer.
     *
     * Call this function periodically before the Watchdog times out.
     * Otherwise, the system resets.
     *
     * If the Watchdog timer is not running, this function does nothing.
     */
    void kick(){

    }

private:
    Watchdog(){

    }
    ~Watchdog(){

    }

    bool _running;
};


#endif // MBED_WATCHDOG_H
