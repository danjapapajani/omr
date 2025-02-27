/*******************************************************************************
 * Copyright (c) 1991, 2022 IBM Corp. and others
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at https://www.eclipse.org/legal/epl-2.0/
 * or the Apache License, Version 2.0 which accompanies this distribution and
 * is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * This Source Code may also be made available under the following
 * Secondary Licenses when the conditions for such availability set
 * forth in the Eclipse Public License, v. 2.0 are satisfied: GNU
 * General Public License, version 2 with the GNU Classpath
 * Exception [1] and GNU General Public License, version 2 with the
 * OpenJDK Assembly Exception [2].
 *
 * [1] https://www.gnu.org/software/classpath/license.html
 * [2] http://openjdk.java.net/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0 WITH Classpath-exception-2.0 OR LicenseRef-GPL-2.0 WITH Assembly-exception
 *******************************************************************************/


#if !defined(MEMORYSUBSPACESEMISPACE_HPP_)
#define MEMORYSUBSPACESEMISPACE_HPP_

#include "omrcfg.h"
#include "omrmodroncore.h"

#if defined(OMR_GC_MODRON_SCAVENGER)

#include "MemorySubSpace.hpp"

class MM_AllocateDescription;
class MM_EnvironmentBase;
class MM_HeapStats;
class MM_MemoryPool;
class MM_MemorySpace;
class MM_PhysicalSubArena;
class MM_ObjectAllocationInterface;

#define MODRON_SURVIVOR_SPACE_RATIO_DEFAULT 50

/**
 * @todo Provide class documentation
 * @ingroup GC_Base
 */
class MM_MemorySubSpaceSemiSpace : public MM_MemorySubSpace
{
	/*
	 * Data members
	 */
public:
	enum Flip_step { 
		set_evacuate,
		set_allocate,
		disable_allocation,
		restore_allocation,
		restore_allocation_and_set_survivor,
		backout,
		restore_tilt_after_percolate
	};	 
private:
	MM_MemorySubSpace *_memorySubSpaceAllocate;
	MM_MemorySubSpace *_memorySubSpaceSurvivor;
	MM_MemorySubSpace *_memorySubSpaceEvacuate;

	void *_allocateSpaceBase, *_allocateSpaceTop;
	void *_survivorSpaceBase, *_survivorSpaceTop;

	uintptr_t _survivorSpaceSizeRatio;

	uintptr_t _previousBytesFlipped;
	uintptr_t _tiltedAverageBytesFlipped;
	uintptr_t _tiltedAverageBytesFlippedDelta;

	double _averageScavengeTimeRatio;
	uint64_t _lastGCEndTime;

	double _desiredSurvivorSpaceRatio;
#if defined(OMR_GC_CONCURRENT_SCAVENGER)
	uintptr_t _bytesAllocatedDuringConcurrent;
	uintptr_t _avgBytesAllocatedDuringConcurrent;
	float _deviationBytesAllocatedDuringConcurrent;
	float _avgDeviationBytesAllocatedDuringConcurrent;
#endif /* OMR_GC_CONCURRENT_SCAVENGER */

	MM_LargeObjectAllocateStats *_largeObjectAllocateStats; /**< Approximate allocation profile for large objects. Struct to keep merged stats from two allocate pools */

protected:
public:
	
	/*
	 * Function members
	 */
private:
	bool initialize(MM_EnvironmentBase *env);
	virtual void tearDown(MM_EnvironmentBase *env);

	void tilt(MM_EnvironmentBase *env, uintptr_t allocateSpaceSize, uintptr_t survivorSpaceSize);
	void tilt(MM_EnvironmentBase *env, uintptr_t survivorSpaceSizeRatioRequest);

	void checkSubSpaceMemoryPostCollectTilt(MM_EnvironmentBase *env);
	void checkSubSpaceMemoryPostCollectResize(MM_EnvironmentBase *env);

protected:
	virtual void *allocationRequestFailed(MM_EnvironmentBase *env, MM_AllocateDescription *allocateDescription, AllocationType allocationType, MM_ObjectAllocationInterface *objectAllocationInterface, MM_MemorySubSpace *baseSubSpace, MM_MemorySubSpace *previousSubSpace);

public:
	static MM_MemorySubSpaceSemiSpace *newInstance(MM_EnvironmentBase *env, MM_Collector *collector, MM_PhysicalSubArena *physicalSubArena, MM_MemorySubSpace *memorySubSpaceAllocate, MM_MemorySubSpace *memorySubSpaceSurvivor, bool usesGlobalCollector, uintptr_t minimumSize, uintptr_t initialSize, uintptr_t maximumSize);

	virtual const char *getName() { return MEMORY_SUBSPACE_NAME_SEMISPACE; }
	virtual const char *getDescription() { return MEMORY_SUBSPACE_DESCRIPTION_SEMISPACE; }

	virtual void *allocateObject(MM_EnvironmentBase *env, MM_AllocateDescription *allocDescription, MM_MemorySubSpace *baseSubSpace, MM_MemorySubSpace *previousSubSpace, bool shouldCollectOnFailure);
	virtual void *allocateArrayletLeaf(MM_EnvironmentBase *env, MM_AllocateDescription *allocDescription, MM_MemorySubSpace *baseSubSpace, MM_MemorySubSpace *previousSubSpace, bool shouldCollectOnFailure);
	
	virtual void *allocateTLH(MM_EnvironmentBase *env, MM_AllocateDescription *allocDescription, MM_ObjectAllocationInterface *objectAllocationInterface, MM_MemorySubSpace *baseSubSpace, MM_MemorySubSpace *previousSubSpace, bool shouldCollectOnFailure);

