//
//  Behavior.cpp
//  Behavior
//
//  Created by hailong on 12-8-18.
//
//

#include "BTNode.h"

namespace behavior {
#if defined(BT_USE_BEHAVIOR_POOL)
    // TODO implement object pool here
    void* Behavior::operator new (size_t size)
    {
//        return allocate_from_pool(size);
        return ::operator new (size);
    }
    
    void Behavior::operator delete (void *p)
    {
//        free_to_pool(p); // return memory to pool
        ::operator delete (p);
    }
    
    void* Behavior::operator new[] (size_t size)
    {
        return ::operator new[] (size);
    }
    
    void Behavior::operator delete[] (void* p)
    {
        ::operator delete[] (p);
    }
#endif
    
    void Behavior::setup(BTNode& node, BTTaskContext* ctx)
    {
        teardown();
        m_pNode = &node;
        m_pNode->retain();
        m_pTask = node.createTask();
        m_bTerminated = false;
        m_eStatus = kBTInvalid;
        m_pContext = ctx;
    }
    
    void Behavior::teardown()
    {
        if (m_pTask == NULL)
            return;
        
        //assert(m_eStatus != kBTRunning);
        m_pNode->destroyTask(m_pTask);
        m_pNode->release();
        m_pNode = NULL;
        m_pTask = NULL;
        m_pContext = NULL;
    }
    
    eBTStatus Behavior::tick(float dt)
    {
        if (m_bTerminated) {
            return m_eStatus;
        }
        
        eTaskInitResult initRet = kReadyToUpdate;
        if (m_eStatus == kBTInvalid) {
            initRet = m_pTask->initialize(m_pContext);
        }
        
        if (initRet == kReadyToUpdate) {
            m_eStatus = m_pTask->update(dt);
        } else {
            m_eStatus = initRet == kTaskIsDone ? kBTSuccess: kBTFailure;
        }

        if (m_eStatus != kBTRunning) {
            m_pTask->terminate(m_eStatus);
            m_bTerminated = true;
        }
        
        return m_eStatus;
    }
    
    void Behavior::cancel()
    {
        if (!m_bTerminated) {
            if (m_pTask) {
                m_pTask->cancel();
                // m_pTask->terminate(m_eStatus);
            }
            m_bTerminated = true;
            if (m_eStatus == kBTRunning) {
                m_eStatus = kBTFailure;
            }
        }
    }
} /* end of namespace behavior */