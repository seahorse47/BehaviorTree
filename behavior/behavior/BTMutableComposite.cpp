//
//  BTMutableComposite.cpp
//  Behavior
//
//  Created by hailong on 13-2-26.
//
//

#include "BTMutableComposite.h"

namespace behavior {
    ////////////////////////////////////////////////////////////////////////////////
    // Mutable Composite node
    
    BTMutableComposite::BTMutableComposite()
    {
        m_pCurrent = NULL;
    }
    
    BTMutableComposite::~BTMutableComposite()
    {
        discardCurrent();
    }
    
    void BTMutableComposite::discardCurrent()
    {
        if (m_pCurrent) {
            m_pCurrent->release();
            m_pCurrent = NULL;
        }
    }
    
    BTNode* BTMutableComposite::next() {
        BTNode* node = NULL;
        discardCurrent();
        while (node == NULL && m_children.size() > 0) {
            node = m_children.front();
            m_children.pop_front();
            m_pCurrent = node;
        }
        return node;
    }
    
    ////////////////////////////////////////////////////////////////////////////////
    // Mutable Sequence
    eTaskInitResult MutableSequence::initialize(BTTaskContext* ctx)
    {
        BTTask::initialize(ctx);
        BTNode* next = getNode().next();
        if (next) {
            m_currentBehavior.setup(*next, ctx);
            return kReadyToUpdate; // initialize successfully
        } else {
            return kTaskIsDone; // action should return kBTSuccess
        }
    }
    
    void MutableSequence::terminate(eBTStatus)
    {
        m_currentBehavior.teardown();
    }
    
    eBTStatus MutableSequence::update(float dt)
    {
        eBTStatus status = m_currentBehavior.tick(dt);
        if (status != kBTSuccess) {
            return status;
        }
        BTNode* next = getNode().next();
        if (next == NULL) {
            return kBTSuccess;
        }
        m_currentBehavior.setup(*next, getContext());
        return kBTRunning;
    }
    
    void MutableSequence::cancel()
    {
        m_currentBehavior.cancel();
    }
    
    ////////////////////////////////////////////////////////////////////////////////
    // Selector
    eTaskInitResult MutableSelector::initialize(BTTaskContext* ctx)
    {
        BTTask::initialize(ctx);
        BTNode* next = getNode().next();
        if (next) {
            m_currentBehavior.setup(*next, ctx);
            return kReadyToUpdate; // initialize successfully
        } else {
            return kTaskFailed; // action should return kBTFailure
        }
    }
    
    void MutableSelector::terminate(eBTStatus)
    {
        m_currentBehavior.teardown();
    }
    
    eBTStatus MutableSelector::update(float dt)
    {
        eBTStatus status = m_currentBehavior.tick(dt);
        if (status != kBTFailure) {
            return status;
        }
        BTNode* next = getNode().next();
        if (next == NULL) {
            return kBTFailure;
        }
        m_currentBehavior.setup(*next, getContext());
        return kBTRunning;
    }
    
    void MutableSelector::cancel()
    {
        m_currentBehavior.cancel();
    }
    
    ////////////////////////////////////////////////////////////////////////////////
    // Parallel nodes
    eTaskInitResult MutableParallel::initialize(BTTaskContext* ctx)
    {
        BTTask::initialize(ctx);
        // setupNewTasks(); //
        return kReadyToUpdate; // initialize successfully
    }
    
    void MutableParallel::terminate(eBTStatus)
    {
        BehaviorListIterator it = m_currentBehaviors.begin();
        for (; it != m_currentBehaviors.end(); ++it) {
            delete (*it);
        }
        m_currentBehaviors.clear();
    }
    
    void MutableParallel::cancel()
    {
        BehaviorListIterator it = m_currentBehaviors.begin();
        for (; it != m_currentBehaviors.end(); ++it) {
            (*it)->cancel();
        }
    }
    
    void MutableParallel::setupNewTasks() {
        BTNode* next = getNode().next();
        while (next) {
            Behavior* behavior = new Behavior(*next, getContext());
            m_currentBehaviors.push_back(behavior);
        }
    }
    
    void MutableParallel::cleanupStoppedTasks() {
        BehaviorListIterator it = m_currentBehaviors.begin();
        for (; it != m_currentBehaviors.end(); ) {
            Behavior* behavior = *it;
            if (behavior && !behavior->isRunning()) {
                delete behavior;
                it = m_currentBehaviors.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    eBTStatus MutableParallel::update(float dt)
    {
        setupNewTasks();
        eBTStatus status = doUpdate(dt);
        cleanupStoppedTasks();
        return status;
    }
    
    static void cancelRunningTasks(BehaviorList& behaviors)
    {
        BehaviorListIterator it = behaviors.begin();
        for (; it != behaviors.end(); ++it) {
            if ((*it)->isRunning()) {
                (*it)->cancel();
            }
        }
    }
    
    eBTStatus MutableParallelSequence::doUpdate(float dt)
    {
        setupNewTasks();
        
        int rets[4] = { 0, 0, 0, 0 };
        BehaviorListIterator it = m_currentBehaviors.begin();
        for (; it != m_currentBehaviors.end(); ++it) {
            eBTStatus status = (*it)->tick(dt);
            rets[status]++;
        }
        if (rets[kBTSuccess] > 0) {
            cancelRunningTasks(m_currentBehaviors);
            return kBTSuccess;
        }
        if (rets[kBTRunning] > 0) {
            return kBTRunning;
        }
        return kBTFailure;
    }
    
    eBTStatus MutableParallelSelector::doUpdate(float dt)
    {
        setupNewTasks();
        
        int rets[4] = { 0, 0, 0, 0 };
        BehaviorListIterator it = m_currentBehaviors.begin();
        for (; it != m_currentBehaviors.end(); ++it) {
            eBTStatus status = (*it)->tick(dt);
            rets[status]++;
        }
        if (rets[kBTFailure] > 0) {
            cancelRunningTasks(m_currentBehaviors);
            return kBTFailure;
        }
        if (rets[kBTRunning] > 0) {
            return kBTRunning;
        }
        return kBTSuccess;
    }
    
} /* end of namespace behavior */
