#include <iostream>
#include "vm.hpp"
#include <time.h>



#define BINARY_OP(valueType, op) \
    do { \
      if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
        runtimeError("Operands must be numbers."); \
        return INTERPRET_RUNTIME_ERROR; \
      } \
      double b = AS_NUMBER(stack_pop()); \
      double a = AS_NUMBER(stack_pop()); \
      stack_push(valueType(a op b)); \
    } while (false)

void VirtualMachine::stack_push(value_t val){
    *stack_ptr = val;
    (stack_ptr)++;
}

value_t VirtualMachine::stack_pop(){
    stack_ptr--;
    return *stack_ptr;
}

value_t VirtualMachine::peek(int offset){
    return *(stack_ptr-offset-1);
}

bool VirtualMachine::isFalsey(value_t val){
    return IS_NIL(val) || (IS_BOOL(val) && !AS_BOOL(val));
}

value_t clockNative(int argCount, value_t* args){
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC );
}

bool VirtualMachine::callValue(value_t callee, int argCount){
    if (IS_OBJ(callee)){
        switch(OBJ_TYPE(callee)){
            case OBJ_CLOSURE:
                return call(AS_CLOSURE(callee), argCount);
            case OBJ_NATIVE:{
                NativeFn native = AS_NATIVE(callee);
                value_t result = native(argCount, stack_ptr - argCount);
                stack_ptr -= argCount + 1;
                stack_push(result);
                return true;
            }
            default:
                break;
        }
    }
    runtimeError("Can only call function and classes.");
    return false;
}

ObjUpvalue* VirtualMachine::captureUpvalue(value_t* local){
    ObjUpvalue* prevUpvalue = NULL;
    ObjUpvalue* upvalue = openUpvalues;

    while(upvalue != NULL && upvalue->location > local){
        prevUpvalue = upvalue;
        upvalue = upvalue->next;
    }

    if(upvalue != NULL && upvalue->location == local){
        return upvalue;
    }

    ObjUpvalue* createdUpvalue = new ObjUpvalue(local);

    if(prevUpvalue == NULL){
        openUpvalues = createdUpvalue;
    }else{
        prevUpvalue->next = createdUpvalue;
    }
    return createdUpvalue;

}

void VirtualMachine::closeUpvalues(value_t* last){
    while(openUpvalues != NULL && openUpvalues->location >= last){
        ObjUpvalue* upvalue = openUpvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        openUpvalues = upvalue->next;
    }
}

bool VirtualMachine::call(ObjClosure* closure, int argCount){
    if (argCount != closure->function->arity){
        runtimeError("Expected call");
        return false;
    }

    if(frameCount == FRAMES_MAX){
        runtimeError("Stack overflow");
        return false;
    }

    CallFrame* frame = &frames[frameCount++];
    frame->closure = closure;
    frame->ip = closure->function->chunk->getChunk()->begin();
    frame->slots = stack_ptr - argCount; // stack_ptr is pointer at top of stack
    return true;
}

void VirtualMachine::concatenate(){
    ObjString* b = AS_STRING(stack_pop());
    ObjString* a = AS_STRING(stack_pop());
    int con_length = a->length + b->length;
    std::string con_strs = a->strs + b->strs;
    ObjString* c = new ObjString{OBJ_STRING, con_length, con_strs};
    stack_push(OBJ_VAL(c));
}

InterpretResult VirtualMachine::interpret(std::string source){
    Compiler compiler(source);
    compiler.setCurrent(&compiler);
    ObjFunction* function = compiler.compile(source);
    if(function==NULL) return INTERPRET_COMPILE_ERROR;

    ObjClosure* closure = new ObjClosure(function);
    stack_push(OBJ_VAL(closure));
    // CallFrame* frame = &frames[frameCount++];
    // frame->function = std::move(function);
    // frame->ip = frame->function->chunk.getChunk()->begin();
    // frame->slots = stack_ptr;
    call(closure, 0);

    return run();
}

void VirtualMachine::runtimeError(std::string format){
    size_t offset = ip - chunk->getChunk()->begin();
    int line = chunk->getLine(offset);
    std::cout << "[line " << line << "] Error";
    for(int i = frameCount -1; i >=0; i--){
        CallFrame* frame = &frames[i];
        ObjFunction* function = frame->closure->function;
        chunk_iter instruction = frame->ip - function->chunk->getChunk()->size() - 1;
        int line = chunk->getLine(*instruction);
        std::cout << "[line " << line << "] Error";
        if (function->name == NULL){
            std::cerr << "script\n" << std::endl;
        }else{
            std::cerr << function->name << std::endl;
        }
    }
}

// void VirtualMachine::defineNative(std::string name, NativeFn funtion){
//     stack_push(OBJ_STRING);
//     stack_push()
// }