	virtual MM_MemorySubSpace *getDefaultMemorySubSpace();

	bool isObjectInEvacuateMemory(omrobjectptr_t objectPtr);
	bool isObjectInNewSpace(omrobjectptr_t objectPtr);
	bool isObjectInNewSpace(void *objectBase, void *objectTop);

	uintptr_t getMaxSpaceForObjectInEvacuateMemory(omrobjectptr_t objectPtr);

	void mainSetupForGC(MM_EnvironmentBase *envBase);
	void mainTeardownForSuccessfulGC(MM_EnvironmentBase* env);
	void mainTeardownForAbortedGC(MM_EnvironmentBase *env);

	void poisonEvacuateSpace();

	void cacheRanges(MM_MemorySubSpace *subSpace, void **base, void **top);

	MM_MemorySubSpace *getTenureMemorySubSpace() { 	return _parent->getTenureMemorySubSpace(); }
	MM_MemorySubSpace *getMemorySubSpaceAllocate() { return _memorySubSpaceAllocate; };
	MM_MemorySubSpace *getMemorySubSpaceSurvivor() { return _memorySubSpaceSurvivor; };

	virtual bool isChildActive(MM_MemorySubSpace *memorySubSpace);

	virtual uintptr_t getActualActiveFreeMemorySize();
	virtual uintptr_t getApproximateActiveFreeMemorySize();
	virtual uintptr_t getActiveMemorySize(uintptr_t includeMemoryType);
	virtual uintptr_t getActualActiveFreeMemorySize(uintptr_t includeMemoryType);
	virtual uintptr_t getApproximateActiveFreeMemorySize(uintptr_t includeMemoryType);
	virtual uintptr_t getActiveSurvivorMemorySize(uintptr_t includeMemoryType);
	virtual uintptr_t getApproximateActiveFreeSurvivorMemorySize(uintptr_t includeMemoryType);
	virtual uintptr_t getActualActiveFreeSurvivorMemorySize(uintptr_t includeMemoryType);
	
	virtual void abandonHeapChunk(void *addrBase, void *addrTop);
	
	virtual	void mergeHeapStats(MM_HeapStats *heapStats);
	virtual	void mergeHeapStats(MM_HeapStats *heapStats, uintptr_t includeMemoryType);
	virtual void systemGarbageCollect(MM_EnvironmentBase *env, uint32_t gcCode);

	/* Type specific methods */
	void flip(MM_EnvironmentBase *env, Flip_step action);
	
	MMINLINE uintptr_t getSurvivorSpaceSizeRatio() const { return _survivorSpaceSizeRatio; }
	MMINLINE void setSurvivorSpaceSizeRatio(uintptr_t size) { _survivorSpaceSizeRatio = size; }
	
	virtual void checkResize(MM_EnvironmentBase *env, MM_AllocateDescription *allocDescription = NULL, bool systemGC = false);
	virtual intptr_t performResize(MM_EnvironmentBase *env, MM_AllocateDescription *allocDescription = NULL);

	virtual MM_LargeObjectAllocateStats *getLargeObjectAllocateStats();

	/**
	 * Merge the stats from children memory subSpaces
	 * Not thread safe - caller has to make sure no other threads are modifying the stats for any of children.
	 */
	virtual void mergeLargeObjectAllocateStats(MM_EnvironmentBase *env);

	virtual uintptr_t releaseFreeMemoryPages(MM_EnvironmentBase* env);

	/**
	 * Create a MemorySubSpaceSemiSpace object.
	 */
	MM_MemorySubSpaceSemiSpace(MM_EnvironmentBase *env, MM_Collector *collector, MM_PhysicalSubArena *physicalSubArena, MM_MemorySubSpace *memorySubSpaceAllocate, MM_MemorySubSpace *memorySubSpaceSurvivor, bool usesGlobalCollector, uintptr_t minimumSize, uintptr_t initialSize, uintptr_t maximumSize) :
		MM_MemorySubSpace(env, collector, physicalSubArena, usesGlobalCollector, minimumSize, initialSize, maximumSize, MEMORY_TYPE_NEW, 0)
		,_memorySubSpaceAllocate(memorySubSpaceAllocate)
		,_memorySubSpaceSurvivor(memorySubSpaceSurvivor)
		,_memorySubSpaceEvacuate(memorySubSpaceSurvivor)
		,_allocateSpaceBase(NULL)
		,_allocateSpaceTop(NULL)
		,_survivorSpaceBase(NULL)
		,_survivorSpaceTop(NULL)
		,_survivorSpaceSizeRatio(MODRON_SURVIVOR_SPACE_RATIO_DEFAULT)
		,_previousBytesFlipped(0)
		,_tiltedAverageBytesFlipped(0)
		,_tiltedAverageBytesFlippedDelta(0)
		,_averageScavengeTimeRatio(0.0)
		,_lastGCEndTime(0)
		,_desiredSurvivorSpaceRatio(0.0)
#if defined(OMR_GC_CONCURRENT_SCAVENGER)		
		,_bytesAllocatedDuringConcurrent(0)
		,_avgBytesAllocatedDuringConcurrent(0)
		,_deviationBytesAllocatedDuringConcurrent(0)
		,_avgDeviationBytesAllocatedDuringConcurrent(0)
#endif /* OMR_GC_CONCURRENT_SCAVENGER */ 		
	{
		_typeId = __FUNCTION__;
	}
};

#endif /* OMR_GC_MODRON_SCAVENGER */

#endif /* MEMORYSUBSPACESEMISPACE_HPP_ */
