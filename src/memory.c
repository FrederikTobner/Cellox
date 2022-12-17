/****************************************************************************
 * Copyright (C) 2022 by Frederik Tobner                                    *
 *                                                                          *
 * This file is part of Cellox.                                             *
 *                                                                          *
 * Permission to use, copy, modify, and distribute this software and its    *
 * documentation under the terms of the GNU General Public License is       *
 * hereby granted.                                                          *
 * No representations are made about the suitability of this software for   *
 * any purpose.                                                             *
 * It is provided "as is" without express or implied warranty.              *
 * See the <https://www.gnu.org/licenses/gpl-3.0.html/>GNU General Public   *
 * License for more details.                                                *
 ****************************************************************************/

/**
 * @file memory.c
 * @brief File containing implementation of functionality used for memory management.
 */

#include "memory.h"

#include <stdlib.h>
#include <stdio.h>

#include "compiler.h"
#ifdef DEBUG_LOG_GC
#include "debug.h"
#endif
#include "virtual_machine.h"

#define GC_HEAP_GROWTH_FACTOR (2)

static void memory_blacken_object(object_t *);
static void memory_free_object(object_t *);
static void memory_mark_array(dynamic_value_array_t *);
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
  hash_table_remove_white(&virtualMachine.strings);
  // reclaim the garbage
  memory_sweep();
  // Adjusts the threshold when the next garbage collection will occur
  virtualMachine.nextGC = virtualMachine.bytesAllocated * GC_HEAP_GROWTH_FACTOR;
#ifdef DEBUG_LOG_GC
  printf("garbage collection process has ended\n");
  printf("   collected %zu bytes (from %zu to %zu) next at %zu\n", before - virtualMachine.bytesAllocated, before, virtualMachine.bytesAllocated, virtualMachine.nextGC);
#endif
}

void memory_free_objects()
{
  object_t * object = virtualMachine.objects;
  while (object)
  {
    object_t * next = object->next;
    memory_free_object(object);
    object = next;
  }
  if(virtualMachine.grayStack)
    free(virtualMachine.grayStack);
  virtualMachine.objects = NULL;
}

void memory_mark_object(object_t * object)
{
  if (!object)
    return;
  // Object is already marked, so we don't need to mark it again
  if (object->isMarked)
    return;
#ifdef DEBUG_LOG_GC
  printf("%p marked ", (void *)object);
  value_print(OBJECT_VAL(object));
  printf("\n");
#endif
  object->isMarked = true;
  if (virtualMachine.grayCapacity < virtualMachine.grayCount + 1)
  {
    virtualMachine.grayCapacity = GROW_CAPACITY(virtualMachine.grayCapacity);
    virtualMachine.grayStack = (object_t **)realloc(virtualMachine.grayStack, sizeof(object_t *) * virtualMachine.grayCapacity);
  }
  if (!virtualMachine.grayStack)
    exit(EXIT_CODE_SYSTEM_ERROR);
  virtualMachine.grayStack[virtualMachine.grayCount++] = object;
}


void memory_mark_value(value_t value)
{
  if (IS_OBJECT(value))
    memory_mark_object(AS_OBJECT(value));
}

void * memory_reallocate(void * pointer, size_t oldSize, size_t newSize)
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
  if (!newSize)
  {
    free(pointer);
    return NULL;
  }

  void *result = realloc(pointer, newSize);
  if (!result)
  {
    fprintf(stderr, "Failed too allocate memory");
    exit(EXIT_CODE_SYSTEM_ERROR);
  }
  return result;
}

/// @brief Blackens an object
/// @param object The object that is blackened
/// @details This means all the references of this object have been marked
static void memory_blacken_object(object_t * object)
{
#ifdef DEBUG_LOG_GC
  printf("%p blackened ", (void *)object);
  value_print(OBJECT_VAL(object));
  printf("\n");
#endif
  switch (object->type)
  {
  case OBJECT_ARRAY:
  {
    object_dynamic_value_array_t * array = (object_dynamic_value_array_t *)object;
    // If a function is reachable all of the constants stored in the chunk are reachable, too.
    memory_mark_array(&array->array);
    break;
  }
  case OBJECT_BOUND_METHOD:
  {
    object_bound_method_t * bound = (object_bound_method_t *)object;
    // If a method bound to a instance is reachable the instance itself is reachable, too.
    memory_mark_value(bound->receiver);
    // If a method bound to a instance is reachable we need to preserve the function
    memory_mark_object((object_t *)bound->method);
    break;
  }
  case OBJECT_CLASS:
  {
    object_class_t * celloxClass = (object_class_t *)object;
    memory_mark_object((object_t *)celloxClass->name);
    // If a class is reachable, all the methods are reachable, too.
    hash_table_mark(&celloxClass->methods);
    break;
  }
  case OBJECT_CLOSURE:
  {
    object_closure_t * closure = (object_closure_t *)object;
    // If the closoure is reachable, the function is reachable, too.
    memory_mark_object((object_t *)closure->function);
    // If the closure is reachable all the upvalues captured by the closure are reachable, too.
    for (uint32_t i = 0; i < closure->upvalueCount; i++)
      memory_mark_object((object_t *)closure->upvalues[i]);
    break;
  }
  case OBJECT_FUNCTION:
  {
    object_function_t * function = (object_function_t *)object;
    memory_mark_object((object_t *)function->name);
    // If a function is reachable all of the constants stored in the chunk are reachable, too.
    memory_mark_array(&function->chunk.constants);
    break;
  }
  case OBJECT_INSTANCE:
  {
    object_instance_t * instance = (object_instance_t *)object;
    memory_mark_object((object_t *)instance->celloxClass);
    // If the instace is reachable all of it's fields are reachable, too.
    hash_table_mark(&instance->fields);
    break;
  }
  case OBJECT_UPVALUE:
    // If a upvalue is reachable the captured value is reachable, too.
    memory_mark_value(((object_upvalue_t *)object)->closed);
    break;
  case OBJECT_NATIVE:
  case OBJECT_STRING:
    break;
  }
}

