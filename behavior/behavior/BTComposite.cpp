//
//  BTComposite.cpp
//  Behavior
//
//  Created by hailong on 12-8-18.
//
//

#include "BTComposite.h"

namespace behavior {
    ////////////////////////////////////////////////////////////////////////////////
    // Composite node
    BTComposite::~BTComposite()
    {
        BTNodeIterator it = m_children.begin();
        for (; it != m_children.end(); ++it) {
            (*it)->release();
        }
    }
    
    void BTComposite::appendChildNode(BTNode* node)
    {
        if (node) {
            m_children.push_back(node);
            node->retain();
        }
    }
    
    void BTComposite::traverse(BTNodeTraveller traveller, void* param)
    {
        BTNode::traverse(traveller, param);
        BTNodeIterator it = m_children.begin();
        for (; it != m_children.end(); ++it) {
            (*it)->traverse(traveller, param);
        }
    }
    
    ////////////////////////////////////////////////////////////////////////////////
    // Sequence
    eTaskInitResult Sequence::initialize(BTTaskContext* ctx)
    {
        BTTask::initialize(ctx);
        const BTNodes& children = getNode().getChildren();
        m_itCurrent = children.begin();
        if (m_itCurrent != children.end()) {
            m_currentBehavior.setup(**m_itCurrent, ctx);
            return kReadyToUpdate; // initialize successfully
        } else {
            return kTaskIsDone; // action should return kBTSuccess
        }
    }
    
    void Sequence::terminate(eBTStatus)
    {
        m_currentBehavior.teardown();
    }
    
    eBTStatus Sequence::update(float dt)
    {
        eBTStatus status = m_currentBehavior.tick(dt);
        if (status != kBTSuccess) {
            return status;
        }
        ++m_itCurrent;
        if (m_itCurrent == getNode().getChildren().end()) {
            return kBTSuccess;
        }
        m_currentBehavior.setup(**m_itCurrent, getContext());
        return kBTRunning;
    }
    
    void Sequence::cancel()
    {
        m_currentBehavior.cancel();
    }
    
    ////////////////////////////////////////////////////////////////////////////////
    // Selector
    eTaskInitResult Selector::initialize(BTTaskContext* ctx)
    {
        BTTask::initialize(ctx);
        const BTNodes& children = getNode().getChildren();
        m_itCurrent = children.begin();
        if (m_itCurrent != children.end()) {
            m_currentBehavior.setup(**m_itCurrent, ctx);
            return kReadyToUpdate; // initialize successfully
        } else {
            return kTaskFailed; // action should return kBTFailure
        }
    }
    
    void Selector::terminate(eBTStatus)
    {
        m_currentBehavior.teardown();
    }
    
    eBTStatus Selector::update(float dt)
    {
        eBTStatus status = m_currentBehavior.tick(dt);
        if (status != kBTFailure) {
            return status;
        }
        ++m_itCurrent;
        if (m_itCurrent == getNode().getChildren().end()) {
            return kBTFailure;
        }
        m_currentBehavior.setup(**m_itCurrent, getContext());
        return kBTRunning;
    }
    
    void Selector::cancel()
    {
        m_currentBehavior.cancel();
    }
    
    ////////////////////////////////////////////////////////////////////////////////
    // Parallel nodes
    eTaskInitResult Parallel::initialize(BTTaskContext* ctx)
    {
        BTTask::initialize(ctx);
        const BTNodes& children = getNode().getChildren();
        BTNodeConstIterator it = children.begin();
        for (; it != children.end(); ++it) {
            Behavior* behavior = new Behavior(**it, ctx);
            m_currentBehaviors.push_back(behavior);
        }
        return kReadyToUpdate; // initialize successfully
    }
    
    void Parallel::terminate(eBTStatus)
    {
        BehaviorListIterator it = m_currentBehaviors.begin();
        for (; it != m_currentBehaviors.end(); ++it) {
            delete (*it);
        }
        m_currentBehaviors.clear();
    }
    
    void Parallel::cancel()
    {
        BehaviorListIterator it = m_currentBehaviors.begin();
        for (; it != m_currentBehaviors.end(); ++it) {
            (*it)->cancel();
        }
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
    
    eBTStatus ParallelSequence::update(float dt)
    {
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
    
    eBTStatus ParallelSelector::update(float dt)
    {
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
