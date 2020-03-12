#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <fstream>
#include <iostream>
#include <assert.h>
#include "thread.h"
#include "interrupt.h"

using namespace std;


struct thread {
    ucontext_t* context;
    int id;
};

struct queue{
  thread* t;
  queue* next;
};

struct condqueue{
  thread* t;
  unsigned int cond;
  condqueue* next;
};

queue* convert(condqueue* q)
{
  assert_interrupts_disabled();
  assert(q!=NULL);
  queue* ret = (queue*) malloc(sizeof(queue));
  assert(ret!= NULL);
  ret->t = q->t;
  ret->next = NULL;
  return ret;
}

struct lockt{
  lockt* next;
  unsigned int lock;
  bool locked;
  int id;
  condqueue* condQueue;
  queue* lockQueue;
};

bool ranBefore = false;
int idCount = 0;
thread* currentThread = NULL;
queue* readyQueue = NULL;
queue* currentQueue = NULL;
queue* zombieQueue = NULL;
lockt* locks = NULL;

void cleanup()
{
  assert_interrupts_disabled();

  cout << "Cleanup, cleanup, everybody do your share" << endl;
  /*queue* temp = zombieQueue;
  condqueue* cond;
  condqueue* back;
  while(zombieQueue->next !=NULL)
  {

    while(temp->next !NULL)
    {

      free(temp->t->context->uc_stack.ss_sp);
      free(temp->t->context);
      free(temp->t);


      free(zombieQueue);
      cout << "free'd queue, moving onto next queue" <<endl;
      zombieQueue = temp;
    }
    cout << "Cleaned zombie queue " << endl;
    lockt* tlock = locks;
    while(tlock != NULL)
    {
      temp = tlock->lockQueue;
      zombieQueue = temp;
      while(temp !=NULL)
      {
        free(temp->t->context->uc_stack.ss_sp);
        free(temp->t->context);
        free(temp->t);

        temp = temp->next;
        free(zombieQueue);
        zombieQueue = temp;
      }
      cout << "Cleaned lockqueue " << endl;
      cond = tlock->condQueue;
      back = cond;
      while(cond!=NULL)
      {
        free(cond->t->context->uc_stack.ss_sp);
        free(cond->t->context);
        free(cond->t);

        cond = cond->next;
        free(back);
        back = cond;
      }
      cout << "Cleaned condqueue " << endl;
      tlock = tlock->next;
      free(locks);
      locks = tlock;
    }
*/

    interrupt_enable();
  cout << "Thread library exiting.\n";
  exit(0);

}

void addReady(queue* q)
{
  assert_interrupts_disabled();
  queue* temp = readyQueue;

  if(readyQueue == NULL)
  {
   readyQueue = q;
   readyQueue->next = NULL;
   return;
  }
  while(temp->next != NULL)
  {
    temp = temp->next;
  }

  temp->next = q;
  q->next = NULL;
}

queue* popReady()
{
  assert_interrupts_disabled();
  if(readyQueue == NULL)
  {
    return NULL;
  }
  queue* temp = readyQueue;
  readyQueue = readyQueue->next;
  temp->next = NULL;
  return temp;
}

int swapret(queue* store, queue* into)
{
  assert_interrupts_disabled();
  assert(store != NULL);
  assert(store->t != NULL);
  assert(store->t->context != NULL);
  assert(store->t->context->uc_stack.ss_sp != NULL);
  assert(into != NULL);
  assert(into->t != NULL);
  assert(into->t->context != NULL);
  assert(into->t->context->uc_stack.ss_sp != NULL);
  currentThread = into->t;
  currentQueue = into;
  into->t = currentThread;

  into->next = NULL;
  store->next = NULL;


  swapcontext(store->t->context, into->t->context);


  currentThread = store->t;
  currentQueue = store;
  store->t = currentThread;


  return 0;
}



bool lockExist(unsigned int lock)
{
  assert_interrupts_disabled();
  if(locks == NULL)
  {
    return false;
  }
  lockt* temp = locks;
  while(temp != NULL)
  {
    if(temp->lock == lock)
    {
      return true;
    }
    temp= temp->next;
  }
  return false;

}
bool ismylock(unsigned int lock)
{
  assert_interrupts_disabled();
  assert(locks!=NULL);
  lockt* temp = locks;
  while(temp != NULL)
  {
    if(temp->id == currentThread->id && temp->lock == lock)
    {
      return true;
    }
    if(temp->lock == lock)
    {
      return false;
    }
    temp = temp->next;
  }
  return false;

}

