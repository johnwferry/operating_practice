#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <fstream>
#include <iostream>
#include <assert.h>

#include <queue>
#include <map>
#include <utility>
#include <iterator>

#include "thread.h"
#include "interrupt.h"

using namespace std;



bool ranBefore = false;
bool initCalled = false;

ucontext_t* currentContext_ptr = NULL;
queue < ucontext_t* > *readyQueue = NULL;
map < unsigned int, queue < ucontext_t* > *> *waitMap = NULL;
map < unsigned int, ucontext_t* > *lockMap = NULL;
map < unsigned long, queue < ucontext_t* > *> *condMap = NULL;

queue < ucontext_t* > *zombieQueue = NULL;


void cleanup()
{
  assert_interrupts_disabled();
  cout << "Thread library exiting.\n";
  interrupt_enable();
  exit(0);

}

//conversion for long
unsigned long hashConvert(unsigned int lock, unsigned int cond)
{
    assert_interrupts_disabled();
    unsigned long tempLock = (unsigned long) lock;
    unsigned long tempCond = (unsigned long) lock;
    return ((tempLock << 32)+ tempCond);
}
//is lock in map?
bool inMap(unsigned int lock)
{
  assert_interrupts_disabled();
  if((*lockMap).empty())
  {
    return false;
  }

  if((*lockMap).find(lock)==(*lockMap).end())
  {
    return false;
  }
  return true;
}
//'' version for waitmap
bool inWaitMap(unsigned int lock)
{
  assert_interrupts_disabled();
  if((*waitMap).empty())
  {
    return false;
  }

  if((*waitMap).find(lock)==(*waitMap).end())
  {
    return false;
  }
  return true;
}
//'' version for condMap
bool inCondMap(unsigned int lock, unsigned int cond)
{
  unsigned long key = hashConvert(lock,cond);
  assert_interrupts_disabled();
  if((*condMap).empty())
  {
    return false;
  }

  if((*condMap).find(key) == (*condMap).end())
  {
    return false;
  }
  return true;
}
//tries to popQueue
//returns null if it can't
ucontext_t* popQueue(queue <ucontext_t*> *q)
{
  assert_interrupts_disabled();
  if((*q).empty())
  {
    return NULL;
  }
  ucontext_t* ret = (*q).front();
    //cout << "readyQueue" << readyQueue << endl;
  (*q).pop();
  //cout << "Front3: " << readyQueue.front() << endl;
  return ret;
}

//tries to pop readyQueue
//if null, cleansup instead.
ucontext_t* popReady()
{
  assert_interrupts_disabled();
  ucontext_t* ret = popQueue(readyQueue);
  if(ret == NULL)
  {
    cleanup();
  }
  return ret;
}

//claims lock
//is VERY assertive and will kill any context currently there.
int claimLock(unsigned int lock, ucontext_t* context)
{
  assert_interrupts_disabled();
  if(!inMap(lock))
  {
    (*lockMap).insert(pair <unsigned int, ucontext_t*> (lock, context));
    return 0;
  }

  ((*lockMap).find(lock))->second = context;

  assert_interrupts_disabled();
  return 0;
}


//pushes the context into the queue for lock
//no bad returns??
int push_wait_lock(unsigned int lock, ucontext_t* context)
{
  assert_interrupts_disabled();
  queue <ucontext_t*> *temp;


  if(!inWaitMap(lock))
  {
    temp = new queue <ucontext_t*>;
    (*waitMap).insert(pair < unsigned int, queue < ucontext_t* > *> (lock, temp));
  }

  temp = (*waitMap).find(lock)->second;
  (*temp).push(context);
  return 0;
}


//pops the queue housed in wait at lock
//returns null if failed to pop
ucontext_t* pop_wait_lock(unsigned int lock)
{
  assert_interrupts_disabled();
  queue <ucontext_t*> *temp;
  ucontext_t* ret;

  if(!inWaitMap(lock))
  {
    return NULL;
  }

  temp = (*waitMap).find(lock)->second;

  ret = popQueue(temp);
  if((*temp).front() == NULL)
  {
    (*waitMap).erase(lock);
    return ret;
  }

  return ret;


}

