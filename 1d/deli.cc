#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <assert.h>
#include "thread.h"
#include "interrupt.h"

using namespace std;

int maxOrders = 0;

struct FuckingBullshit{
  int argc;
  char** argv;
};
struct Order{
  int sw;
  int id;
  bool inuse;
};
Order* board = NULL;
int numInUse;
int livingCashiers;
static const unsigned int lock = 1;
static const unsigned int chefCond = 0;
static const unsigned int openCond = 1;



struct CashierArgs{
  char* filename;
  int id;
};
struct ChefArgs{
  int nothing;
};


void chef(void* useless)
{
  ChefArgs* thingee = (ChefArgs*) useless;
  int lastSw = thingee->nothing;


  assert(thread_lock(lock)!=-1);
  cheflock:
  {

  }
  if(livingCashiers == -2)
  {
//   printf("Chef is closing up shop \n");

    return;
  }
//  printf("Chef has the lock and now is checking if board is full \n");
//  printf(" numInUse is %i, maxOrders is %i, livingCashiers is %i \n", numInUse, maxOrders, livingCashiers);
  while(numInUse != maxOrders && numInUse != livingCashiers)
  {
//   printf("Board is not full, chef is waiting for an order to be placed \n");
    thread_wait(lock, chefCond);
  }
//  printf("Chef has found board to be full, searching for next sw \n Previous sandwitch was %i \n", lastSw);
  int next = -1;
  int nextDif = 1005;
  int testDif;
  for(int i = 0; i < maxOrders; i++)
  {
  //  printf("Chef is checking order %i \n", i);
    if((board+i)->inuse)
    {
    //  printf("Chef order %i is a valid order with sandwitch %i \n", i, (board+i)->sw );
      testDif = abs((board+i)->sw - lastSw);
      if(testDif < nextDif)
      {
      //  printf("Sandwitch %i is closer than %i to %i \n", (board+i)->sw, (board+next)->sw, lastSw);
        nextDif = testDif;
        next = i;
      }

    }
  }
  assert(next != -1);
//  printf("Chef is taking down order %i \n", next);
  (board+next)->inuse = false;
  numInUse = numInUse -1;
  cout << "READY: cashier " << (board+next)->id -2 << " sandwich " << (board+next)->sw << endl;
//  printf("READY: cashier %i sandwitch %i\n", (board+next)->id -2, (board+next)->sw);
  lastSw =(board+next)->sw;
//  printf("\n Shouting on id %i \n", (board+next)->id);
  thread_broadcast(lock, (board+next)->id);
  goto cheflock;




}
void cashier(void* input)
{
  CashierArgs* ca = (CashierArgs*) input;
  char* filename = ca->filename;
  int id = ca->id;
  int sw = -1;
  FILE* fp = fopen(filename, "r");

  if(fp == NULL)
  {
    //printf("Failed to open file %s \n", filename);
    exit(1);
  }

  assert(thread_lock(lock) !=-1);
  if(livingCashiers==-1)
  {
    livingCashiers = 0;
  }
  livingCashiers = livingCashiers +1;
//  printf("Cashier %i has incremented %i \n", id-2,livingCashiers);

  while(fscanf(fp, "%d", &sw) == 1)
  {

    if(sw < 0 || sw >= 1000)
    {
      goto nextSandwitch;
    }
    while(numInUse == maxOrders)
    {
    //  printf("Cashier %i is waiting for a spot on the board \n", id);
      thread_wait(lock, openCond);

    //  printf("Cashier %i has found a spot on the board \n", id);

    }

    for(int i = 0; i < maxOrders; i++)
    {
    //  printf("Cashier %i is checking board spot %i \n", id, i);
      if(!(board+i)->inuse)
      {
      //  printf("Cashier %i has found a spot at %i \n", id, i);
        (board+i)->inuse = true;
        numInUse = numInUse + 1;
        (board+i)->id = id;
        (board+i)->sw = sw;
        cout << "POSTED: cashier " << id-2 << " sandwich " << sw << endl;
      //  printf("POSTED: cashier %i sandwitch %i\n", id-2, sw);
        thread_broadcast(lock, chefCond);
        while((board+i)->inuse)
        {
          thread_wait(lock, id);
        //  printf("\n Heard on %i \n", id);
        }
        assert(!((board+i)->inuse));
        thread_broadcast(lock, openCond);

        goto nextSandwitch;
      }
    }
    printf("Bad jack. You messed up somewhere");
    nextSandwitch:
    {

    }
  }

  if(livingCashiers==1)
  {
    livingCashiers = -1;
  }
  livingCashiers = livingCashiers -1;
 //printf("Cashier %i has left the building, Living %i \n", id-2, livingCashiers);
 thread_broadcast(lock, openCond);
 thread_broadcast(lock, chefCond);

  thread_unlock(lock);




}

void thing(void* bullshit)
{
  FuckingBullshit* shit = (FuckingBullshit*) bullshit;
  int argc = shit->argc;
  char** argv = shit->argv;

  //make the board;
  board = (Order*) malloc(sizeof(Order)*maxOrders);
  assert(board != NULL);
  for(int i = 0; i < maxOrders; i++)
  {
    (board + i)->inuse = false;
  }

  numInUse = 0;
  livingCashiers = -1;
  for(int i = 2; i < argc; i++)
  {
    CashierArgs* temp = (CashierArgs*) malloc(sizeof(CashierArgs));
    temp->filename = argv[i];
    temp->id = i;
    if(thread_create(cashier, temp)==-1)
    {
      //printf("Failed to generate cashier with filename %s \n", temp->filename);
      exit(1);
    }
  }
  ChefArgs* really = (ChefArgs*) malloc(sizeof(ChefArgs));
  really->nothing = -1;
  if(thread_create(chef, really)==-1)
  {
    exit(1);
  }

}


int main(int argc, char** argv)
{
  start_preemptions(true, true, 1539547544);
  FuckingBullshit* fb =(FuckingBullshit*) malloc(sizeof(FuckingBullshit));
  assert(fb != NULL);
  assert(argc != 0);
  assert(argv != NULL);
  assert(argc >2);
  fb->argc = argc;
  fb->argv = argv;
  maxOrders = atoi(argv[1]);
//  printf("maxOrders is %i \n", maxOrders);
  assert(maxOrders > 0);

  if( thread_libinit(thing, (void*) fb) == -1)
  {
    return -1;
  }
  printf("error got past thread_libinit");
  assert(-1 != -1);

  return 0;
}