lockt* findLock(unsigned int lock)
{
  assert_interrupts_disabled();
  assert(lockExist(lock));
  assert(locks!=NULL);
  lockt* temp = locks;

  while(temp->lock != lock)
  {
    assert(temp!=NULL);
    temp = temp->next;
  }
  assert(temp->lock == lock);
  return temp;
}


bool islocked(unsigned int lock)
{
  assert_interrupts_disabled();
  return findLock(lock)->locked;

}


int thread_start(thread_startfunc_t func, void* args)
{
  assert_interrupts_disabled();

    /*ucontext_t* context = (ucontext_t*) malloc(sizeof(ucontext_t));
        //  cout << "thread_start context isn't problem" << endl;
  if(context == NULL)
  {
    return -1;
  }
      //  cout << "thread_start getcontext isn't problem" << endl;

  char* stack = new char [STACK_SIZE];

  if(stack == NULL)
  {
    return -1;
  }


  context->uc_stack.ss_sp = stack;
  context->uc_stack.ss_size = STACK_SIZE;
  context->uc_stack.ss_flags = 0;
  context->uc_link = NULL;

      //  cout << "thread_start uc_stack assignement isn't problem" << endl;

  makecontext(context, (void (*) ()) func, 1, args);
      //  cout << "thread_start makecontext isn't problem" << endl;
      */

  currentThread->id = idCount;
      //  cout << "thread_start thread assignemnt isn't problem" << endl;
  idCount = idCount + 1;
  getcontext(currentThread->context);
  currentQueue->t = currentThread;
  currentQueue->next = NULL;

        //initialize the currentThread and currentQueue

        /*
  //cout << "currentQueue isn't null" << endl;
  queue* temp = readyQueue;
  if(temp!= NULL)
  {
    while(temp->next !=NULL)
    {
      temp = temp->next;
    }
    //cout << "exited while" << endl;
    temp->next = (queue*) malloc(sizeof(queue));
    temp = temp->next;
  }else{
      temp = (queue*) malloc(sizeof(queue));
  }
  //cout << "temp->next has been allocated" << endl;
  if(temp == NULL)
  {
    return -1;
  }
  //cout << "temp next isn't problem" << endl;
  temp->t = (thread*) malloc(sizeof(thread));
  if(temp->t == NULL)
  {
    return -1;
  }
  //cout << "temp thread isn't problem" << endl;
  temp->t->id = -1;
      //cout << "oldthread id isn't problem" << endl;
  //nEED TO MAKE CONTEXT!!!!
  ucontext_t* context = (ucontext_t*) malloc(sizeof(ucontext_t));
  if(context == NULL)
  {
    return -1;
  }
    //cout << "thread_start context isn't problem" << endl;
  temp->t->context = context;

  */
  interrupt_enable();
  func(args);
  interrupt_disable();
    // cout<<"Wrapper back from function" << endl;
  //ZOMBIE cleanup
  if(zombieQueue == NULL)
  {
    zombieQueue = currentQueue;
    currentQueue->next == NULL;
  }else
  {
    queue* tempZ = zombieQueue;
    while(tempZ->next != NULL)
    {
      tempZ = tempZ->next;
    }
    tempZ->next = currentQueue;
    currentQueue->next = NULL;
  }
  //cout << "Finished adding self to zombieQueue" << endl;
  if(readyQueue==NULL)
  {
    cleanup();
  }

  queue* temp = readyQueue;
  readyQueue = readyQueue->next;
  currentThread = temp->t;
  //cout << "readyqueue is not null" << endl;
  assert(temp->t->context !=NULL);
  currentQueue = temp;
  currentQueue->next = NULL;

  setcontext(currentThread->context);

}
int thread_libinit(thread_startfunc_t func, void* args)
{
  interrupt_disable();
  assert_interrupts_disabled();

  if(ranBefore)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }
  ranBefore = true;
  idCount = 0;

  ucontext_t* context = (ucontext_t*) malloc(sizeof(ucontext_t));
  if(context == NULL)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }
  getcontext(context);

  if(context == NULL)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }

  char* stack = new char [STACK_SIZE];

  if(stack == NULL)
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

  thread* me = (thread*) malloc(sizeof(thread));
  if(me == NULL)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }

  me->id = -1;
  me->context = context;
  queue* q = (queue*) malloc(sizeof(queue));
  if(q == NULL)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }
  q->next = NULL;
  q->t = me;
  currentThread = me;
  currentQueue = q;

  if(thread_start(func, args) == -1)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }

  interrupt_enable();
  assert_interrupts_enabled();
  return -1;

}

