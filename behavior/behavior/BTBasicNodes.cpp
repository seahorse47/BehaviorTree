//
//  BTBasicNodes.cpp
//  Behavior
//
//  Created by hailong on 13-4-19.
//
//

#include "BTBasicNodes.h"

namespace behavior {
    
    eBTStatus BTWaitNode::Task::update(float dt) {
        if (!m_bFirstTick) {
            m_fTicks += dt;
        }
        m_bFirstTick = false;
        BTWaitNode* node = static_cast<BTWaitNode*>(m_pNode);
        if (m_fTicks < node->getTime()) {
            return kBTRunning;
        } else {
            return kBTSuccess;
        }
    }
    
    eTaskInitResult BTWaitNode::Task::initialize(BTTaskContext* ctx)
    {
        BTTask::initialize(ctx);
        m_fTicks = 0;
        m_bFirstTick = true;
        return kReadyToUpdate;
    }
    
    //
    eBTStatus BTWaitUntilNode::Task::update(float dt) {
        BTWaitUntilNode* node = (BTWaitUntilNode*)m_pNode;
        BTCondition* cond = node->getCondition();
        if (cond && !cond->isTrue()) {
            return kBTRunning;
        } else {
            return kBTSuccess;
        }
    }
    
    eTaskInitResult BTWaitUntilNode::Task::initialize(BTTaskContext* ctx)
    {
        BTTask::initialize(ctx);
        BTWaitUntilNode* node = (BTWaitUntilNode*)m_pNode;
        BTCondition* cond = node->getCondition();
        if (cond) {
            cond->beginCheckingCondition();
        }
        return kReadyToUpdate;
    }
    
} /* end of namespace behavior */
