#include "memory.h"

#include <stdlib.h>

#include "compiler.h"
#include "vm.h"
#ifdef DEBUG_LOG_GC
#include <stdio.h>
#include "debug.h"
#endif

#define GC_HEAP_GROW_FACTOR 2

static void blackenObject(Object *);
static void freeObject(Object *);
static void markArray(ValueArray *);
static void markRoots();
static void sweep();
static void traceReferences();

void collectGarbage()
{
#ifdef DEBUG_LOG_GC
  printf("-- gc begin\n");
  size_t before = vm.bytesAllocated;
#endif
  markRoots();
  traceReferences();
  // We have to remove the strings with a another method, because they have their own hashtable
  tableRemoveWhite(&vm.strings);
  // reclaim the garbage
  sweep();
  // Adjusts the threshold when the next garbage collection will occur
  vm.nextGC = vm.bytesAllocated * GC_HEAP_GROW_FACTOR;
#ifdef DEBUG_LOG_GC
  printf("-- gc end\n");
  printf("   collected %zu bytes (from %zu to %zu) next at %zu\n",
         before - vm.bytesAllocated, before, vm.bytesAllocated,
         vm.nextGC);
#endif
}

void freeObjects()
{
  Object *object = vm.objects;
  while (object != NULL)
  {
    Object *next = object->next;
    freeObject(object);
    object = next;
  }
  free(vm.grayStack);
}

void markObject(Object *object)
{
  if (object == NULL)
    return;
  // Object is already marked, so we don't need to mark it again
  if (object->isMarked)
    return;
#ifdef DEBUG_LOG_GC
  printf("%p marked ", (void *)object);
  printValue(OBJ_VAL(object));
  printf("\n");
#endif
  object->isMarked = true;
  if (vm.grayCapacity < vm.grayCount + 1)
  {
    vm.grayCapacity = GROW_CAPACITY(vm.grayCapacity);
    vm.grayStack = (Object **)realloc(vm.grayStack, sizeof(Object *) * vm.grayCapacity);
  }
  if (vm.grayStack == NULL)
    exit(1);
  vm.grayStack[vm.grayCount++] = object;
}

void markValue(Value value)
{
  if (IS_OBJECT(value))
    markObject(AS_OBJECT(value));
}

// Helper method for reallocating the memory used by a dynamic Array
void *reallocate(void *pointer, size_t oldSize, size_t newSize)
{
  vm.bytesAllocated += newSize - oldSize;
  if (newSize > oldSize)
  {
#ifdef DEBUG_STRESS_GC
    collectGarbage();
#endif
    if (vm.bytesAllocated > vm.nextGC)
    {
      collectGarbage();
    }
  }
  if (newSize == 0)
  {
    free(pointer);
    return NULL;
  }

  void *result = realloc(pointer, newSize);
  if (result == NULL)
    exit(1);
  return result;
}

// Blackens an object - all the references that this object have been marked
static void blackenObject(Object *object)
{
#ifdef DEBUG_LOG_GC
  printf("%p blacken ", (void *)object);
  printValue(OBJ_VAL(object));
  printf("\n");
#endif
  switch (object->type)
  {
  case OBJ_BOUND_METHOD:
  {
    ObjectBoundMethod *bound = (ObjectBoundMethod *)object;
    markValue(bound->receiver);
    markObject((Object *)bound->method);
    break;
  }
  case OBJ_CLASS:
  {
    ObjectClass *celloxClass = (ObjectClass *)object;
    markObject((Object *)celloxClass->name);
    markTable(&celloxClass->methods);
    break;
  }
  case OBJ_CLOSURE:
  {
    ObjectClosure *closure = (ObjectClosure *)object;
    markObject((Object *)closure->function);
    for (int32_t i = 0; i < closure->upvalueCount; i++)
    {
      markObject((Object *)closure->upvalues[i]);
    }
    break;
  }
  case OBJ_FUNCTION:
  {
    ObjectFunction *function = (ObjectFunction *)object;
    markObject((Object *)function->name);
    markArray(&function->chunk.constants);
    break;
  }
  case OBJ_INSTANCE:
  {
    ObjectInstance *instance = (ObjectInstance *)object;
    markObject((Object *)instance->celloxClass);
    markTable(&instance->fields);
    break;
  }
  case OBJ_UPVALUE:
    markValue(((ObjectUpvalue *)object)->closed);
    break;
  case OBJ_NATIVE:
  case OBJ_STRING:
    break;
  }
}