int thread_create(thread_startfunc_t func, void* args)
{

  interrupt_disable();

  if(!ranBefore)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }
  ucontext_t* context = (ucontext_t*) malloc(sizeof(ucontext_t));
  if(context == NULL)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }
  getcontext(context);
  if(context == NULL)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }

  char* stack = new char[STACK_SIZE];

  if(stack == NULL)
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

  thread* me = (thread*) malloc(sizeof(thread));
  if(me == NULL)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }
  me->context = context;
  me->id = -1;

  queue* que = (queue*) malloc(sizeof(queue));
  if(que == NULL)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }
  que->t = me;
  que->next = NULL;

  addReady(que);

  interrupt_enable();
  assert_interrupts_enabled();
  return 0;
}

int thread_lock(unsigned int lock)
{
  interrupt_disable();
  assert_interrupts_disabled();
  if(!ranBefore)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }
  if(locks == NULL)
  {
    //cout << "no locks, making locks" << endl;
    //there are no locks, so initialize locks;
    locks = (lockt*) malloc(sizeof(lockt));
    if(locks ==NULL)
    {
      interrupt_enable();
      assert_interrupts_enabled();
      return -1;
    }
    assert(locks!= NULL);
    locks->id = currentThread->id;
    locks->lock = lock;
    locks->locked = true;
    locks->next = NULL;
    locks->lockQueue = NULL;
    locks->condQueue = NULL;
  //  cout << "locking lock " << locks->lock << " with id " << locks->id << endl;
    interrupt_enable();
    assert_interrupts_enabled();
    return 0;
  }
  if(!lockExist(lock))
  {
    //cout << "lock " << lock << " doesn't exist" << endl;
    lockt* temp = locks;
    while(temp->next!=NULL)
    {
      temp = temp->next;
    }
    temp->next = (lockt*) malloc(sizeof(lockt));
    //allocate space for the next lock
    if(temp->next ==NULL)
    {
      interrupt_enable();
      assert_interrupts_enabled();
      return -1;
    }
    temp = temp->next;
    //move to the allocated space;
    temp->id = currentThread->id;
    temp->lock = lock;
    temp->locked = true;
    temp->next = NULL;
    temp->lockQueue = NULL;
    temp->condQueue = NULL;
  //  cout << "locking lock " << temp->lock << " with id " << temp->id << endl;
    //initialize everything and return locked
    //cout << "Lock: " << lock <<  " lock doesn't exist return" << endl;
    interrupt_enable();
    assert_interrupts_enabled();
    return 0;
  }
  if(ismylock(lock) && islocked(lock))
  {
    //cout << "lock " << lock << " is my lock and is locked" << endl;
    //you can't lock your own lock silly
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }

  //so locks is not null, the lock exists, and you don't own it.
  lockt* temp = findLock(lock);
  if(!(temp->locked))
  {
    //cout << "Lock: " << lock << " is not locked return" << endl;
    temp->id = currentThread->id;
    temp->locked = true;
    interrupt_enable();
    assert_interrupts_enabled();
    return 0;
  }
  //put self on queue for lock.
  //cout << "put id " << currentThread->id << " on queue for lock " << lock << endl;
  queue* q = NULL;
  if(temp->lockQueue == NULL)
  {
    //cout << "Lock: queue for lock " << lock << " is null" << endl;
    temp->lockQueue = currentQueue;
    q = temp->lockQueue;

    q->next = NULL;
    //q->t = (thread*) malloc(sizeof(thread));
    q->t = currentThread;
    if(q->t == NULL)
    {
      interrupt_enable();
      assert_interrupts_enabled();
      return -1;
    }
    //q->t->id = currentThread->id;

   //cout << "Lock: put my context and id " << currentThread->id << " in queue" << endl;

   queue* pop = popReady();
   if(pop == NULL)
   {
     cleanup();
   }
   swapret(q, pop);

   assert(temp!=NULL);
   assert(!temp->locked);
   temp->id = currentThread->id;
   temp->locked = true;


    interrupt_enable();
    assert_interrupts_enabled();
    return 0;
  }
  q = temp->lockQueue;
  while(q->next != NULL)
  {
    q= q->next;
  }
  q->next = currentQueue;
  if(q->next == NULL)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }

  q = q->next;

  q->next = NULL;
  //  q->t = (thread*) malloc(sizeof(thread));
  q->t = currentThread;
  if(q->t == NULL)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }
  //put context on the q's thread;
  //  cout << "Lock: putting context on lock, lock queue exits" << endl;

  queue* pop = popReady();
  if(pop == NULL)
  {
    cleanup();
  }
  swapret(q, pop);
  assert(temp!=NULL);
  assert(!temp->locked);
  temp->id = currentThread->id;
  temp->locked = true;

  interrupt_enable();
  assert_interrupts_enabled();
  return 0;

}
int thread_lock_internal(unsigned int lock) // TO EDIT FOR MATCHING LOCK
{
  assert_interrupts_disabled();
  if(!ranBefore)
  {
    return -1;
  }
  if(locks == NULL)
  {
    //cout << "no locks, making locks" << endl;
    //there are no locks, so initialize locks;
    locks = (lockt*) malloc(sizeof(lockt));
    if(locks ==NULL)
    {
      return -1;
    }
    assert(locks!= NULL);
    locks->id = currentThread->id;
    locks->lock = lock;
    locks->locked = true;
    locks->next = NULL;
    locks->lockQueue = NULL;
    locks->condQueue = NULL;
  //  cout << "locking lock " << locks->lock << " with id " << locks->id << endl;
    return 0;
  }
  if(!lockExist(lock))
  {
    //cout << "lock " << lock << " doesn't exist" << endl;
    lockt* temp = locks;
    while(temp->next!=NULL)
    {
      temp = temp->next;
    }
    temp->next = (lockt*) malloc(sizeof(lockt));
    //allocate space for the next lock
    if(temp->next ==NULL)
    {

      return -1;
    }
    temp = temp->next;
    //move to the allocated space;
    temp->id = currentThread->id;
    temp->lock = lock;
    temp->locked = true;
    temp->next = NULL;
    temp->lockQueue = NULL;
    temp->condQueue = NULL;
  //  cout << "locking lock " << temp->lock << " with id " << temp->id << endl;
    //initialize everything and return locked
    //cout << "Lock: " << lock <<  " lock doesn't exist return" << endl;
    return 0;
  }
  if(ismylock(lock) && islocked(lock))
  {
    //cout << "lock " << lock << " is my lock and is locked" << endl;
    //you can't lock your own lock silly
    return -1;
  }

  //so locks is not null, the lock exists, and you don't own it.
  lockt* temp = findLock(lock);
  if(!(temp->locked))
  {
    //cout << "Lock: " << lock << " is not locked return" << endl;
    temp->id = currentThread->id;
    temp->locked = true;
    return 0;
  }
  //put self on queue for lock.
  //cout << "put id " << currentThread->id << " on queue for lock " << lock << endl;
  queue* q = NULL;
  if(temp->lockQueue == NULL)
  {
    //cout << "Lock: queue for lock " << lock << " is null" << endl;
    temp->lockQueue = currentQueue;
    q = temp->lockQueue;

    q->next = NULL;
    //q->t = (thread*) malloc(sizeof(thread));
    q->t = currentThread;
    if(q->t == NULL)
    {
      return -1;
    }
    //q->t->id = currentThread->id;

   //cout << "Lock: put my context and id " << currentThread->id << " in queue" << endl;

   queue* pop = popReady();
   if(pop == NULL)
   {
     cleanup();
   }
   swapret(q, pop);

   assert(temp!=NULL);
   assert(!temp->locked);
   temp->id = currentThread->id;
   temp->locked = true;


    return 0;
  }
  q = temp->lockQueue;
  while(q->next != NULL)
  {
    q= q->next;
  }
  q->next = currentQueue;
  if(q->next == NULL)
  {
    return -1;
  }

  q = q->next;

  q->next = NULL;
  //  q->t = (thread*) malloc(sizeof(thread));
  q->t = currentThread;
  if(q->t == NULL)
  {
    return -1;
  }
  //put context on the q's thread;
  //  cout << "Lock: putting context on lock, lock queue exits" << endl;

  queue* pop = popReady();
  if(pop == NULL)
  {
    cleanup();
  }
  swapret(q, pop);
  assert(temp!=NULL);
  assert(!temp->locked);
  temp->id = currentThread->id;
  temp->locked = true;

  return 0;

}

