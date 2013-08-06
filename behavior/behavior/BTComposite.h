//
//  BTComposite.h
//  Behavior
//
//  Created by hailong on 12-8-18.
//
//

#ifndef Behavior_BTComposite_h
#define Behavior_BTComposite_h
#include "BTNode.h"
#include <list>
#include <stdarg.h>

namespace behavior {
    typedef std::list<BTNode*> BTNodes;
    typedef BTNodes::iterator BTNodeIterator;
    typedef BTNodes::const_iterator BTNodeConstIterator;
    
    class BTComposite: public BTNode
    {
    public:
        ~BTComposite();
        
        void appendChildNode(BTNode*);
        inline const BTNodes& getChildren() { return m_children; }
        virtual void traverse(BTNodeTraveller traveller, void* param);
    protected:
        BTNodes m_children;
    };
    
    template<class TASK>
    class BTCompositeImpl: public BTComposite
    {
    public:
        ~BTCompositeImpl() {}
        
        static BTCompositeImpl* create(BTNode* node1, ... );
        
        virtual BTTask* createTask() { return new TASK(this); }
        virtual void destroyTask(BTTask* task) { delete task; }
    };
    
    template<class TASK>
    BTCompositeImpl<TASK>* BTCompositeImpl<TASK>::create(BTNode* node1, ... )
    {
        BTCompositeImpl<TASK>* btNode = new BTCompositeImpl<TASK>();
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
     *  Sequence run one child to finish after the other. If one fails the whole sequence fails, too.
     *   Without a reset or without finishing the last child node a sequence stores the last running child to
     *   immediately return to it on the next update.
     */
    class Sequence: public BTTask
    {
    public:
        Sequence(BTComposite* node) : BTTask(node) {}
        virtual ~Sequence() {}
        
        inline BTComposite& getNode()
        {
            return *static_cast<BTComposite*>(m_pNode);
        }
        
        virtual eBTStatus update(float dt);
        virtual eTaskInitResult initialize(BTTaskContext* ctx);
        virtual void terminate(eBTStatus);
        virtual void cancel();
    protected:
        BTNodeConstIterator m_itCurrent;
        Behavior m_currentBehavior;
    };
    
    /**
     *  Selector run one child to finish after the other. If one successes the whole sequence successes, too. 
     *   Without a reset or without finishing the last child node a sequence stores the last running child to 
     *   immediately return to it on the next update.
     */
    class Selector: public BTTask
    {
    public:
        Selector(BTComposite* node) : BTTask(node) {}
        virtual ~Selector() {}
        
        inline BTComposite& getNode()
        {
            return *static_cast<BTComposite*>(m_pNode);
        }
        
        virtual eBTStatus update(float dt);
        virtual eTaskInitResult initialize(BTTaskContext* ctx);
        virtual void terminate(eBTStatus);
        virtual void cancel();
    protected:
        BTNodeConstIterator m_itCurrent;
        Behavior m_currentBehavior;
    };
    
    typedef std::list<Behavior*> BehaviorList;
    typedef BehaviorList::iterator BehaviorListIterator;
    typedef BehaviorList::const_iterator BehaviorListConstIterator;
    
    /**
     *  Parallel nodes run all children concurrently.
     */
    class Parallel: public BTTask
    {
    public:
        Parallel(BTComposite* node) : BTTask(node) {}
        virtual ~Parallel() {}
        
        inline BTComposite& getNode()
        {
            return *static_cast<BTComposite*>(m_pNode);
        }
        
        //virtual eBTStatus update(float dt);
        virtual eTaskInitResult initialize(BTTaskContext* ctx);
        virtual void terminate(eBTStatus);
        virtual void cancel();
    protected:
        BehaviorList m_currentBehaviors;
    };
    
    /**
     *  ParallelSequence run all children concurrently. If one or multiple success the whole sequence successes, too.
     */
    class ParallelSequence: public Parallel
    {
    public:
        ParallelSequence(BTComposite* node) : Parallel(node) {}
        virtual ~ParallelSequence() {}
        
        virtual eBTStatus update(float dt);
        //virtual int initialize(BTTaskContext* ctx);
        //virtual void terminate(eBTStatus);
    protected:
        //BehaviorList m_currentBehaviors;
    };
    
    /**
     *  ParallelSelector run all children concurrently. If one or multiple fail the whole sequence fails, too.
     */
    class ParallelSelector: public Parallel
    {
    public:
        ParallelSelector(BTComposite* node) : Parallel(node) {}
        virtual ~ParallelSelector() {}
        
        virtual eBTStatus update(float dt);
        //virtual int initialize(BTTaskContext* ctx);
        //virtual void terminate(eBTStatus);
    protected:
        //BehaviorList m_currentBehaviors;
    };
    
    typedef BTCompositeImpl<Sequence>   BTSequenceNode;
    typedef BTCompositeImpl<Selector>   BTSelectorNode;
    typedef BTCompositeImpl<ParallelSequence>   BTParallelSequenceNode;
    typedef BTCompositeImpl<ParallelSelector>   BTParallelSelectorNode;
    
} /* end of namespace behavior */

#endif
