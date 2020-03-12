#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <assert.h>
#include "thread.h"
#include "interrupt.h"

using namespace std;

int queue;

void ping(void* args)
{
  assert_interrupts_enabled();
  cout << "ping is in control" << endl;
  assert(thread_yield()==0);
  assert_interrupts_enabled();
  cout << "ping yielded 1" << endl;

  assert(thread_yield()==0);
  assert_interrupts_enabled();
  cout << "ping yielded 1" << endl;

  queue = 3;




}
void thing(void* bud)
{
    cout << "Entering thing" << endl;
    assert_interrupts_enabled();



    assert(thread_create(ping, bud) == 0);
    assert_interrupts_enabled();
    cout << "thing created ping" << endl;

    assert(thread_yield() == 0);
    assert_interrupts_enabled();
    cout << "thing yield returned to thing" <<endl;




}

int main(int argc, char** argv)
{
  void* fb = malloc(sizeof(int));


  if( thread_libinit(thing, (void*) fb) == -1)
  {
    assert(false);
  }
  printf("error got past thread_libinit");
  assert(false);

  return 0;
}