int thread_unlock_internal(unsigned int lock) // TO EDIT FOR MATCHING UNLOCK
{
  assert_interrupts_disabled();
  if(!ranBefore)
  {
    return -1;
  }

  if(!lockExist(lock))
  {
    //cout << "Unlock: " << lock << " doesn't exist, you can't unlock it" << endl;

    return -1;
  }
  if(!islocked(lock))
  {
    //cout << "Unlock: " << lock << " isn't locked, you can't unlock it" << endl;

    return -1;
  }
  if(ismylock(lock))
  {
    //cout << "Unlock: " << lock << " is my lock, starting to unlock" << endl;
    lockt* tempLock = findLock(lock);
    assert(tempLock->lock == lock);
    //cout << "current id " << currentThread->id << " should be " << tempLock->id << " on lock " << lock << endl;
    assert(tempLock->id == currentThread->id);

    queue* tempQueue;
    if(tempLock->lockQueue == NULL)
    {
      //cout << "lock " << lock << " has no lock queue" << endl;
    }
    if(tempLock->lockQueue != NULL)
    {
      tempQueue = tempLock->lockQueue;
      tempLock->lockQueue = tempLock->lockQueue->next;
      //cout << "Adding tempQueue with id " << tempQueue->t->id << endl;
      addReady(tempQueue);
    }

    tempLock->locked = false;

    return 0;

  }

  return -1;


}
int thread_unlock(unsigned int lock)
{
  interrupt_disable();
  assert_interrupts_disabled();
  if(!ranBefore)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }

  if(!lockExist(lock))
  {
    //cout << "Unlock: " << lock << " doesn't exist, you can't unlock it" << endl;
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }
  if(!islocked(lock))
  {
    //cout << "Unlock: " << lock << " isn't locked, you can't unlock it" << endl;
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }
  if(ismylock(lock))
  {
    //cout << "Unlock: " << lock << " is my lock, starting to unlock" << endl;
    lockt* tempLock = findLock(lock);
    assert(tempLock->lock == lock);
    //cout << "current id " << currentThread->id << " should be " << tempLock->id << " on lock " << lock << endl;
    assert(tempLock->id == currentThread->id);

    queue* tempQueue;
    if(tempLock->lockQueue == NULL)
    {
      //cout << "lock " << lock << " has no lock queue" << endl;
    }
    if(tempLock->lockQueue != NULL)
    {
      tempQueue = tempLock->lockQueue;
      tempLock->lockQueue = tempLock->lockQueue->next;
      //cout << "Adding tempQueue with id " << tempQueue->t->id << endl;
      addReady(tempQueue);
    }

    tempLock->locked = false;
    interrupt_enable();
    assert_interrupts_enabled();
    return 0;

  }
  interrupt_enable();
  assert_interrupts_enabled();
  return -1;
}

