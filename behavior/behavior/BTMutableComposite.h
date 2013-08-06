//
//  BTMutableComposite.h
//  Behavior
//
//  Created by hailong on 13-2-26.
//
//

#ifndef __Behavior__BTMutableComposite__
#define __Behavior__BTMutableComposite__

#include "BTComposite.h"
#include <list>
#include <stdarg.h>

namespace behavior {
//    typedef std::list<BTNode*> BTNodes;
//    typedef BTNodes::iterator BTNodeIterator;
//    typedef BTNodes::const_iterator BTNodeConstIterator;
    
    class BTMutableComposite: public BTComposite
    {
    public:
        BTMutableComposite();
        ~BTMutableComposite();
        
        bool hasNext() { return m_children.size() > 0; }
        BTNode* next();
    private:
        BTNode* m_pCurrent;
        /* Hide the getChildren() method */
        inline const BTNodes& getChildren() { return m_children; }
        void discardCurrent();
    };
    
    template<class TASK>
    class BTMutableCompositeImpl: public BTMutableComposite
    {
    public:
        ~BTMutableCompositeImpl() {}
        
        static BTMutableCompositeImpl* create(BTNode* node1, ... );
        
        virtual BTTask* createTask() { return new TASK(*this); }
        virtual void destroyTask(BTTask* task) { delete task; }
    };
    
    template<class TASK>
    BTMutableCompositeImpl<TASK>* BTMutableCompositeImpl<TASK>::create(BTNode* node1, ... )
    {
        BTMutableCompositeImpl<TASK>* btNode = new BTMutableCompositeImpl<TASK>();
        va_list va;
        va_start(va, node1);
        BTNode* child = node1;
        while (child) {
            btNode->appendChildNode(child);
            child = va_arg(va, BTNode*);
        }
        va_end(va);
        
        btNode->autorelease();
        
        return btNode;
    }
    
    /**
     *  Similar to Sequence.
     */
    class MutableSequence: public BTTask
    {
    public:
        MutableSequence(BTMutableComposite* node) : BTTask(node) {}
        virtual ~MutableSequence() {}
        
        inline BTMutableComposite& getNode()
        {
            return *static_cast<BTMutableComposite*>(m_pNode);
        }
        
        virtual eBTStatus update(float dt);
        virtual eTaskInitResult initialize(BTTaskContext* ctx);
        virtual void terminate(eBTStatus);
        virtual void cancel();
    protected:
        Behavior m_currentBehavior;
    };
    
    /**
     *  Similiar to Selector.
     */
    class MutableSelector: public BTTask
    {
    public:
        MutableSelector(BTMutableComposite* node) : BTTask(node) {}
        virtual ~MutableSelector() {}
        
        inline BTMutableComposite& getNode()
        {
            return *static_cast<BTMutableComposite*>(m_pNode);
        }
        
        virtual eBTStatus update(float dt);
        virtual eTaskInitResult initialize(BTTaskContext* ctx);
        virtual void terminate(eBTStatus);
        virtual void cancel();
    protected:
        Behavior m_currentBehavior;
    };
    
//    typedef std::list<Behavior*> BehaviorList;
//    typedef BehaviorList::iterator BehaviorListIterator;
//    typedef BehaviorList::const_iterator BehaviorListConstIterator;
    
    /**
     *  Parallel nodes run all children concurrently.
     */
    class MutableParallel: public BTTask
    {
    public:
        MutableParallel(BTMutableComposite* node) : BTTask(node) {}
        virtual ~MutableParallel() {}
        
        inline BTMutableComposite& getNode()
        {
            return *static_cast<BTMutableComposite*>(m_pNode);
        }
        
        virtual eBTStatus update(float dt);
        virtual eTaskInitResult initialize(BTTaskContext* ctx);
        virtual void terminate(eBTStatus);
        virtual void cancel();
    protected:
        BehaviorList m_currentBehaviors;
        
        virtual eBTStatus doUpdate(float dt) = 0;
        void setupNewTasks();
        void cleanupStoppedTasks();
    };
    
    /**
     *  Similiar to ParallelSequence.
     */
    class MutableParallelSequence: public MutableParallel
    {
    public:
        MutableParallelSequence(BTMutableComposite* node) : MutableParallel(node) {}
        virtual ~MutableParallelSequence() {}
        
        virtual eBTStatus doUpdate(float dt);
    };
    
    /**
     *  ParallelSelector run all children concurrently. If one or multiple fail the whole sequence fails, too.
     */
    class MutableParallelSelector: public MutableParallel
    {
    public:
        MutableParallelSelector(BTMutableComposite* node) : MutableParallel(node) {}
        virtual ~MutableParallelSelector() {}
        
        virtual eBTStatus doUpdate(float dt);
    };
    
    typedef BTMutableCompositeImpl<MutableSequence>   BTMutableSequenceNode;
    typedef BTMutableCompositeImpl<MutableSelector>   BTMutableSelectorNode;
    typedef BTMutableCompositeImpl<MutableParallelSequence>   BTMutableParallelSequenceNode;
    typedef BTMutableCompositeImpl<MutableParallelSelector>   BTMutableParallelSelectorNode;
    
} /* end of namespace behavior */

#endif /* defined(__Behavior__BTMutableComposite__) */