//pushes the context into the queue for lock on channel cond
//no bad returns??
int push_cond_lock(unsigned int lock, unsigned int cond, ucontext_t* context)
{
  assert_interrupts_disabled();
  queue <ucontext_t*> *temp;
  unsigned long key = hashConvert(lock,cond);


  if(!inCondMap(lock, cond))
  {
    temp = new queue <ucontext_t*>;
    (*condMap).insert(pair < unsigned long, queue < ucontext_t* > *> (key,temp));
  }

  temp = (*condMap).find(key)->second;
  (*temp).push(context);
  return 0;

}

//pops the queue housed in lock on channel cond
//returns null if failed to pop
ucontext_t* pop_cond_lock(unsigned int lock, unsigned int cond)
{
  assert_interrupts_disabled();
  queue <ucontext_t*> *temp;
  ucontext_t* ret;
  unsigned long key = hashConvert(lock, cond);

  if(!inCondMap(lock, cond))
  {
    return NULL;
  }

  temp = (*condMap).find(key)->second;

  ret = popQueue(temp);
  //cout << "Front: " << ret << endl;

  if((*temp).front() == NULL)
  {
    (*condMap).erase(key);
    return ret;
  }
  return ret;


}

//swaps context to into and stores at store
//Highly error resistant
int swapret(ucontext_t* store, ucontext_t* into)
{
  assert_interrupts_disabled();
  assert(store != NULL);
  assert(store->uc_stack.ss_sp != NULL);
  assert(into != NULL);
  assert(into->uc_stack.ss_sp != NULL);
  currentContext_ptr = into;

  //cout << "Swapping context from " << store << " to context " << into << endl;
  swapcontext(store, into);
  assert_interrupts_disabled();
    //cout << "Returning context from " << into << " to context " << store << endl;
  //cout << "Front " << (*zombieQueue).front() << " me " << currentContext_ptr<< endl;

  currentContext_ptr = store;
  return 0;
}

int thread_start(thread_startfunc_t func, void* args)
{
  assert_interrupts_disabled();
  assert(lockMap!=NULL);
  assert(condMap!=NULL);
  assert(waitMap!=NULL);

  interrupt_enable();
  func(args);
  interrupt_disable();

  if((*readyQueue).empty())
  {
    cleanup();
  }
  ucontext_t* temp;
  //cout << "Front " << (*zombieQueue).front() << " me " << currentContext_ptr<< endl;
  while(!(*zombieQueue).empty())
  {
    temp = popQueue(zombieQueue);
    assert(temp!=currentContext_ptr);
    assert(temp!=NULL);
    assert(temp->uc_stack.ss_sp !=NULL);
    delete[]  ((char*)temp->uc_stack.ss_sp);
    delete temp;
  }

  if(initCalled)
  {
    (*zombieQueue).push(currentContext_ptr);
  }else
  {
    initCalled = true;
  }

  setcontext(popReady());
}

int thread_libinit(thread_startfunc_t func, void* args)
{
  interrupt_disable();

  if(ranBefore)
  {
    interrupt_enable();
    return -1;
  }
  ranBefore = true;

  ucontext_t* context = new ucontext_t;
  if(!context)
  {
    interrupt_enable();
    return -1;
  }
  currentContext_ptr = context;
  readyQueue = new queue < ucontext_t* >;
  waitMap = new map < unsigned int, queue < ucontext_t* > *>;
  lockMap = new map < unsigned int, ucontext_t* >;
  condMap = new map < unsigned long, queue < ucontext_t* > *>;
  zombieQueue = new queue < ucontext_t* >;
  getcontext(context);

  char* stack = new char[STACK_SIZE];
  if(!stack)
  {
    interrupt_enable();
    return -1;
  }

  context->uc_stack.ss_sp = stack;
  context->uc_stack.ss_size = STACK_SIZE;
  context->uc_stack.ss_flags = 0;
  context->uc_link = NULL;

  makecontext(context, (void (*) ()) thread_start, 2, func, args);


  setcontext(context);

  interrupt_enable();
  return -1;

}

