//
//  BTNode.h
//  Behavior
//
//  Created by hailong on 12-8-18.
//
//

#ifndef Behavior_BTNode_h
#define Behavior_BTNode_h

#include <stddef.h>
#include "BTObject.h"

namespace behavior {
    class BTNode;
    class BTTask;
    
    enum eBTStatus
    {
        kBTInvalid,
        kBTSuccess,
        kBTFailure,
        kBTRunning
    };
    
    typedef void (*BTNodeTraveller)(BTNode*, void*);
    
    class BTNode: public BTObject
    {
    public:
        BTNode() {}
        virtual ~BTNode() {}
        
        virtual BTTask* createTask() = 0;
        virtual void destroyTask(BTTask* task) = 0;
        virtual void traverse(BTNodeTraveller traveller, void* param)
        {
            if (traveller != NULL) { (*traveller)(this, param); }
        }
    };
    
    class BTTaskContext: public BTObject
    {
    public:
        BTTaskContext() { }
        virtual ~BTTaskContext() { }
    };
    
    typedef AutoPtr<BTTaskContext> BTTaskCtxPointer;
    
    enum eTaskInitResult {
        kReadyToUpdate  = 0, // initialize successfully, only in this case will update() be called
        kTaskFailed     = -1, // initialize failed, this node should return kBTFailure immediately
        kTaskIsDone     = 1, // task is done, this node should return kBTSuccess immediately
    };
    
    class BTTask
    {
    public:
        BTTask(BTNode* node) : m_pNode(node) {}
        virtual ~BTTask() {}
        
        virtual eBTStatus update(float dt) = 0;
        virtual eTaskInitResult initialize(BTTaskContext* ctx) { m_pContext = ctx; return kReadyToUpdate; }
        virtual void terminate(eBTStatus) { m_pContext = NULL; }
        virtual void cancel() { m_pContext = NULL; }
        BTTaskContext* getContext() { return m_pContext; }
    protected:
        BTNode* m_pNode;
        BTTaskCtxPointer m_pContext;
    };
    
#if defined(BT_USE_BEHAVIOR_POOL)
    class __Behavior_sealer {
        friend class Behavior;
        __Behavior_sealer(){}
    };
    class Behavior: __Behavior_sealer
    {
    public:
        static void* operator new (size_t size);
        static void operator delete (void *p);
        static void* operator new[] (size_t size);
        static void operator delete[] (void* p);
    private:
#else
    class Behavior
    {
#endif
    public:
        Behavior()
        : m_pNode(NULL), m_pTask(NULL), m_pContext(NULL), m_eStatus(kBTInvalid), m_bTerminated(true)
        {
        }
        
        Behavior(BTNode& node, BTTaskContext* ctx)
        : m_pNode(&node), m_pTask(NULL), m_pContext(NULL), m_eStatus(kBTInvalid), m_bTerminated(true)
        {
            setup(node, ctx);
        }
        
        ~Behavior()
        {
            m_eStatus = kBTInvalid;
            teardown();
        }
        
        void setup(BTNode& node, BTTaskContext* ctx);
        void teardown();
        eBTStatus tick(float dt);
        void cancel();
        bool isRunning() { return !m_bTerminated && m_eStatus != kBTInvalid; }
        BTNode* currentNode() { return m_pNode; }
        BTTaskContext* getContext() { return m_pContext; }
    private:
        BTNode* m_pNode;
        BTTask* m_pTask;
        eBTStatus m_eStatus;
        bool m_bTerminated;
        BTTaskCtxPointer m_pContext;
    };
} /* end of namespace behavior */

#endif
