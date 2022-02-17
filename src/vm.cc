#include "vm.hpp"
#include <iostream>


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
            case OBJ_CLASS:{
                // instantiate class
                // if there is init method, call it first
                ObjClass* klass = AS_CLASS(callee);
                stack_ptr[-argCount -1] = OBJ_VAL(new ObjInstance(klass));
                if(!(klass->methods.find("init") == klass->methods.end())){
                    value_t initializer;
                    initializer = klass->methods["init"];
                    call(AS_CLOSURE(initializer), argCount);
                }else if(argCount != 0){
                    runtimeError("Expected 0 arguments but got some.");
                    return false;
                }
                return true;
            }
            case OBJ_BOUND_METHOD:{
                ObjBoundMethod* bound = AS_BOUND_METHOD(callee);
                stack_ptr[-argCount -1] = bound->receiver;
                return call(bound->method, argCount);
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

void VirtualMachine::defineNative(
    std::string name, NativeFn function){
    ObjString* strObj = new ObjString;
    // strObj->strs = name;
    // strObj->length = name.end() - name.begin();
    // stack_push(OBJ_VAL(strObj));

    ObjNative* native = new ObjNative(function);
    // stack_push(OBJ_VAL(native));

    globals_table[name] = OBJ_VAL(native);
}

void VirtualMachine::defineMethod(ObjString* name){
    value_t method = peek(0);
    ObjClass* klass = AS_CLASS(peek(1));
    klass->methods[name->strs] = method;
    stack_pop();
}

bool VirtualMachine::bindMethod(ObjClass* klass, ObjString* name){
    if(klass->methods.find(name->strs) == klass->methods.end()){
        runtimeError("Undefined proprety.");
        return false;
    }
    value_t method = klass->methods[name->strs];
    ObjBoundMethod* bound = new ObjBoundMethod(peek(0), AS_CLOSURE(method));
    stack_pop();
    stack_push(OBJ_VAL(bound));
    return true;
}

bool VirtualMachine::call(ObjClosure* closure, int argCount){
    if (argCount != closure->function->arity){
        std::string error_msg = "Expected " + std::to_string(closure->function->arity)+\
                                " arguments bet got " + std::to_string(argCount);
        runtimeError(error_msg);
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

bool VirtualMachine::invokeFromClass(ObjClass* klass, ObjString* name,
                            int argCount) {
    if (klass->methods.find(name->strs) == klass->methods.end()){
        runtimeError("Undefined property.");
        return false;
    }

    value_t method = klass->methods[name->strs];
    return call(AS_CLOSURE(method), argCount);
}

bool VirtualMachine::invoke(ObjString* name, int argCount) {
    value_t receiver = peek(argCount);

    if (!IS_INSTANCE(receiver)) {
        runtimeError("Only instances have methods.");
        return false;
    }

    ObjInstance* instance = AS_INSTANCE(receiver);

    value_t value;
    if (!(instance->fields.find(name->strs) == instance->fields.end())){
        stack_ptr[-argCount - 1] = value;
        return callValue(value, argCount);
    }

  return invokeFromClass(instance->klass, name, argCount);
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
    call(closure, 0);

    return run();
}

void VirtualMachine::runtimeError(std::string format){
    std::cout << "Traceback (most recent call last):" << std::endl;
    for(int i = 0; i < frameCount; i++){
        CallFrame* frame = &frames[i];
        uint8_t offset = frame->ip - frame->closure->function->chunk->getChunk()->begin();
        int line = frame->closure->function->chunk->getLine(offset);
        std::cout << "  line " << line << ", in ";
        if (frame->closure->function->name == ""){
            std::cerr << "script\n" << std::endl;
        }else{
            std::cerr << "<" << frame->closure->function->name << ">" <<std::endl;
        }
    }
    std::cerr << "RuntimeError: " << format << std::endl;
}

InterpretResult VirtualMachine::run(){
    CallFrame* frame = &frames[frameCount - 1];
    auto read_byte = [&](){chunk_iter ret_ip = frame->ip; frame->ip += 1; return *ret_ip;};
    auto read_short = [&](){frame->ip += 2; return (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]);};
    for(;;){
        #ifdef DEBUG_TRACE_EXECUTION
            std::cout << std::endl;
            for(stack_iter slot = stack_memory->begin(); slot != stack_ptr; slot++){
                if (slot->type == VAL_BOOL){
                    std::cout << "[" << AS_BOOL(*slot) << "]" << std::endl;
                }else if (slot->type == VAL_NIL){
                    std::cout << "[Nil]" << std::endl;
                }else if (slot->type == VAL_NUMBER){
                    std::cout << "[" << AS_NUMBER(*slot) << "]" << std::endl;
                }else if (IS_STRING(*slot)){
                    std::cout << "String: " << "[" << AS_CSTRING(*slot) << "]" << std::endl;
                }else if (IS_CLOSURE(*slot)){
                    ObjClosure* closure = AS_CLOSURE(*slot);
                    std::string name = closure->function->name;
                    std::cout << "[closure] : " << name << std::endl;
                }else if (IS_INSTANCE(*slot)){
                    std::cout << "[instance] : " << AS_INSTANCE(*slot)->klass->name->strs << std::endl;
                }else if (IS_CLASS(*slot)){
                    std::cout << "[class] : " << AS_CLASS(*slot)->name->strs << std::endl;
                }else{
                    std::cout << "UNKONWN OBJECT" << std::endl;
                }
            }
            std::cout << get_op_code(*frame->ip) << " :" << frame->closure->function->name << std::endl;
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
                stack_push(frame->slots[slot-1]);
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
            case OP_INVOKE: {
                ObjString* method = AS_STRING(frame->closure->function->chunk->getValue(read_byte()));
                int argCount = read_byte();
                if(!invoke(method, argCount)){
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
            case OP_CLASS:{
                stack_push(OBJ_VAL(
                    new ObjClass(AS_STRING(frame->closure->function->chunk->getValue(read_byte()))))
                    );
                break;
            }
            case OP_GET_PROPERTY:{
                if(!IS_INSTANCE(peek(0))){
                    runtimeError("Only instance have properties.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjInstance* instance = AS_INSTANCE(peek(0));
                ObjString* name = AS_STRING(frame->closure->function->chunk->getValue(read_byte()));

                value_t val;
                if(!(instance->fields.find(name->strs) == instance->fields.end())){
                    stack_pop();
                    stack_push(instance->fields[name->strs]);
                    break;
                }

                if(!bindMethod(instance->klass, name)){
                    return INTERPRET_RUNTIME_ERROR;
                }

                break;
            }
            case OP_SET_PROPERTY:{
                if(!IS_INSTANCE(peek(1))){
                    runtimeError("Only instance have feilds.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjInstance* instance = AS_INSTANCE(peek(1));
                ObjString* field_name = AS_STRING(frame->closure->function->chunk->getValue(read_byte()));
                instance->fields[field_name->strs] = peek(0);
                value_t val = stack_pop();
                stack_push(val);
                break;
            }
            case OP_METHOD:
                defineMethod(AS_STRING(frame->closure->function->chunk->getValue(read_byte())));
                break;
            case OP_INHERIT:{
                value_t superclass = peek(1);
                if(!IS_CLASS(superclass)){
                    runtimeError("Superclass must be a class.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjClass* subclass = AS_CLASS(peek(0));
                subclass->methods = AS_CLASS(superclass)->methods;
                stack_pop();
                break;
            }
            case OP_GET_SUPER:{
                ObjString* name = AS_STRING(frame->closure->function->chunk->getValue(read_byte()));
                ObjClass* superclass = AS_CLASS(stack_pop());

                if(!bindMethod(superclass, name)){
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SUPER_INVOKE:{
                ObjString* method = AS_STRING(frame->closure->function->chunk->getValue(read_byte()));
                int argCount = read_byte();
                ObjClass* superclass = AS_CLASS(stack_pop());
                if(!invokeFromClass(superclass, method, argCount)){
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &frames[frameCount-1];
                break;
            }
        }
    }
}
