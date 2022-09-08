#include "memory.h"

#include <stdlib.h>

#include "compiler.h"
#include "virtual_machine.h"
#ifdef DEBUG_LOG_GC
#include <stdio.h>
#include "debug.h"
#endif

#define GC_HEAP_GROW_FACTOR 2

static void memory_blacken_object(Object *);
static void memory_free_object(Object *);
static void memory_mark_array(DynamicArray *);
static void memory_mark_roots();
static void memory_sweep();
static void memory_trace_references();

void memory_collect_garbage()
{
#ifdef DEBUG_LOG_GC
  printf("garbage collection process has begun\n");
  size_t before = virtualMachine.bytesAllocated;
#endif
  memory_mark_roots();
  memory_trace_references();
  // We have to remove the strings with a another method, because they have their own hashtable
  table_remove_white(&virtualMachine.strings);
  // reclaim the garbage
  memory_sweep();
  // Adjusts the threshold when the next garbage collection will occur
  virtualMachine.nextGC = virtualMachine.bytesAllocated * GC_HEAP_GROW_FACTOR;
#ifdef DEBUG_LOG_GC
  printf("garbage collection process has ended\n");
  printf("   collected %zu bytes (from %zu to %zu) next at %zu\n", before - virtualMachine.bytesAllocated, before, virtualMachine.bytesAllocated, virtualMachine.nextGC);
#endif
}

void memory_free_objects()
{
  Object *object = virtualMachine.objects;
  while (object != NULL)
  {
    Object *next = object->next;
    memory_free_object(object);
    object = next;
  }
  free(virtualMachine.grayStack);
}

void memory_mark_object(Object *object)
{
  if (object == NULL)
    return;
  // Object is already marked, so we don't need to mark it again
  if (object->isMarked)
    return;
#ifdef DEBUG_LOG_GC
  printf("%p marked ", (void *)object);
  value_print(OBJ_VAL(object));
  printf("\n");
#endif
  object->isMarked = true;
  if (virtualMachine.grayCapacity < virtualMachine.grayCount + 1)
  {
    virtualMachine.grayCapacity = GROW_CAPACITY(virtualMachine.grayCapacity);
    virtualMachine.grayStack = (Object **)realloc(virtualMachine.grayStack, sizeof(Object *) * virtualMachine.grayCapacity);
  }
  if (virtualMachine.grayStack == NULL)
    exit(1);
  virtualMachine.grayStack[virtualMachine.grayCount++] = object;
}

void memory_mark_value(Value value)
{
  if (IS_OBJECT(value))
    memory_mark_object(AS_OBJECT(value));
}

