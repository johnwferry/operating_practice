#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <assert.h>
#include "thread.h"
#include "interrupt.h"

using namespace std;

int queue;
struct intWrapper{
  int a;
};

void wakeup(void* args)
{
  assert_interrupts_enabled();
  cout << "Wakeup started" << endl;
  for(int i = 3; i < 20; i++)
  {
    assert(thread_signal(1,i)==0);
    assert_interrupts_enabled();
    cout << "Signaled 1" << endl;

    assert(thread_signal(2,i)==0);
    assert_interrupts_enabled();
    cout << "Signaled 2" << endl;

    assert(thread_signal(i,i)==0);
    assert_interrupts_enabled();
    cout << "Signaled " << i << " on " << i << endl;



  }

  for(int i = 3; i < 20; i++)
  {
    assert(thread_broadcast(1,i)==0);
    assert_interrupts_enabled();
    cout << "Broadcasted on 1" << endl;


    assert(thread_broadcast(2,i)==0);
    assert_interrupts_enabled();
    cout << "Broadcasted on 2" << endl;

  }

  for(int k = 0; k < 100; k++)
  {
    assert(thread_yield()==0);
    assert_interrupts_enabled();

  }
  assert(thread_lock(8)==0);
  assert_interrupts_enabled();
  cout << "wakeup took a lock" << endl;

}
void pam(void* args)
{
  assert_interrupts_enabled();
  int me = ((intWrapper*) args)->a;
  cout << "Pam" << me << " started" << endl;

  assert(thread_lock(2)==0);
  assert_interrupts_enabled();
  cout << "Pam" << me << " locked" << endl;

  assert(thread_wait(2, 2)==0);
  assert_interrupts_enabled();
  cout << "Pam" << me << " returned from wait" << endl;

  assert(thread_unlock(2)==0);
  assert_interrupts_enabled();
  cout << "Pam" << me << " unlocked" << endl;
  free(args);
}
void bob(void* args)
{
  assert_interrupts_enabled();
  int me = ((intWrapper*) args)->a;
  cout << "Bob" << me << " started" << endl;

  assert(thread_lock(2)==0);
  assert_interrupts_enabled();
  cout << "Bob" << me << " locked" << endl;

  assert(thread_wait(2, me)==0);
  assert_interrupts_enabled();
  cout << "Bob" << me << " returned from wait" << endl;

  assert(thread_unlock(2)==0);
  assert_interrupts_enabled();
  cout << "Bob" << me << " unlocked" << endl;



  free(args);
}

void dole(void* args)
{
  assert_interrupts_enabled();
  int me = ((intWrapper*) args)->a;
  cout << "Dole" << me << " started" << endl;

  assert(thread_lock(me)==0);
  assert_interrupts_enabled();
  cout << "Dole" << me << " locked" << endl;

  assert(thread_wait(me, me)==0);
  assert_interrupts_enabled();
  cout << "Dole" << me << " returned from wait" << endl;


  free(args);
}


void pong(void* args)
{
    assert_interrupts_enabled();
    int me = ((intWrapper*) args)->a;
    cout << "Pong" << me << " started" << endl;

    assert(thread_lock(1)==0);
    assert_interrupts_enabled();
    cout << "Pong" << me << " locked" << endl;

    assert(thread_wait(1, me)==0);
    assert_interrupts_enabled();
    cout << "Pong" << me << " returned from wait" << endl;

    assert(thread_unlock(1)==0);
    assert_interrupts_enabled();
    cout << "Pong" << me << " unlocked" << endl;

    assert(thread_wait(1, me)==-1);
    assert_interrupts_enabled();
    cout << "Pong" << me << " wait failed" << endl;

    assert(thread_lock(1)==0);
    assert_interrupts_enabled();
    cout << "Pong" << me << " locked" << endl;

    assert(thread_wait(1, me)==0);
    assert_interrupts_enabled();
    cout << "Pong" << me << " returned from wait" << endl;

    assert(thread_unlock(1)==0);
    assert_interrupts_enabled();
    cout << "Pong" << me << " unlocked" << endl;


  free(args);

}

