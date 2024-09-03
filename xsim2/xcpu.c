#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xis.h"
#include "xcpu.h"
#include "xmem.h"

void shortSplitter (unsigned short value, unsigned char data[2])
{
    data[1] = value;
    unsigned short byteDest = value;
    byteDest >>= 8;
    data[0] = byteDest;
}

void byteSplitter (unsigned char nibble[2], unsigned char byte)
{
    unsigned char nibbleSource = byte;
    unsigned char nibbleDest = byte;
    nibble[0] = nibbleSource >> 4;
    nibbleDest <<= 4; nibble[1] = nibbleDest >> 4;
}

unsigned short immediate_value (xcpu *c, unsigned char data[2])
{
    xmem_load(c->pc, data);     //load data
    unsigned short tmp = data[0];
    tmp <<= 8;                      //bitshift short to fit two bytes
    tmp |= data[1];
    return tmp;
}

extern int xcpu_execute( xcpu *c ) {
    //byte instruction, byte operand

    unsigned char data[2];          //data from memory
    xmem_load(c->pc, data);     //load data
    c->pc += 2;                     //increment program counter
    unsigned char nibble[2];        //store split byte here for register calls

    byteSplitter(nibble, data[1]);

    switch (data[0]) {
        //0-operand instructions
        case I_RET:
            xmem_load(c->regs[15], data); //load the data at mem[r15]
            c->regs[15] += 2;   //add 2 to go back to previous location
            unsigned short tmp = data[0];
            tmp <<= 8;
            tmp |= data[1];
            c->pc = tmp;    //set the program counter back to original position before jump
            break;
        case I_CLD: //Clear debug bit
            c->state &= ~X_STATE_DEBUG_ON;
            break;
        case I_STD: //Set debug bit
            c->state |= X_STATE_DEBUG_ON;
            break;
        //1-operand instruction
        case I_NEG: //negate register
            c->regs[nibble[0]] *= -1;
            break;
        case I_NOT: //logical negate
            c->regs[nibble[0]] = !c->regs[nibble[0]];
            break;
        case I_INC: //increment reg
            c->regs[nibble[0]] += 1;
            break;
        case I_DEC: //decrement reg
            c->regs[nibble[0]] -= 1;
            break;
        case I_PUSH:
            c->regs[15] -= 2;
            shortSplitter(c->regs[nibble[0]], data);
            xmem_store(data, c->regs[15]);
            break;
        case I_POP:
            xmem_load(c->regs[15], data);
            c->regs[15] += 2;
            tmp = data[0];
            tmp <<= 8;
            tmp |= data[1];
            c->regs[nibble[0]] = tmp;
            break;
        case I_JMPR:
            c->pc = c->regs[nibble[0]];
            break;
        case I_CALLR:
            c->regs[15] -= 2;
            shortSplitter(c->pc, data);
            xmem_store(data, c->regs[15]);
            c->pc = c->regs[nibble[0]];
            break;
        case I_OUT:
            printf("%c", c->regs[nibble[0]]);
            break;
        case I_BR:
            if ((c->state & X_STATE_COND_FLAG) == 1)
            {
                c->pc -= 2;
                c->pc += data[1];
            }
            break;
        case I_JR:
            c->pc += data[1]; //this is the label, need to move pointer back
            break;
        //2-operand instruction
        case I_ADD:
            c->regs[nibble[1]] += c->regs[nibble[0]];
            break;
        case I_SUB:
            c->regs[nibble[1]] -= c->regs[nibble[0]];
            break;
        case I_MUL:
            c->regs[nibble[1]] *= c->regs[nibble[0]];
            break;
        case I_DIV:
            c->regs[nibble[1]] /= c->regs[nibble[0]];
            break;
        case I_AND:
            c->regs[nibble[1]] &= c->regs[nibble[0]];
            break;
        case I_OR:
            c->regs[nibble[1]] |= c->regs[nibble[0]];
            break;
        case I_XOR:
            c->regs[nibble[1]] ^= c->regs[nibble[0]];
            break;
        case I_SHR:
            c->regs[nibble[1]] >>= c->regs[nibble[0]];
            break;
        case I_SHL:
            c->regs[nibble[1]] <<= c->regs[nibble[0]];
            break;
        case I_TEST:
            if ((c->regs[nibble[0]] & c->regs[nibble[1]]) != 0)
                c->state |= X_STATE_COND_FLAG;
            else
                c->state &= ~X_STATE_COND_FLAG;
            break;
        case I_CMP:
            if (c->regs[nibble[0]] < c->regs[nibble[1]])
                c->state |= X_STATE_COND_FLAG;
            else
                c->state &= ~X_STATE_COND_FLAG;
            break;
        case I_EQU:
            if (c->regs[nibble[0]] == c->regs[nibble[1]])
                c->state |= X_STATE_COND_FLAG;
            else
                c->state &= ~X_STATE_COND_FLAG;
            break;
        case I_MOV:
            c->regs[nibble[1]] = c->regs[nibble[0]];
            break;
        case I_LOAD:
            xmem_load(c->regs[nibble[0]], data);
            c->regs[nibble[1]] = data[0];
            c->regs[nibble[1]] <<= 8;
            c->regs[nibble[1]] |= data[1];
            break;
        case I_STOR:
            shortSplitter(c->regs[nibble[0]], data);
            xmem_store(data, c->regs[nibble[1]]);
            break;
        case I_LOADB:
            xmem_load(c->regs[nibble[0]], data);
            c->regs[nibble[1]] = data[0];
            break;
        case I_STORB:
            xmem_load(c->regs[nibble[1]], data);
            data[0] = c->regs[nibble[0]];
            xmem_store(data, c->regs[nibble[1]]);
            break;
        //3-extended instruction
        case I_JMP:
            c->pc = immediate_value(c, data);
            break;
        case I_CALL:
            c->regs[15] -= 2;
            shortSplitter(c->pc+2, data);
            xmem_store(data, c->regs[15]);    //store next program counter mem[r15]
            c->pc = immediate_value(c, data);    //jump to address
            break;
        case I_LOADI:
            c->regs[nibble[0]] = immediate_value(c, data);
            c->pc += 2; //next word operand
            break;
        default:
            return 0;
    }
    if (c->state & X_STATE_DEBUG_ON) //print current cpu state
        xcpu_print(c);
    return 1;
}