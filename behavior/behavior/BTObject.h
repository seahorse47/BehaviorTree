//
//  BTObject.h
//  Behavior
//
//  Created by hailong on 13-2-24.
//
//

#ifndef __Behavior__BTObject__
#define __Behavior__BTObject__

#include <stddef.h>
#include <list>
#include <assert.h>

namespace behavior {
    template <class T>
    class AutoReleasePool
    {
    public:
        typedef T ElemObject;
        typedef std::list<ElemObject*> AutoreleaseObjects;
        
        AutoReleasePool() {
        }
        
        ~AutoReleasePool() {
            releaseAllObjects();
        }
        
        void addObject(ElemObject* obj) {
            if (obj) {
                m_arrManagedObjects.push_back(obj);
                obj->retain();
            }
        }
        
        void releaseAllObjects() {
            typename AutoreleaseObjects::iterator it = m_arrManagedObjects.begin();
            for (; it != m_arrManagedObjects.end(); ++it) {
                (*it)->release();
            }
            if (m_arrManagedObjects.size() > 0) {
                m_arrManagedObjects.clear();
            }
        }
    private:
        AutoreleaseObjects m_arrManagedObjects;
    };
    
    template<class Cls>
    class AutoPtr
    {
    public:
        typedef Cls  RawType;
        typedef Cls* RawPointer;
        
        AutoPtr(RawPointer p = NULL) : m_ptr(NULL) { reset(p); }
        AutoPtr(const AutoPtr& ptr) : m_ptr(NULL) { reset(ptr.rawPtr()); }
        ~AutoPtr() { reset(NULL); }
        
        inline RawPointer rawPtr() const { return m_ptr; }
        inline RawType& operator*() const { return *m_ptr; }
        inline RawPointer operator->() const { return m_ptr; }
        inline operator RawPointer() { return m_ptr; }
        
        inline AutoPtr& operator=(RawPointer p) { reset(p); return *this; }
        inline AutoPtr& operator=(AutoPtr& ptr) { reset(ptr.rawPtr()); return *this; }
        inline bool operator==(RawPointer p) const { return m_ptr == p; }
        inline bool operator==(const AutoPtr& ptr) const { return m_ptr == ptr.rawPtr(); }
        inline bool operator!=(RawPointer p) const { return m_ptr != p; }
        inline bool operator!=(const AutoPtr& ptr) const { return m_ptr != ptr.rawPtr(); }
        
    private:
        void reset(RawPointer p) {
            if (p != m_ptr) {
                if (p) { p->retain(); }
                if (m_ptr) { m_ptr->release(); }
                m_ptr = p;
            }
        }
        RawPointer m_ptr;
    };
    
    class BTObject
    {
    public:
        BTObject() : m_iRefCnt(1) {}
        virtual ~BTObject() {}
        
        void retain() {
            ++m_iRefCnt;
        }
        
        void release() {
            if (0 == (--m_iRefCnt)) {
                delete this;
            }
        }
        
        BTObject* autorelease() {
            autoreleasePool()->addObject(this);
            this->release();
            return this;
        }
        
        // retrieve the autorelease pool
        // call the pool's releaseAllObjects() method main loop if you plan to use autorelease
        static AutoReleasePool<BTObject>* autoreleasePool() {
            static AutoReleasePool<BTObject> s_pool;
            return &s_pool;
        }
    private:
        int m_iRefCnt;
    };
    
#   define IMPL_AUTORELEASE(T) inline T* autorelease() { return (T*)behavior::BTObject::autorelease(); }
    
} /* end of namespace behavior */

#endif /* defined(__Behavior__BTObject__) */