void ping(void* args)
{
  assert_interrupts_enabled();
  cout << "ping is in control" << endl;
  assert(thread_lock(1)==0);
  assert_interrupts_enabled();
  cout << "ping locked 1" << endl;

  queue = 3;
  assert(thread_unlock(1)==0);
  assert_interrupts_enabled();
  cout << "ping unlocked 1" << endl;
  assert(thread_yield() == 0);
  assert_interrupts_enabled();
  cout << "ping returned from yield" << endl;
  intWrapper* temp = (intWrapper*) malloc(sizeof(intWrapper));
  temp->a = 3;

  assert(thread_signal(1,3) == 0);
    assert_interrupts_enabled();
  cout << "ping signaled" << endl;

  assert(thread_create(pong, temp) == 0);
  assert_interrupts_enabled();
  cout << "ping created pong" << endl;

  assert(thread_yield() == 0);
  assert_interrupts_enabled();
  cout << "ping returned from yield" << endl;

  assert(thread_signal(1,3) == 0);
    assert_interrupts_enabled();
  cout << "ping signaled" << endl;

  assert(thread_unlock(1)==-1);
    assert_interrupts_enabled();
  cout << "ping unlock failed 1" << endl;

  assert(thread_broadcast(1,3) == 0);
    assert_interrupts_enabled();
  cout << "ping Broadcasted" << endl;

  assert(thread_yield() == 0);
  assert_interrupts_enabled();
  cout << "ping returned from yield" << endl;


  assert(thread_broadcast(1,3) == 0);
    assert_interrupts_enabled();
  cout << "ping Broadcasted" << endl;



  while(queue < 23)
  {
    temp = (intWrapper*) malloc(sizeof(intWrapper));
    temp->a = queue;
    assert(thread_create(pong, (void*) temp)==0);
    assert_interrupts_enabled();
    cout << "ping created pong " << queue << endl;

    assert(thread_create(dole, (void*) temp)==0);
    assert_interrupts_enabled();
    cout << "ping created dole " << queue << endl;

    assert(thread_create(bob, (void*) temp)==0);
    assert_interrupts_enabled();
    cout << "ping created bob " << queue << endl;

    assert(thread_create(pam, (void*) temp)==0);
    assert_interrupts_enabled();
    cout << "ping created pam " << queue << endl;

    queue = queue + 1;
  }




  assert(thread_create(wakeup, args)==0);
  assert_interrupts_enabled();
  cout << "ping created wakeup" << endl;


}
void thing(void* bud)
{
    cout << "Entering thing" << endl;
    assert_interrupts_enabled();

    assert(thread_libinit(thing, bud) == -1);
    assert_interrupts_enabled();
      cout << "thread libinit failed" << endl;

    assert(thread_broadcast(0,1)==0);
      cout << "thread broadcast succeded" << endl;
    assert_interrupts_enabled();

    assert(thread_broadcast(1,1)==0);
      cout << "thread broadcast succeded" << endl;
    assert_interrupts_enabled();

    assert(thread_signal(0,1)==0);
      cout << "thread signal succeded" << endl;
    assert_interrupts_enabled();

    assert(thread_signal(1,1)==0);
      cout << "thread signal succeded" << endl;
    assert_interrupts_enabled();

    assert(thread_yield()==0);
      cout << "thread yield succeded" << endl;
    assert_interrupts_enabled();

    assert(thread_wait(0,1)==-1);
    assert_interrupts_enabled();
      cout << "thing wait failed" << endl;



    assert(thread_lock(1) == 0);
    assert_interrupts_enabled();
    cout << "thing locked 1" << endl;

    assert(thread_wait(0,1)==-1);
    assert_interrupts_enabled();
      cout << "thing wait failed2" << endl;

    assert(thread_create(ping, bud) == 0);
    assert_interrupts_enabled();
    cout << "thing created ping" << endl;

    assert(thread_yield() == 0);
    assert_interrupts_enabled();
    cout << "thing yield returned to thing" <<endl;

    assert(thread_unlock(0) == -1);
    assert_interrupts_enabled();
    cout << "thing unlock on non existant lock failed" << endl;

    assert(thread_lock(1) == -1);
    assert_interrupts_enabled();
    cout << "thing failed to lock own lock" << endl;

    queue = 1;

    assert(thread_unlock(1) == 0);
    assert_interrupts_enabled();
    cout << "thing unlocked 1" << endl;

    assert(thread_wait(1,1)==-1);
    assert_interrupts_enabled();
      cout << "thing wait failed" << endl;

    assert(thread_lock(1) == 0);
    assert_interrupts_enabled();
    cout << "thing locked lock 1" << endl;

    assert(queue == 1);
    cout << "unlocking doesn't yield thread control" << endl;

    assert(thread_unlock(1) == 0);
    assert_interrupts_enabled();
    cout << "thing unlocked lock 1" << endl;





}

int main(int argc, char** argv)
{
  void* fb = malloc(sizeof(int));

  assert(thread_lock(1) == -1);
  assert_interrupts_enabled();
  cout << "thread lock failed as expected" << endl;
  assert(thread_unlock(1)==-1);
  assert_interrupts_enabled();
  cout << "thread unlock failed as expected" << endl;
  assert(thread_wait(1,1)==-1);
  assert_interrupts_enabled();
  cout << "thread wait failed as expected" << endl;
  assert(thread_broadcast(1,1)==-1);
  assert_interrupts_enabled();
  cout << "thread broadcast failed as expected" << endl;
  assert(thread_signal(1,1)==-1);
  assert_interrupts_enabled();
  cout << "thread signal failed as expected" << endl;
  assert(thread_create(thing, fb)==-1);
  assert_interrupts_enabled();
    cout << "thread create failed as expected" << endl;
  if( thread_libinit(thing, (void*) fb) == -1)
  {
    assert(false);
  }
  printf("error got past thread_libinit");
  assert(false);

  return 0;
}
