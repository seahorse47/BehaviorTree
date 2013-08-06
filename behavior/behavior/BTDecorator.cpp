//
//  BTDecorator.cpp
//  Behavior
//
//  Created by hailong on 12-8-19.
//
//

#include "BTDecorator.h"

namespace behavior {

    /// delay task
    eBTStatus BTDelayNode::Task::update(float dt)
    {
        float delay = static_cast<BTDelayNode*>(m_pNode)->getDelay();
        if (m_iTicks < delay) {
            m_iTicks += dt;
            return kBTRunning;
        }
        
        return BTDecoratorTask::update(dt);
    }
    
    eTaskInitResult BTDelayNode::Task::initialize(BTTaskContext* ctx)
    {
        m_iTicks = 0.0f;
        return BTDecoratorTask::initialize(ctx);
    }

    /// repeat task
    eBTStatus BTRepeatNode::Task::update(float dt)
    {
        if (m_iRepeatTimes >= 0 && m_iCounter >= m_iRepeatTimes) {
            return kBTSuccess;
        }
        
        int cnt = 2;
        while (true) {
            eBTStatus status = m_behavior.tick(dt); // BTDecoratorTask::update(dt);
            if (status != kBTRunning) {
                BTRepeatNode::eBreakAction action = static_cast<BTRepeatNode*>(m_pNode)->getActionAfterARound();
                if ((action == kBreakWhenFail && status == kBTFailure) ||
                    (action == kBreakWhenSuccess && status == kBTSuccess)) {
                    return status;
                }
                m_iCounter++;
                if (m_iRepeatTimes >= 0 && m_iCounter >= m_iRepeatTimes) {
                    return status;
                }
                BTDecoratorTask::terminate(status);
                BTDecoratorTask::initialize(getContext());
                if ((--cnt) > 0) {
                    continue;
                }
            }
            break;
        }
        
        return kBTRunning;
    }
    
    eTaskInitResult BTRepeatNode::Task::initialize(BTTaskContext* ctx)
    {
        m_iCounter = 0;
        m_iRepeatTimes = static_cast<BTRepeatNode*>(m_pNode)->getRepeatTimes();
        return BTDecoratorTask::initialize(ctx);
    }

    //
    eBTStatus BTForceNode::Task::update(float dt)
    {
        eBTStatus status = BTDecoratorTask::update(dt);
        if (status != kBTRunning) {
            eForceResult r = static_cast<BTForceNode*>(m_pNode)->getForceResult();
            if (r == kAlwaysSuccess) {
                status = kBTSuccess;
            } else if (r == kAlwaysFail) {
                status = kBTFailure;
            } else if (r == kReverseResult) {
                status = (status == kBTSuccess? kBTFailure: kBTSuccess);
            }
        }
        return status;
    }
} /* end of namespace behavior */