#include <iostream>

#define TYPE int
typedef struct{
    TYPE* start;
    TYPE* curr;
    TYPE* end;
}Stack;

int Stack_resize(Stack* stack, unsigned long long new_size){
#ifdef DEBUG_FCALL
    printf("resizing stack at address %p\n", stack);
    printf("from size %d to %d", (stack->end - stack->start), new_size);
#endif
    TYPE* new_start = (TYPE*) calloc(new_size, sizeof(TYPE));
    if (new_start == nullptr) {
#ifdef DUBUG_ALLOC_FAIL
        printf("allocation failed for stack at address %p\n", stack);
        printf("in function Stack_resize");
#endif
        return -1;
    }

    TYPE* old_stack_it = stack->start;
    TYPE* new_stack_it = new_start;
    while(old_stack_it != stack->curr){
        *new_stack_it = *old_stack_it;
        ++new_stack_it;
        ++old_stack_it;
    }

    free(stack->start);

    stack->curr = new_start + (stack->curr - stack->start);
    stack->start = new_start;
    stack->end = new_start + new_size;

    return 0;
}

int Stack_push(Stack* stack, const TYPE* value){
#ifdef  DEBUG_FCALL
    printf("call of Stack_push for stack at address %p\n", stack);
#endif
    *(stack->curr) = *value;
    ++stack->curr;
    if (stack->curr == stack->end) {
#ifdef DEBUG_ALLOC_FAIL
        printf("call function Stack_resize from function Stack_push\n");
        printf("for stack at address %p", stack);
        int retval = Stack_resize(stack, (stack->end - stack->start) * 2);
        if (!retval) printf("fail call Stack_resize for stack at address %p", stack);
#else
        return Stack_resize(stack, (stack->end - stack->start) * 2);
#endif
    }
    return 0;
}

int Stack_pop (Stack* stack, TYPE* value){
#ifdef  DEBUG_FCALL
    printf("call of Stack_pop for stack at address %p\n", stack);
#endif
    --stack->curr;
    if (stack->curr < stack->start) {
#ifdef  STACK_ERROR
        printf("error: can't pop one more element\n");
        printf("the stack at address %p is already empty", stack);
#endif
        return -1;
    }
    *value = *(stack->curr);
    if (stack->curr < stack->start + (stack->start - stack->end) / 2)
        Stack_resize(stack, (stack->end - stack->start) / 2);
}

unsigned long long Stack_size(const Stack* stack){
    return stack->end - stack->start;
}

int Stack_empty(const Stack* stack){
    return stack->curr == stack->start;
}
#undef TYPE

int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}