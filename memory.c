#include <stdlib.h>

#include "compiler.h"
#include "memory.h"
#include "vm.h"
#ifdef DEBUG_LOG_GC
#include <stdio.h>
#include "debug.h"
#endif

#define GC_HEAP_GROW_FACTOR 2

static void blackenObject(Obj *object);
static void freeObject(Obj *object);
static void markArray(ValueArray *array);
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
  // We have to remove in a seperet way, because they are not allocated on the stack
  tableRemoveWhite(&vm.strings);
  // reclaim the garbage
  sweep();
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
  Obj *object = vm.objects;
  while (object != NULL)
  {
    Obj *next = object->next;
    freeObject(object);
    object = next;
  }
  free(vm.grayStack);
}

void markObject(Obj *object)
{
  if (object == NULL)
    return;
  // Object is already marked, so we don't need to mark it again
  if (object->isMarked)
    return;
#ifdef DEBUG_LOG_GC
  printf("%p mark ", (void *)object);
  printValue(OBJ_VAL(object));
  printf("\n");
#endif
  object->isMarked = true;
  if (vm.grayCapacity < vm.grayCount + 1)
  {
    vm.grayCapacity = GROW_CAPACITY(vm.grayCapacity);
    vm.grayStack = (Obj **)realloc(vm.grayStack, sizeof(Obj *) * vm.grayCapacity);
  }
  if (vm.grayStack == NULL)
    exit(1);
  vm.grayStack[vm.grayCount++] = object;
}

void markValue(Value value)
{
  if (IS_OBJ(value))
    markObject(AS_OBJ(value));
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

static void blackenObject(Obj *object)
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
    ObjBoundMethod *bound = (ObjBoundMethod *)object;
    markValue(bound->receiver);
    markObject((Obj *)bound->method);
    break;
  }
  case OBJ_CLASS:
  {
    ObjClass *kelloxClass = (ObjClass *)object;
    markObject((Obj *)kelloxClass->name);
    markTable(&kelloxClass->methods);
    break;
  }
  case OBJ_CLOSURE:
  {
    ObjClosure *closure = (ObjClosure *)object;
    markObject((Obj *)closure->function);
    for (int32_t i = 0; i < closure->upvalueCount; i++)
    {
      markObject((Obj *)closure->upvalues[i]);
    }
    break;
  }
  case OBJ_FUNCTION:
  {
    ObjFunction *function = (ObjFunction *)object;
    markObject((Obj *)function->name);
    markArray(&function->chunk.constants);
    break;
  }
  case OBJ_INSTANCE:
  {
    ObjInstance *instance = (ObjInstance *)object;
    markObject((Obj *)instance->kelloxClass);
    markTable(&instance->fields);
    break;
  }
  case OBJ_UPVALUE:
    markValue(((ObjUpvalue *)object)->closed);
    break;
  case OBJ_NATIVE:
  case OBJ_STRING:
    break;
  }
}

// Dealocates the memomory used by the object
static void freeObject(Obj *object)
{
#ifdef DEBUG_LOG_GC
  printf("%p free type %d\n", (void *)object, object->type);
#endif
  switch (object->type)
  {
  case OBJ_BOUND_METHOD:
    FREE(ObjBoundMethod, object);
    break;
  case OBJ_CLASS:
  {
    ObjClass *klass = (ObjClass *)object;
    freeTable(&klass->methods);
    FREE(ObjClass, object);
    break;
  }
  case OBJ_CLOSURE:
  {
    ObjClosure *closure = (ObjClosure *)object;
    FREE_ARRAY(ObjUpvalue *, closure->upvalues,
               closure->upvalueCount);
    FREE(ObjClosure, object);
    break;
  }
  case OBJ_FUNCTION:
  {
    ObjFunction *function = (ObjFunction *)object;
    freeChunk(&function->chunk);
    FREE(ObjFunction, object);
    break;
  }
  case OBJ_INSTANCE:
  {
    ObjInstance *instance = (ObjInstance *)object;
    freeTable(&instance->fields);
    FREE(ObjInstance, object);
    break;
  }
  case OBJ_NATIVE:
    FREE(ObjNative, object);
    break;
  case OBJ_STRING:
  {
    ObjString *string = (ObjString *)object;
    FREE_ARRAY(char, string->chars, string->length + 1);
    FREE(ObjString, object);
    break;
  }
  case OBJ_UPVALUE:
    FREE(ObjUpvalue, object);
    break;
  }
}

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
    markObject((Obj *)vm.frames[i].closure);
  }
  // all the ObjectUpvalues
  for (ObjUpvalue *upvalue = vm.openUpvalues;
       upvalue != NULL;
       upvalue = upvalue->next)
  {
    markObject((Obj *)upvalue);
  }
  // all the global variables
  markTable(&vm.globals);
  // And all the compiler roots allocated on the heap
  markCompilerRoots();
  markObject((Obj *)vm.initString);
}

/* Walks through the linked list of objects on the heap and ckecks their mark bits.
 * If an object is unmarked, it is unlinked from the list
 * and the memory used by the object is freed*/
static void sweep()
{
  Obj *previous = NULL;
  Obj *object = vm.objects;
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
      Obj *unreached = object;
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

static void traceReferences()
{
  while (vm.grayCount > 0)
  {
    Obj *object = vm.grayStack[--vm.grayCount];
    blackenObject(object);
  }
}
