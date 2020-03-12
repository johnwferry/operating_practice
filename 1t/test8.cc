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
  cout << "2 " << endl;





}
void thing(void* bud)
{

for(int i = 0; i < 10000; i++){
  cout << "0 " << endl;
  assert(thread_create(ping, bud) == 0);
  assert_interrupts_enabled();
  cout << "1 " << i << endl;

  assert(thread_yield()==0);
  assert_interrupts_enabled();
  cout << "3 " << endl;
}








}

int main(int argc, char** argv)
{
    start_preemptions(true, true, 1539547544);
  void* fb = malloc(sizeof(int));


  if( thread_libinit(thing, (void*) fb) == -1)
  {
    assert(false);
  }
  printf("error got past thread_libinit");
  assert(false);

  return 0;
}