/// @brief Dealocates the memomory used by the object
/// @param object The object that is freed
static void memory_free_object(object_t * object)
{
#ifdef DEBUG_LOG_GC
  printf("freed object %p of the type %s\n", (void *)object, object_stringify_type(object));
#endif
  switch (object->type)
  {
  case OBJECT_ARRAY:
  {
    object_dynamic_value_array_t * array = (object_dynamic_value_array_t *)object;
    dynamic_value_array_free(&array->array);
    FREE(object_dynamic_value_array_t, object);
    break;
  }
  case OBJECT_BOUND_METHOD:
    FREE(object_bound_method_t, object);
    break;
  case OBJECT_CLASS:
  {
    object_class_t * celloxClass = (object_class_t *)object;
    // If a class is unreachable, all the methods are unreachable, too.
    hash_table_free(&celloxClass->methods);
    FREE(object_class_t, object);
    break;
  }
  case OBJECT_CLOSURE:
  {
    object_closure_t * closure = (object_closure_t *)object;
    // If a closure is unreachable we also need to free all the memory used by the upvalues that are captured by the closure
    FREE_ARRAY(object_upvalue_t *, closure->upvalues, closure->upvalueCount);
    FREE(object_closure_t, object);
    break;
  }
  case OBJECT_FUNCTION:
  {
    object_function_t * function = (object_function_t *)object;
    // If a function is unreachable we also need to free all the memory used by the chunk
    chunk_free(&function->chunk);
    FREE(object_function_t, object);
    break;
  }
  case OBJECT_INSTANCE:
  {
    object_instance_t * instance = (object_instance_t *)object;
    // If a instance is unreachable we also need to free all the memory used by the fields
    hash_table_free(&instance->fields);
    FREE(object_instance_t, object);
    break;
  }
  case OBJECT_NATIVE:
    FREE(object_native_t, object);
    break;
  case OBJECT_STRING:
  {
    object_string_t * string = (object_string_t *)object;
    // If a string is unreachable we need to free the memory the underlying character sequence occupies
    FREE_ARRAY(char, string->chars, string->length + 1);
    FREE(object_string_t, object);
    break;
  }
  case OBJECT_UPVALUE:
    FREE(object_upvalue_t, object);
    break;
  }
}

/// @brief  Marks all the values in an array
/// @param array The array where all the values are marked
static void memory_mark_array(dynamic_value_array_t * array)
{
  for (int32_t i = 0; i < array->count; i++)
    memory_mark_value(array->values[i]);
}

/// @brief Marks the roots of the compiler
/// @details These are the local variables and temporaries sitting in the VirtualMachine's stack
static void memory_mark_roots()
{
  // We mark all the values
  for (value_t * slot = virtualMachine.stack; slot < virtualMachine.stackTop; slot++)
    memory_mark_value(*slot);
  // all the objects
  for (int32_t i = 0; i < virtualMachine.frameCount; i++)
    memory_mark_object((object_t *)virtualMachine.callStack[i].closure);
  // all the upvalues
  for (object_upvalue_t * upvalue = virtualMachine.openUpvalues; upvalue; upvalue = upvalue->next)
    memory_mark_object((object_t *)upvalue);
  // all the global variables
  hash_table_mark(&virtualMachine.globals);
  // and all the compiler roots allocated on the heap
  compiler_mark_roots();
  memory_mark_object((object_t *)virtualMachine.initString);
}

/** @brief Walks through the linked list of objects on the heap and checks their mark bits.
 * @details If an object is unmarked, it is unlinked from the list
 * and the memory used by the object is reclaimed
 */
static void memory_sweep()
{
  object_t * previous = NULL;
  object_t * object = virtualMachine.objects;
  while (object)
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
      // Unreachable value -> free memory used by the object
      object_t * unreached = object;
      object = object->next;
      if (previous)
        previous->next = object;
      else
        virtualMachine.objects = object;
      memory_free_object(unreached);
    }
  }
}

/// @brief Traces all the references to the objects of the virtual machine that are reachable
/// All the objects that are reachable are marked as gray after the compiler roots are marked.
static void memory_trace_references()
{
  while (virtualMachine.grayCount)
  {
    object_t * object = virtualMachine.grayStack[--virtualMachine.grayCount];
    memory_blacken_object(object);
  }
}