int thread_create(thread_startfunc_t func, void* args)
{

  interrupt_disable();

  if(!ranBefore)
  {
    interrupt_enable();
    return -1;
  }

  ucontext_t* context = new ucontext_t;
  if(!context)
  {
    interrupt_enable();
    return -1;
  }

  getcontext(context);
  if(!context)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }

  char* stack = new char[STACK_SIZE];

  if(!stack)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }
  context->uc_stack.ss_sp = stack;
  context->uc_stack.ss_size = STACK_SIZE;
  context->uc_stack.ss_flags = 0;
  context->uc_link = NULL;

  makecontext(context, (void (*) ()) thread_start, 2, func, args);

  (*readyQueue).push(context);

  interrupt_enable();
  return 0;
}

int thread_lock(unsigned int lock)
{
  interrupt_disable();

  if(!ranBefore)
  {
    interrupt_enable();
    return -1;
  }

  ucontext_t* myContext = currentContext_ptr;
  assert(myContext !=NULL);
    //cout << "The front of readyqueue " << readyQueue.front() << endl;


  //the lock isn't in the map
  if(!inMap(lock))
  {
    //cout << "The lock isn't in the map " << myContext << endl;
    claimLock(lock, myContext);
    assert(((*lockMap).find(lock))->second == myContext);
    interrupt_enable();
    return 0;
  }

  ucontext_t* temp = (*lockMap).find(lock)->second;

  //the lock holds my context
  if(temp == myContext)
  {
      //cout << "The lock is mine " << myContext << endl;
    interrupt_enable();
    return -1;
  }


  //lock is in the map and doesn't hold my context

  //push my context onto the lock queue
    //cout << "I'm going on the wait for lock " << myContext << " popping " << readyQueue.front() << endl;
  push_wait_lock(lock, myContext);

  //swapcontext into the top of readyQueue
  ucontext_t* ptr = popReady();
  //cout << "Front: " << readyQueue.front() << endl;
  swapret(myContext, ptr);

  //claim lock on return;
  claimLock(lock, myContext);
    assert(((*lockMap).find(lock))->second == myContext);

  interrupt_enable();
  return 0;

}
int thread_lock_internal(unsigned int lock) // TO EDIT FOR MATCHING LOCK
{
    assert_interrupts_disabled();


    if(!ranBefore)
    {
      return -1;
    }

    ucontext_t* myContext = currentContext_ptr;
    assert(myContext !=NULL);
      //cout << "The front of readyqueue " << readyQueue.front() << endl;


    //the lock isn't in the map
    if(!inMap(lock))
    {
      //cout << "Lock not in map " << myContext << endl;
      claimLock(lock, myContext);
            assert(((*lockMap).find(lock))->second == myContext);
      return 0;
    }

    ucontext_t* temp = (*lockMap).find(lock)->second;

    //the lock holds my context
    if(temp == myContext)
    {
        //cout << "The lock is mine " << myContext << endl;
      return -1;
    }


    //lock is in the map and doesn't hold my context

    //push my context onto the lock queue
      //cout << "I'm going on the wait for lock " << myContext << " popping " << readyQueue.front() << endl;
    push_wait_lock(lock, myContext);

    //swapcontext into the top of readyQueue
    ucontext_t* ptr = popReady();
    //cout << "Front: " << readyQueue.front() << endl;
    swapret(myContext, ptr);

    //claim lock on return;
    claimLock(lock, myContext);
    assert(((*lockMap).find(lock))->second == myContext);
    //cout << "Claimed lock on return " << myContext << endl;
    return 0;


}

