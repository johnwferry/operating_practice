#include <stdio.h>  // needed for size_t
#include <sys/mman.h> // needed for mmap
#include <assert.h> // needed for asserts
#include "dmm.h"


typedef struct metadata {
  size_t size;
  struct metadata* prev;
  struct metadata* next;

}metadata_t;



static metadata_t* freelist = NULL;

metadata_t* split(metadata_t* ptr, size_t size)
{
  metadata_t* temp =  (metadata_t*)((void*)(((char*) ptr) + size + METADATA_T_ALIGNED));

(temp->prev) = ptr;
  temp->next = ptr->next;
  temp->size = ptr->size - size - METADATA_T_ALIGNED;
(ptr->next) = temp;
  ptr->size = size;
  return temp;
}


metadata_t* race(metadata_t* a)
{
  //turutle goes 1/2 speed of hare so when hare reaches end of list, turtle is just under halfway
  metadata_t* turtle = a;
  metadata_t* hare = a;
  metadata_t* temp = NULL;
  while(hare->next != NULL && (metadata_t*)(((metadata_t*)(hare->next))->next) != NULL)
  {
    hare = (metadata_t*)(((metadata_t*)(hare->next))->next);
    turtle = turtle->next;
  }
  temp = turtle->next;
  turtle->next = NULL;
  return temp;

}
metadata_t* join(metadata_t* a, metadata_t* b)
{
  metadata_t* temp;
  if(a==NULL)
  {
    return b;
  }
  if(b==NULL)
  {
    return a;
  }

  if(a < b)
  {

    a->next = join(a,b->next);
    (a->next)->prev = a;
    a->prev = NULL;
    return a;
  }

  b->next = join(b,a->next);
  (b->next)->prev =b;
  b->prev = NULL;
  return b;
}
metadata_t* sort(metadata_t* thingee)
{
  metadata_t* temp = race(thingee);
  if(!(thingee==NULL || thingee->next == NULL))
  {
    thingee = sort(thingee);
    temp = sort(temp);
   return join(temp, thingee);
  }

return thingee;
}


bool coal()
{
  metadata_t* c = NULL;
  if(freelist != NULL)
  {

    metadata_t* temp = freelist;
    while(temp->next!=NULL)
    {
      c = (metadata_t*)(((char*) temp) + METADATA_T_ALIGNED + temp->size);
      if(c == temp->next)
      {
        if(c->next != NULL)
        {
          temp->next = c->next;
          ((metadata_t*)(c->next))->prev = temp;
        }
        if(c->next == NULL)
        {
          temp->next = NULL;
        }
        temp->size = temp->size + METADATA_T_ALIGNED + c->size;

        return true;

      }
      temp = temp->next;
    }
  }
  return false;
}
bool genList = false;
void* dmalloc(size_t numbytes) {
  /* initialize through mmap call first time */
  if(freelist == NULL && genList == false) {
    genList= true;
    if(!dmalloc_init())
      return NULL;
  }
  assert(numbytes > 0);
//printf("malloc\n");
metadata_t* temp = freelist;
metadata_t* splice;
if(freelist != NULL){
  printf("%u \n",temp);
  while(temp!=NULL)
  {
  //  printf("in the while \n");
    if(temp->size >= numbytes + METADATA_T_ALIGNED)
    { //room to splice
      splice = split(temp, numbytes);
      //splice has inherented temp's next.
      if(temp->prev !=NULL)
      { //there is a previous
      //  printf("path a \n");
        splice->prev = temp->prev;
        //splice previous becomes first previous
        ((metadata_t*)(temp->prev))->next = splice;
        //previous's next becomes splice
        return (void*)(((char*) temp)+ METADATA_T_ALIGNED);
      }
      //there is no previous
      if(temp->prev ==NULL)
      {
      //  printf("path b \n");
        splice->prev = NULL;
        freelist = splice;
        return (void*)(((char*) temp)+ METADATA_T_ALIGNED);
      }

    }
/*    if(temp->size >= numbytes)
    {
      if(temp->prev == NULL && temp->next !=NULL)
      {
        print_freelist();
        assert(temp == freelist);
        freelist = freelist->next;
        print_freelist();
        freelist->prev = NULL;
        temp->next = NULL;
      }
      if(temp->prev !=NULL && temp->next ==NULL)
      {
        print_freelist();
        (temp->prev)->next = NULL;
        temp->prev = NULL;
        print_freelist();
      }
      if(temp->prev !=NULL && temp->next !=NULL)
      {
        print_freelist();
        (temp->prev)->next = temp->next;
        (temp->next)->prev = temp->prev;
        temp->prev = NULL;
        temp->next = NULL;
        print_freelist();
      }
      return (void*)(((char*) temp)+ METADATA_T_ALIGNED);
  } */
  /*  if(temp->size >= numbytes)
    {
      printf("path c \n");
      //no room to splice
      if(temp->prev !=NULL)
      {
        //something before
        printf("path d \n");
        if(temp->next !=NULL)
        {

          printf("path e \n");
          //something After; so link them
          ((metadata_t*)(temp->prev))->next = temp->next;
          ((metadata_t*)(temp->next))->prev = temp->prev;
          return (void*)(((char*) temp)+ METADATA_T_ALIGNED);
        }
        //only something before
        ((metadata_t*)(temp->prev))->next = NULL;
        return (void*)(((char*) temp)+ METADATA_T_ALIGNED);

      }
      //check for only somthing after
      if(temp->next !=NULL)
      {
        printf("path f \n");
        temp->next->prev = NULL;
        return (void*)(((char*) temp)+ METADATA_T_ALIGNED);
      }
      printf("path g \n");
      //only reached if nothing is before and nothing is After
      assert(temp == freelist);
      freelist = NULL;
      return (void*)(((char*) temp)+ METADATA_T_ALIGNED);
    } */
    //increment temp
    temp = temp->next;
  }

}
//printf("exit malloc");
return NULL;
}



void dfree(void* ptr)
{
  //printf("free\n");
  assert(ptr != NULL);
  metadata_t* temp = (metadata_t*)((void*)(((char*)ptr) - METADATA_T_ALIGNED));

  if(freelist != NULL)
  {

  metadata_t* temp2 = freelist->next;
  freelist->next = temp;
  temp->prev = freelist;
  temp->next = temp2;
  if(temp2 != NULL)
  {
      temp2->prev = temp;
  }
  freelist =sort(freelist);
  while(coal())
  {freelist =sort(freelist);};
}else
{
  freelist = temp;
}

}



bool dmalloc_init() {

  size_t max_bytes = ALIGN(MAX_HEAP_SIZE);

  freelist = (metadata_t*) mmap(NULL, max_bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (freelist == (void *)-1)
    return false;
  freelist->next = NULL;
  freelist->prev = NULL;
  freelist->size = max_bytes - METADATA_T_ALIGNED;

  return true;
}


void print_freelist() {
  metadata_t *freelist_head = freelist;
  while(freelist_head != NULL)
  {
    printf("Prev %u, next %u, size %u \n", freelist_head->prev, freelist_head->next, freelist_head->size);
    freelist_head = freelist_head->next;
  }

}