// Dealocates the memomory used by the object
static void freeObject(Object *object)
{
#ifdef DEBUG_LOG_GC
  printf("%p free type %d\n", (void *)object, object->type);
#endif
  switch (object->type)
  {
  case OBJ_BOUND_METHOD:
    FREE(ObjectBoundMethod, object);
    break;
  case OBJ_CLASS:
  {
    ObjectClass *klass = (ObjectClass *)object;
    freeTable(&klass->methods);
    FREE(ObjectClass, object);
    break;
  }
  case OBJ_CLOSURE:
  {
    ObjectClosure *closure = (ObjectClosure *)object;
    FREE_ARRAY(ObjectUpvalue *, closure->upvalues,
               closure->upvalueCount);
    FREE(ObjectClosure, object);
    break;
  }
  case OBJ_FUNCTION:
  {
    ObjectFunction *function = (ObjectFunction *)object;
    freeChunk(&function->chunk);
    FREE(ObjectFunction, object);
    break;
  }
  case OBJ_INSTANCE:
  {
    ObjectInstance *instance = (ObjectInstance *)object;
    freeTable(&instance->fields);
    FREE(ObjectInstance, object);
    break;
  }
  case OBJ_NATIVE:
    FREE(ObjectNative, object);
    break;
  case OBJ_STRING:
  {
    ObjectString *string = (ObjectString *)object;
    FREE_ARRAY(char, string->chars, string->length + 1);
    FREE(ObjectString, object);
    break;
  }
  case OBJ_UPVALUE:
    FREE(ObjectUpvalue, object);
    break;
  }
}

// Marks all the values in an array
static void markArray(ValueArray *array)
{
  for (int32_t i = 0; i < array->count; i++)
  {
    markValue(array->values[i]);
  }
}

// Marks the roots - local variables or temporaries sitting in the VM's stack
static void markRoots()
{
  // We mark all the values
  for (Value *slot = vm.stack; slot < vm.stackTop; slot++)
  {
    markValue(*slot);
  }
  // all the objects
  for (int32_t i = 0; i < vm.frameCount; i++)
  {
    markObject((Object *)vm.callStack[i].closure);
  }
  // all the ObjectUpvalues
  for (ObjectUpvalue *upvalue = vm.openUpvalues;
       upvalue != NULL;
       upvalue = upvalue->next)
  {
    markObject((Object *)upvalue);
  }
  // all the global variables
  markTable(&vm.globals);
  // And all the compiler roots allocated on the heap
  markCompilerRoots();
  markObject((Object *)vm.initString);
}

/* Walks through the linked list of objects on the heap and checks their mark bits.
 * If an object is unmarked, it is unlinked from the list
 * and the memory used by the object is reclaimed*/
static void sweep()
{
  Object *previous = NULL;
  Object *object = vm.objects;
  while (object != NULL)
  {
    if (object->isMarked)
    {
      // We need to unmark the object so it is picked up during the next grabage collection process
      object->isMarked = false;
      previous = object;
      object = object->next;
    }
    else
    {
      Object *unreached = object;
      object = object->next;
      if (previous != NULL)
      {
        previous->next = object;
      }
      else
      {
        vm.objects = object;
      }

      freeObject(unreached);
    }
  }
}

// Traces all the references that the objects of the vm contain
static void traceReferences()
{
  while (vm.grayCount > 0)
  {
    Object *object = vm.grayStack[--vm.grayCount];
    blackenObject(object);
  }
}