// Helper method for reallocating the memory used by a dynamic Array
void *memory_reallocate(void *pointer, size_t oldSize, size_t newSize)
{
  virtualMachine.bytesAllocated += newSize - oldSize;
  if (newSize > oldSize)
  {
#ifdef DEBUG_STRESS_GC
    memory_collect_garbage();
#endif
    if (virtualMachine.bytesAllocated > virtualMachine.nextGC)
      memory_collect_garbage();
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
static void memory_blacken_object(Object *object)
{
#ifdef DEBUG_LOG_GC
  printf("%p blacken ", (void *)object);
  value_print(OBJ_VAL(object));
  printf("\n");
#endif
  switch (object->type)
  {
  case OBJECT_BOUND_METHOD:
  {
    ObjectBoundMethod *bound = (ObjectBoundMethod *)object;
    memory_mark_value(bound->receiver);
    memory_mark_object((Object *)bound->method);
    break;
  }
  case OBJECT_CLASS:
  {
    ObjectClass *celloxClass = (ObjectClass *)object;
    memory_mark_object((Object *)celloxClass->name);
    table_mark(&celloxClass->methods);
    break;
  }
  case OBJECT_CLOSURE:
  {
    ObjectClosure *closure = (ObjectClosure *)object;
    memory_mark_object((Object *)closure->function);
    for (uint32_t i = 0; i < closure->upvalueCount; i++)
    {
      memory_mark_object((Object *)closure->upvalues[i]);
    }
    break;
  }
  case OBJECT_FUNCTION:
  {
    ObjectFunction *function = (ObjectFunction *)object;
    memory_mark_object((Object *)function->name);
    memory_mark_array(&function->chunk.constants);
    break;
  }
  case OBJECT_INSTANCE:
  {
    ObjectInstance *instance = (ObjectInstance *)object;
    memory_mark_object((Object *)instance->celloxClass);
    table_mark(&instance->fields);
    break;
  }
  case OBJECT_UPVALUE:
    memory_mark_value(((ObjectUpvalue *)object)->closed);
    break;
  case OBJECT_NATIVE:
  case OBJECT_STRING:
    break;
  }
}

// Dealocates the memomory used by the object
static void memory_free_object(Object *object)
{
#ifdef DEBUG_LOG_GC
  printf("freed object %p of the type %d\n", (void *)object, object->type);
#endif
  switch (object->type)
  {
  case OBJECT_BOUND_METHOD:
    FREE(ObjectBoundMethod, object);
    break;
  case OBJECT_CLASS:
  {
    ObjectClass *celloxClass = (ObjectClass *)object;
    table_free(&celloxClass->methods);
    FREE(ObjectClass, object);
    break;
  }
  case OBJECT_CLOSURE:
  {
    ObjectClosure *closure = (ObjectClosure *)object;
    FREE_ARRAY(ObjectUpvalue *, closure->upvalues, closure->upvalueCount);
    FREE(ObjectClosure, object);
    break;
  }
  case OBJECT_FUNCTION:
  {
    ObjectFunction *function = (ObjectFunction *)object;
    chunk_free(&function->chunk);
    FREE(ObjectFunction, object);
    break;
  }
  case OBJECT_INSTANCE:
  {
    ObjectInstance *instance = (ObjectInstance *)object;
    table_free(&instance->fields);
    FREE(ObjectInstance, object);
    break;
  }
  case OBJECT_NATIVE:
    FREE(ObjectNative, object);
    break;
  case OBJECT_STRING:
  {
    ObjectString *string = (ObjectString *)object;
    FREE_ARRAY(char, string->chars, string->length + 1);
    FREE(ObjectString, object);
    break;
  }
  case OBJECT_UPVALUE:
    FREE(ObjectUpvalue, object);
    break;
  }
}

// Marks all the values in an array
static void memory_mark_array(DynamicArray *array)
{
  for (int32_t i = 0; i < array->count; i++)
    memory_mark_value(array->values[i]);
}

// Marks the roots - local variables or temporaries sitting in the VirtualMachine's stack
static void memory_mark_roots()
{
  // We mark all the values
  for (Value *slot = virtualMachine.stack; slot < virtualMachine.stackTop; slot++)
    memory_mark_value(*slot);
  // all the objects
  for (int32_t i = 0; i < virtualMachine.frameCount; i++)
    memory_mark_object((Object *)virtualMachine.callStack[i].closure);
  // all the ObjectUpvalues
  for (ObjectUpvalue *upvalue = virtualMachine.openUpvalues; upvalue != NULL; upvalue = upvalue->next)
    memory_mark_object((Object *)upvalue);
  // all the global variables
  table_mark(&virtualMachine.globals);
  // And all the compiler roots allocated on the heap
  compiler_mark_roots();
  memory_mark_object((Object *)virtualMachine.initString);
}

/* Walks through the linked list of objects on the heap and checks their mark bits.
 * If an object is unmarked, it is unlinked from the list
 * and the memory used by the object is reclaimed*/
static void memory_sweep()
{
  Object *previous = NULL;
  Object *object = virtualMachine.objects;
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
        previous->next = object;
      else
        virtualMachine.objects = object;
      memory_free_object(unreached);
    }
  }
}

// Traces all the references that the objects of the virtualMachine contain
static void memory_trace_references()
{
  while (virtualMachine.grayCount > 0)
  {
    Object *object = virtualMachine.grayStack[--virtualMachine.grayCount];
    memory_blacken_object(object);
  }
}