int thread_unlock_internal(unsigned int lock) // TO EDIT FOR MATCHING UNLOCK
{
  assert_interrupts_disabled();

  if(!ranBefore)
  {

    return -1;
  }
  ucontext_t* myContext = currentContext_ptr;
  assert(myContext != NULL);


  //lock in map
  if(!inMap(lock))
  {

    return -1;
  }

  ucontext_t* temp = (*lockMap).find(lock)->second;


  //the lock holds my context i.e. is mine
  if(temp == myContext)
  {
    ucontext_t* pop = pop_wait_lock(lock);
    if(pop !=NULL)
    {
      //if there was something waiting, push it onto the ready queue
      (*readyQueue).push(pop);
    }
    //remove the lock from the lockmap, so it is now a free lock
    (*lockMap).erase(lock);


    return 0;
  }

  return -1;
}

int thread_unlock(unsigned int lock)
{
  interrupt_disable();

  if(!ranBefore)
  {
    interrupt_enable();
    return -1;
  }
  ucontext_t* myContext = currentContext_ptr;
  assert(myContext != NULL);


  //lock in map
  if(!inMap(lock))
  {
    interrupt_enable();
    return -1;
  }

  ucontext_t* temp = (*lockMap).find(lock)->second;


  //the lock holds my context i.e. is mine
  if(temp == myContext)
  {
    ucontext_t* pop = pop_wait_lock(lock);
    if(pop !=NULL)
    {
      //if there was something waiting, push it onto the ready queue
      (*readyQueue).push(pop);
    }
    //remove the lock from the (*lockMap), so it is now a free lock
    (*lockMap).erase(lock);

    interrupt_enable();
    return 0;
  }

  interrupt_enable();
  return -1;

}

int thread_wait(unsigned int lock, unsigned int cond)
{
  interrupt_disable();

  if(!ranBefore)
  {
    interrupt_enable();
    return -1;
  }

  if(thread_unlock_internal(lock) == -1)
  {
    //you can't unlock, can't wait;
    interrupt_enable();
    return -1;
  }
  assert_interrupts_disabled();
  ucontext_t* myContext = currentContext_ptr;
  assert(myContext !=NULL);
  push_cond_lock(lock, cond, myContext);
  assert_interrupts_disabled();


  swapret(myContext, popReady());
  assert_interrupts_disabled();

  currentContext_ptr = myContext;

  if(thread_lock_internal(lock) == -1)
  {
    //must get lock after leaving
    interrupt_enable();
    return -1;
  }

  assert_interrupts_disabled();

  interrupt_enable();
  return 0;
}

int thread_broadcast(unsigned int lock, unsigned int cond)
{
  interrupt_disable();
  if(!ranBefore)
  {
    interrupt_enable();
    return -1;
  }

  if(!inCondMap(lock, cond))
  {
    interrupt_enable();
    return 0;
  }

  ucontext_t* pop = pop_cond_lock(lock, cond);
  while(pop!=NULL)
  {
    //cout << "thread" << endl;
    (*readyQueue).push(pop);
    pop = pop_cond_lock(lock, cond);
  }
  interrupt_enable();
  return 0;
}

int thread_signal(unsigned int lock, unsigned int cond)
{
  interrupt_disable();
  if(!ranBefore)
  {
    interrupt_enable();
    return -1;
  }

  if(!inCondMap(lock, cond))
  {
    interrupt_enable();
    return 0;
  }

  ucontext_t* pop = pop_cond_lock(lock, cond);
  if(pop == NULL)
  {
    interrupt_enable();
    return 0;
  }

  (*readyQueue).push(pop);

  interrupt_enable();
  return 0;
}

int thread_yield(void)
{

  interrupt_disable();
  if(!ranBefore)
  {
    interrupt_enable();
    return -1;
  }

  if((*readyQueue).front()==NULL)
  {
    interrupt_enable();
    return 0;
  }
  //cout << "Context " << currentContext_ptr << " is Yielding" << endl;
  ucontext_t* myContext = currentContext_ptr;

  (*readyQueue).push(myContext);
  //cout << "Context " << myContext << " is pushed to readyqueue" << endl;
  //cout << "Front: " << readyQueue.front() << endl;
  swapret(myContext, popReady());


  interrupt_enable();
  return 0;

}