int thread_wait(unsigned int lock, unsigned int cond)
{
  interrupt_disable();
  assert_interrupts_disabled();
  if(!ranBefore)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }

  if(thread_unlock_internal(lock) == -1)
  {
    //you can't unlock, can't wait;
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }
  //cout << "Wait got past unlock" << endl;

  //lock is unlocked
  //need to add myself to cond queue on lock

  lockt* tempLock = findLock(lock);

  condqueue* tempQueue = tempLock->condQueue;
  condqueue* me = (condqueue*) malloc(sizeof(condqueue));
  if(me == NULL)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }
  me->t = currentThread;
  me->next = NULL;
  me->cond = cond;


  if(tempQueue != NULL)
  {

    while(tempQueue->next != NULL)
    {
      tempQueue = tempQueue->next;
    }
    //cout << "assigning non-null condqueue" << endl;
    tempQueue->next = me;

  }else
  {
    cout << "Wait: condqueue on lock " << lock << " is null" << endl;
    //cout << "assigning null condqueue on cond " << cond << " lock " << lock << endl;
    tempLock->condQueue = me;
  }


  queue* pop = popReady();
  if(pop == NULL)
  {
    cleanup();
  }
  swapret(convert(me), pop);
  if(tempQueue!=NULL)
  {
    tempQueue->next = NULL;
  }
  //cout << "assigning null condqueue" << endl;
  if(thread_lock_internal(lock) == -1)
  {
    //must get lock after leaving
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }


  interrupt_enable();
  assert_interrupts_enabled();

  return 0;
}

