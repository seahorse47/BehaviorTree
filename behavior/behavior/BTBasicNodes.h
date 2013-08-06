//
//  BTBasicNodes.h
//  Behavior
//
//  Created by hailong on 13-4-19.
//
//

#ifndef __Behavior__BTBasicNodes__
#define __Behavior__BTBasicNodes__

#include "BTNode.h"

namespace behavior {
    class BTWaitNode: public BTNode {
    public:
        BTWaitNode(float time) {
            m_fTime = time;
        }
        
        class Task: public BTTask
        {
        public:
            Task(BTWaitNode* node)  : BTTask(node) { }
            ~Task() { }
            virtual eBTStatus update(float dt);
            virtual eTaskInitResult initialize(BTTaskContext* ctx);
        private:
            float m_fTicks;
            bool m_bFirstTick;
        };
        
        static BTWaitNode* create(float time) {
            BTWaitNode* obj = new BTWaitNode(time);
            return (BTWaitNode*)obj->autorelease();
        }
        
        virtual BTTask* createTask() {
            return new Task(this);
        }
        virtual void destroyTask(BTTask* task) {
            delete task;
        }
        
        float getTime() {
            return m_fTime;
        }
    private:
        float m_fTime;
    };
    
    class BTCondition: public BTObject
    {
    public:
        virtual void beginCheckingCondition() {}
        virtual bool isTrue() const = 0;
    };
    
    class BTWaitUntilNode: public BTNode {
    public:
        BTWaitUntilNode(BTCondition* cond) {
            m_pCond = cond;
        }
        
        class Task: public BTTask
        {
        public:
            Task(BTWaitUntilNode* node) : BTTask(node) { }
            ~Task() { }
            
            virtual eBTStatus update(float dt);
            virtual eTaskInitResult initialize(BTTaskContext* ctx);
        };
        
        static BTWaitUntilNode* create(BTCondition* cond) {
            BTWaitUntilNode* obj = new BTWaitUntilNode(cond);
            return (BTWaitUntilNode*)obj->autorelease();
        }
        
        virtual BTTask* createTask() {
            return new Task(this);
        }
        virtual void destroyTask(BTTask* task) {
            delete task;
        }
        
        BTCondition* getCondition() {
            return m_pCond;
        }
    private:
        AutoPtr<BTCondition> m_pCond;
    };

    class BTLinkNode: public BTNode {
    public:
        BTLinkNode(BTNode& target) {
            m_pTargetNode = &target;
        }
        
        static BTLinkNode* create(BTNode& target) {
            BTLinkNode* obj = new BTLinkNode(target);
            return (BTLinkNode*)obj->autorelease();
        }
        
        virtual BTTask* createTask() {
            return m_pTargetNode->createTask();
        }
        virtual void destroyTask(BTTask* task) {
            m_pTargetNode->destroyTask(task);
        }
        
        BTNode* getTargetNode() {
            return m_pTargetNode;
        }
    private:
        AutoPtr<BTNode> m_pTargetNode;
    };
    
    class BTCloneableTask: public BTObject, public BTTask {
    public:
        BTCloneableTask(): BTTask(NULL) {}
        BTCloneableTask(const BTCloneableTask* other): BTTask(NULL) {}
        virtual BTCloneableTask* clone() const = 0;
    };
    
    class BTSimpleNode: public BTNode {
    public:
        BTSimpleNode(BTCloneableTask* taskProto) {
            assert(taskProto);
            m_taskProto = taskProto;
        }
        virtual ~BTSimpleNode() {}
        
        virtual BTTask* createTask() {
            return m_taskProto->clone();
        }
        virtual void destroyTask(BTTask* task) {
            static_cast<BTCloneableTask*>(task)->release();
        }
    protected:
        AutoPtr<BTCloneableTask> m_taskProto;
    };
} /* end of namespace behavior */

#endif /* defined(__Behavior__BTBasicNodes__) */
