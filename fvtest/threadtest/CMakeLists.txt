###############################################################################
# Copyright (c) 2017, 2022 IBM Corp. and others
#
# This program and the accompanying materials are made available under
# the terms of the Eclipse Public License 2.0 which accompanies this
# distribution and is available at http://eclipse.org/legal/epl-2.0
# or the Apache License, Version 2.0 which accompanies this distribution
# and is available at https://www.apache.org/licenses/LICENSE-2.0.
#
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the
# Eclipse Public License, v. 2.0 are satisfied: GNU General Public License,
# version 2 with the GNU Classpath Exception [1] and GNU General Public
# License, version 2 with the OpenJDK Assembly Exception [2].
#
# [1] https://www.gnu.org/software/classpath/license.html
# [2] http://openjdk.java.net/legal/assembly-exception.html
#
# SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0 WITH Classpath-exception-2.0 OR LicenseRef-GPL-2.0 WITH Assembly-exception
###############################################################################

omr_add_executable(omrthreadtest
	abortTest.cpp
	CEnterExit.cpp
	CMonitor.cpp
	createTest.cpp
	CThread.cpp
	joinTest.cpp
	keyDestructorTest.cpp
	lockedMonitorCountTest.cpp
	main.cpp
	ospriority.cpp
	priorityInterruptTest.cpp
	rwMutexTest.cpp
	sanityTest.cpp
	sanityTestHelper.cpp
	threadTestHelp.cpp
)

#TODO Unported makefile fragment:
#ifeq (1,$(OMR_THR_FORK_SUPPORT))
	#OBJECTS += forkResetTest forkResetRWMutexTest
#endif

target_link_libraries(omrthreadtest
	omrGtestGlue
	omrtestutil
	j9hashtable
	omrcore
	omrvmstartup
	${OMR_PORT_LIB}
	${OMR_THREAD_LIB}
)
if(OMR_OS_WINDOWS)
	target_link_libraries(omrthreadtest
		ws2_32
		shell32
		Iphlpapi
		psapi
		pdh
	)
endif()

set_property(TARGET omrthreadtest PROPERTY FOLDER fvtest)

if(OMR_OS_LINUX AND OMR_ARCH_RISCV AND CMAKE_CROSSCOMPILING)
	set(EXCLUDE_LIST "${EXCLUDE_LIST}-PriorityInterrupt.*")
endif()

if(EXCLUDE_LIST)
	set(EXCLUDE_LIST "--gtest_filter=${EXCLUDE_LIST}")
	message(WARNING "Excluded omrthread tests: ${EXCLUDE_LIST}")
endif()

omr_add_test(
	NAME threadtest
	COMMAND $<TARGET_FILE:omrthreadtest> --gtest_output=xml:${CMAKE_CURRENT_BINARY_DIR}/omrthreadtest-results.xml ${EXCLUDE_LIST}
)

omr_add_test(
	NAME threadSetAttrThreadWeightTest
	COMMAND $<TARGET_FILE:omrthreadtest> --gtest_output=xml:${CMAKE_CURRENT_BINARY_DIR}/omrthreadtest-results.xml --gtest_also_run_disabled_tests --gtest_filter=ThreadCreateTest.DISABLED_SetAttrThreadWeight
)

if(OMR_OS_LINUX)
	omr_add_test(
		NAME threadRealtimeTest
		COMMAND $<TARGET_FILE:omrthreadtest> --gtest_output=xml:${CMAKE_CURRENT_BINARY_DIR}/omrthreadtest-results.xml --gtest_filter=ThreadCreateTest.* -realtime
	)
endif()