int thread_broadcast(unsigned int lock, unsigned int cond)
{
  interrupt_disable();
  assert_interrupts_disabled();

  if(!ranBefore)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }
  if(locks == NULL)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return 0;
  }
  if(!lockExist(lock))
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return 0;
  }
  lockt* tempLock = findLock(lock);


  if(tempLock->condQueue == NULL)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return 0;
  }
  condqueue* tempQueue = tempLock->condQueue;
  condqueue* prevQueue = NULL;
  while(tempQueue != NULL)
  {
    if(tempQueue->cond == cond)
    {
      if(prevQueue == NULL)
      {
        tempLock->condQueue = tempQueue->next;
        addReady(convert(tempQueue));
        free(tempQueue);
      }else{
        prevQueue->next = tempQueue->next;
        addReady(convert(tempQueue));
        free(tempQueue);

      }

    }
    tempQueue = tempQueue->next;
  }

  interrupt_enable();
  assert_interrupts_enabled();
  return 0;
}

int thread_signal(unsigned int lock, unsigned int cond)
{
  interrupt_disable();
  assert_interrupts_disabled();
  if(!ranBefore)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }

  if(locks == NULL)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return 0;
  }
  if(!lockExist(lock))
  {
  //  cout << "lock doesn't exist return" << endl;
    interrupt_enable();
    assert_interrupts_enabled();
    return 0;
  }
  lockt* tempLock = findLock(lock);


  if(tempLock->condQueue == NULL)
  {
      //  cout << "condqueue is null return on cond " << cond << " lock " << lock << endl;
    interrupt_enable();
    assert_interrupts_enabled();
    return 0;
  }
  condqueue* tempQueue = tempLock->condQueue;
  condqueue* prevQueue = NULL;
  while(tempQueue != NULL)
  {
    if(tempQueue->cond == cond)
    {
      if(prevQueue == NULL)
      {
        tempLock->condQueue = tempQueue->next;
        addReady(convert(tempQueue));
        free(tempQueue);

        interrupt_enable();
        assert_interrupts_enabled();
        return 0;
      }
      prevQueue->next = tempQueue->next;
      addReady(convert(tempQueue));
      free(tempQueue);

      interrupt_enable();
      assert_interrupts_enabled();
      return 0;
    }
    tempQueue = tempQueue->next;
  }

  interrupt_enable();
  assert_interrupts_enabled();
  return 0;
}

int thread_yield(void)
{

  interrupt_disable();
  assert_interrupts_disabled();

  if(!ranBefore)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }


  assert_interrupts_disabled();
      //cout << "Checking readyQueue" << endl;
  if(readyQueue==NULL)
  {
    //cout << "Yield: readyQueue is null, returning 0" << endl;
    interrupt_enable();
    assert_interrupts_enabled();
    return 0;
  }
      //cout << "Yield: popping queue" <<endl;
      addReady(currentQueue);
      queue* pop = popReady();
      if(pop == NULL)
      {
        cleanup();
      }
      swapret(currentQueue, pop);
  //cout << "Yield: came back from swap" << endl;
  interrupt_enable();
  assert_interrupts_enabled();
  return 0;

}
