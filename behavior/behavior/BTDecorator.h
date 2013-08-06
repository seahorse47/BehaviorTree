//
//  BTDecorator.h
//  Behavior
//
//  Created by hailong on 12-8-19.
//
//

#ifndef Behavior_BTDecorator_h
#define Behavior_BTDecorator_h
#include "BTNode.h"

namespace behavior {
    class BTDecorator: public BTNode
    {
    public:
        BTDecorator(BTNode& decorated) : m_pDecoratedNode(&decorated) { m_pDecoratedNode->retain(); }
        ~BTDecorator() { m_pDecoratedNode->release(); }
        BTNode* getDecoratedNode() { return m_pDecoratedNode; }
        virtual void traverse(BTNodeTraveller traveller, void* param)
        {
            BTNode::traverse(traveller, param);
            m_pDecoratedNode->traverse(traveller, param);
        }
    protected:
        BTNode* m_pDecoratedNode;
    };
    
    class BTDecoratorTask: public BTTask
    {
    public:
        BTDecoratorTask(BTDecorator* node) : BTTask(node) {}
        virtual ~BTDecoratorTask() {}
        
        BTDecorator& getNode() { return *static_cast<BTDecorator*>(m_pNode); }
        
        virtual eBTStatus update(float dt) { return m_behavior.tick(dt); }
        virtual eTaskInitResult initialize(BTTaskContext* ctx)
        {
            BTTask::initialize(ctx);
            m_behavior.setup(*(getNode().getDecoratedNode()), ctx);
            return kReadyToUpdate;
        }
        virtual void terminate(eBTStatus currentStatus)
        {
            m_behavior.teardown();
            BTTask::terminate(currentStatus);
        }
        virtual void cancel() { m_behavior.cancel(); }
    protected:
        Behavior m_behavior;
    };
    
//    template<class DECORATOR_TASK>
//    class BTDecoratorImpl: public BTDecorator
//    {
//    public:
//        BTDecoratorImpl(BTNode& node): BTDecorator(node) {}
//        virtual BTTask* createTask()
//        {
//            return new DECORATOR_TASK(*this, m_pDecoratedNode->createTask());
//        }
//        virtual void destroyTask(BTTask* task)
//        {
//            if (task) {
//                BTDecoratorTask* decorator = static_cast<BTDecoratorTask*>(task);
//                m_pDecoratedNode->destroyTask(decorator->getRealTask());
//                delete decorator;
//            }
//        }
//    };
    
    /**
     * Run the given node after a delay.
     */
    class BTDelayNode: public BTDecorator
    {
    public:
        static BTDelayNode* create(BTNode& delayedNode, float delay) {
            BTDelayNode* obj = new BTDelayNode(delayedNode, delay);
            return (BTDelayNode*)obj->autorelease();
        }
    public:
        BTDelayNode(BTNode& delayedNode, float delay) : BTDecorator(delayedNode), m_fDelay(delay) {}
        float getDelay() { return m_fDelay; }
        
        class Task: public BTDecoratorTask
        {
        public:
            Task(BTDelayNode* node): BTDecoratorTask(node) {}
            virtual eBTStatus update(float dt);
            virtual eTaskInitResult initialize(BTTaskContext* ctx);
        protected:
            float m_iTicks;
        };
        
        virtual BTTask* createTask()
        {
            return new Task(this);
        }
        virtual void destroyTask(BTTask* task)
        {
            delete task;
        }
    private:
        float m_fDelay;
    };
    
    /**
     * Repeat the given node for specific times (or infinitely) until it fails. 
     */
    class BTRepeatNode: public BTDecorator
    {
    public:
        enum eBreakAction {
            kNeverBreak,
            kBreakWhenFail,
            kBreakWhenSuccess,
        };
        static BTRepeatNode* create(BTNode& repeatedNode, int times, eBreakAction actionAfterARound = kNeverBreak) {
            BTRepeatNode* obj = new BTRepeatNode(repeatedNode, times, actionAfterARound);
            return (BTRepeatNode*)obj->autorelease();
        }
    public:
        BTRepeatNode(BTNode& repeatedNode, int times, eBreakAction actionAfterARound) : BTDecorator(repeatedNode), m_iRepeatTimes(times), m_eActionAfterARound(actionAfterARound) {}
        int getRepeatTimes() { return m_iRepeatTimes; }
        eBreakAction getActionAfterARound() { return m_eActionAfterARound; }
        
        class Task: public BTDecoratorTask
        {
        public:
            Task(BTRepeatNode* node): BTDecoratorTask(node) {}
            virtual eBTStatus update(float dt);
            virtual eTaskInitResult initialize(BTTaskContext* ctx);
        protected:
            int m_iCounter;
            int m_iRepeatTimes;
        };
        
        virtual BTTask* createTask()
        {
            return new Task(this);
        }
        virtual void destroyTask(BTTask* task)
        {
            delete task;
        }
    private:
        int m_iRepeatTimes;
        eBreakAction m_eActionAfterARound;
    };
    
    /**
     * force node, it can change the result of some node
     */
    class BTForceNode: public BTDecorator
    {
    public:
        enum eForceResult {
            kAlwaysSuccess,
            kAlwaysFail,
            kReverseResult,
        };
        static BTForceNode* create(BTNode& node, eForceResult result) {
            BTForceNode* obj = new BTForceNode(node, result);
            return (BTForceNode*)obj->autorelease();
        }
    public:
        BTForceNode(BTNode& node, eForceResult result)
        : BTDecorator(node), m_eResult(result) {}
        
        eForceResult getForceResult() { return m_eResult; }
        
        class Task: public BTDecoratorTask
        {
        public:
            Task(BTForceNode* node): BTDecoratorTask(node) {}
            virtual eBTStatus update(float dt);
        };
        
        virtual BTTask* createTask()
        {
            return new Task(this);
        }
        virtual void destroyTask(BTTask* task)
        {
            delete task;
        }
    private:
        eForceResult m_eResult;
    };
} /* end of namespace behavior */

#endif