InterpretResult VirtualMachine::run(){
    CallFrame* frame = &frames[frameCount - 1];
    auto read_byte = [&](){chunk_iter ret_ip = frame->ip; frame->ip += 1; return *ret_ip;};
    auto read_short = [&](){frame->ip += 2; return (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]);};
    for(;;){
        #ifdef DEBUG_TRACE_EXECUTION
            std::cout << "             " << std::endl;
            for(stack_iter slot = stack_memory->begin(); slot != stack_ptr; slot++){
                if (slot->type == VAL_BOOL)
                    std::cout << "[" << AS_BOOL(*slot) << "]" << std::endl;
                if (slot->type == VAL_NUMBER)
                    std::cout << "[" << AS_NUMBER(*slot) << "]" << std::endl;
                if (IS_STRING(*slot))
                    std::cout << "[" << AS_CSTRING(*slot) << "]" << std::endl;
                if (IS_CLOSURE(*slot))
                    std::cout << "[closure]" << std::endl;
            }
        #endif
        
        uint8_t instruction;
        switch (instruction = read_byte()){
            case OP_CONSTANT:{
                value_t constant = frame->closure->function->chunk->getValue(read_byte());
                stack_push(constant);
                break;
            }
            case OP_NIL: stack_push(NIL_VAL); break;
            case OP_TRUE: stack_push(BOOL_VAL(true)); break;
            case OP_FALSE: stack_push(BOOL_VAL(false)); break;
            case OP_POP: stack_pop(); break;
            case OP_GET_LOCAL:{
                uint8_t slot = read_byte();
                stack_push(frame->slots[slot]);
                break;
            }
            case OP_SET_LOCAL:{
                uint8_t slot = read_byte();
                frame->slots[slot] = peek(0);
                break;
            }
            case OP_GET_GLOBAL:{
                ObjString* name = AS_STRING(frame->closure->function->chunk->getValue(read_byte()));
                value_t val;
                if (globals_table.find(name->strs) == globals_table.end()){
                    std::string format = "Undifined variable " + name->strs + ".";
                    runtimeError(format);
                    return INTERPRET_RUNTIME_ERROR;
                }else{
                    val = globals_table[name->strs];
                }
                stack_push(val);
                break;
            }
            case OP_DEFINE_GLOBAL:{
                ObjString* name = AS_STRING(frame->closure->function->chunk->getValue(read_byte()));
                globals_table[name->strs] = peek(0);
                stack_pop();
                break;
            }
            case OP_SET_GLOBAL:{
                ObjString* name = AS_STRING(frame->closure->function->chunk->getValue(read_byte()));
                if (globals_table.find(name->strs) == globals_table.end()){
                    std::string format = "Undifined variable " + name->strs + ".";
                    runtimeError(format);
                    return INTERPRET_RUNTIME_ERROR;
                }else{
                    globals_table[name->strs] = peek(0);
                }
                break;
            }
            case OP_GET_UPVALUE:{
                uint8_t slot = read_byte();
                stack_push(*frame->closure->upvalues[slot]->location);
                break;
            }
            case OP_SET_UPVALUE:{
                uint8_t slot = read_byte();
                *frame->closure->upvalues[slot]->location = peek(0);
                break;
            }
            case OP_EQUAL:{
                value_t b = stack_pop();
                value_t a = stack_pop();
                bool flag = Value::valuesEqual(a, b);
                stack_push(BOOL_VAL(flag));
                break;
            }
            case OP_GREATER: BINARY_OP(BOOL_VAL, >); break;
            case OP_LESS: BINARY_OP(BOOL_VAL, <); break;
            case OP_ADD: {
                if(IS_STRING(peek(0)) && IS_STRING(peek(1))){
                    concatenate();
                }else if(IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))){
                    BINARY_OP(NUMBER_VAL, +);
                }else{
                    runtimeError("Operands must be two number or two strings.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
            case OP_DIVIDE: BINARY_OP(NUMBER_VAL, /); break;
            case OP_NOT: stack_push(BOOL_VAL(isFalsey(stack_pop()))); break;
            case OP_NEGATE:{
                if (!IS_NUMBER(peek(0))){
                    runtimeError("Operand must be a number");
                    return INTERPRET_RUNTIME_ERROR;
                }
                stack_push(NUMBER_VAL(- AS_NUMBER(stack_pop())));
                break;
            }
            case OP_PRINT:{
                Value::printValue(stack_pop());
                std::cout << std::endl;
                break;
            }
            case OP_JUMP:{
                uint16_t offset = read_short();
                frame->ip += offset;
                break;
            }
            case OP_JUMP_IF_FALSE:{
                uint16_t offset = read_short();
                if (isFalsey(peek(0))) frame->ip += offset;
                break;
            }
            case OP_LOOP: {
                uint16_t offset = read_short();
                frame->ip -= offset;
                break; 
            }
            case OP_CALL:{
                int argCount = read_byte();
                if(!callValue(peek(argCount), argCount)){
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &frames[frameCount-1];
                break;
            }
            case OP_CLOSURE:{
                // suppose to get function obj
                value_t constant = frame->closure->function->chunk->getValue(read_byte());
                ObjFunction* function = AS_FUNCTION(constant);
                ObjClosure* closure = new ObjClosure(function);
                stack_push(OBJ_VAL(closure));

                for(int i=0; i < closure->upvalueCount; i++){
                    uint8_t isLocal = read_byte();
                    uint8_t index = read_byte();
                    if(isLocal){
                        closure->upvalues[i] = captureUpvalue(
                            &(*(frame->slots+index)));
                    }else{
                        closure->upvalues[i] = frame->closure->upvalues[index];
                    }
                }
                break;
            }
            case OP_CLOSE_UPVALUE:{
                closeUpvalues(&(*(stack_ptr-1)));
                stack_pop();
                break;
            }
            case OP_RETURN:{
                value_t result = stack_pop();
                closeUpvalues(&(*(frame->slots)));
                frameCount--;
                if(frameCount == 0){
                    stack_pop();
                    return INTERPRET_OK;
                }
                stack_ptr = frame->slots-1;
                stack_push(result);
                frame = &frames[frameCount-1];
                break;
            }
        }
    }
}


