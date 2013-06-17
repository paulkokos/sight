/* ***** BEGIN LICENSE BLOCK *****
 * FW4SPL - Copyright (C) IRCAD, 2009-2013.
 * Distributed under the terms of the GNU Lesser General Public License (LGPL) as
 * published by the Free Software Foundation.
 * ****** END LICENSE BLOCK ****** */

#ifndef _FWMEMORY_IBUFFERMANAGER_HPP_
#define _FWMEMORY_IBUFFERMANAGER_HPP_

#include <boost/function.hpp>

#include <fwCore/base.hpp>
#include <fwCore/mt/types.hpp>

#include "fwMemory/BufferAllocationPolicy.hpp"
#include "fwMemory/config.hpp"

namespace fwMemory
{

/**
 * @brief IBufferManager Interface
 *
 * Provides interface for a buffer manager. A BufferManager is able to hook
 * BufferObjects actions an to change it's behaviors. Especially, a
 * BufferObjects can delegate allocation, reallocation and destruction to a
 * BufferManager.
 *
 * A BufferManager can be aware of BufferObjects usages thanks to lock/unlock
 * hooks.
 *
 * For each hook, the BufferManager return a boolean. If the BufferManager did
 * nothing, true is returned and the BufferObject is informed that it may behave
 * as usual. If false is returned, the BufferObject is informed that the action
 * have been delegated to the BufferManager.
 */
class FWMEMORY_CLASS_API IBufferManager : public ::fwCore::BaseObject
{
public:
    typedef void* BufferType;
    typedef const void* ConstBufferType;
    typedef BufferType* BufferPtrType;
    typedef void const * const * ConstBufferPtrType;

    typedef size_t SizeType;
    typedef boost::function< long() > LockCountFunctionType;


    fwCoreClassDefinitionsWithFactoryMacro((IBufferManager), (()), new IBufferManager );
    fwCoreAllowSharedFromThis();


    /**
     * @brief Hook called when a new BufferObject is created
     *
     * @param buffer BufferObject's buffer
     * @param lockCount BufferObject lock counter.
     *
     * @return false if the BufferManager supported the action
     */
    virtual bool registerBuffer(BufferPtrType buffer, LockCountFunctionType lockCount){
        FwCoreNotUsedMacro(buffer);
        FwCoreNotUsedMacro(lockCount);
        return true;
    }

    /**
     * @brief Hook called when a BufferObject is destroyed
     *
     * @param buffer BufferObject's buffer
     *
     * @return false if the BufferManager supported the action
     */
    virtual bool unregisterBuffer(BufferPtrType buffer){
        FwCoreNotUsedMacro(buffer);
        return true;
    }

    /**
     * @brief Hook called when an allocation is requested from a BufferObject
     *
     * @param buffer BufferObject's buffer
     * @param size requested size for allocation
     * @param policy BufferObject's allocation policy
     *
     * @return false if the BufferManager supported the action
     */
    virtual bool allocateBuffer(BufferPtrType buffer, SizeType size, ::fwMemory::BufferAllocationPolicy::sptr policy)
    {
        FwCoreNotUsedMacro(buffer);
        FwCoreNotUsedMacro(size);
        FwCoreNotUsedMacro(policy);
        return true;
    }

    /**
     * @brief Hook called when a request is made to set BufferObject's buffer from an external buffer
     *
     * @param buffer BufferObject's buffer
     * @param size requested size for allocation
     * @param policy BufferObject's allocation policy
     *
     * @return false if the BufferManager supported the action
     */
    virtual bool setBuffer(BufferPtrType buffer, SizeType size, ::fwMemory::BufferAllocationPolicy::sptr policy)
    {
        FwCoreNotUsedMacro(buffer);
        FwCoreNotUsedMacro(size);
        FwCoreNotUsedMacro(policy);
        return true;
    }

    /**
     * @brief Hook called when a reallocation is requested from a BufferObject
     *
     * @param buffer BufferObject's buffer
     * @param newSize requested size for reallocation
     *
     * @return false if the BufferManager supported the action
     */
    virtual bool reallocateBuffer(BufferPtrType buffer, SizeType newSize)
    {
        FwCoreNotUsedMacro(buffer);
        FwCoreNotUsedMacro(newSize);
        return true;
    }


    /**
     * @brief Hook called when a destruction is requested from a BufferObject
     *
     * @param buffer BufferObject's buffer
     *
     * @return false if the BufferManager supported the action
     */
    virtual bool destroyBuffer(BufferPtrType buffer)
    {
        FwCoreNotUsedMacro(buffer);
        return true;
    }


    /**
     * @brief Hook called when a request to swap two BufferObject contents is made
     *
     * @param bufA First BufferObject's buffer
     * @param bufB Second BufferObject's buffer
     *
     * @return false if the BufferManager supported the action
     */
    virtual bool swapBuffer(BufferPtrType bufA, BufferPtrType bufB)
    {
        FwCoreNotUsedMacro(bufA);
        FwCoreNotUsedMacro(bufB);
        return true;
    }


    /**
     * @brief Hook called when a BufferObject is locked
     *
     * @param buffer BufferObject's buffer
     *
     * @return false if the BufferManager supported the action
     */
    virtual bool lockBuffer(ConstBufferPtrType buffer)
    {
        FwCoreNotUsedMacro(buffer);
        return true;
    }


    /**
     * @brief Hook called when a BufferObject lock is released
     *
     * @param buffer BufferObject's buffer
     *
     * @return false if the BufferManager supported the action
     */
    virtual bool unlockBuffer(ConstBufferPtrType buffer)
    {
        FwCoreNotUsedMacro(buffer);
        return true;
    }


    /**
     * @brief Returns the current BufferManager instance
     * @note This method is thread-safe.
     */
    FWMEMORY_API static IBufferManager::sptr getCurrent();

    /**
     * @brief sets the current BufferManager instance
     *
     * @param currentManager BufferManager instance
     * @note This method is thread-safe.
     */
    FWMEMORY_API static void setCurrent( IBufferManager::sptr currentManager );


    /**
     * @brief returns an information string about BufferManager state
     */
    virtual std::string toString() const { return ""; };


protected:

    IBufferManager(){};
    virtual ~IBufferManager(){};

    FWMEMORY_API static IBufferManager::sptr s_currentManager;

    static ::fwCore::mt::ReadWriteMutex s_mutex;
};


}

#endif /* _FWMEMORY_IBUFFERMANAGER_HPP_ */