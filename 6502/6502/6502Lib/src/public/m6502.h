#pragma once
#include <stdio.h>
#include <stdlib.h>

using Byte = unsigned char;
using Word = unsigned short;
using u32 = unsigned int;

struct Mem 
{
    static constexpr u32 MAX_MEM = 1024 * 64;
    Byte Data[MAX_MEM];

    void Initialise()
    {
        for (u32 i = 0; i < MAX_MEM; i++)
        {
            Data[i] = 0;
        }
    }

    //read 1 Byte
    Byte operator[](u32 Address) const 
    {
        // assert Address is < MAX_MEM
        return Data[Address];

    }

    //write 1 Byte
    Byte& operator[](u32 Address)  
    {
        // assert Address is < MAX_MEM
        return Data[Address];

    }

    void WriteWord(
        Word Value, 
        u32 Addr, 
        u32& Cycles)
    {
        Data[Addr]      = Value & 0xFF;
        Data[Addr + 1]  = (Value >> 8);
        Cycles -= 2;
    }
};

struct CPU
{
    Word PC;        //program counter
    Byte SP;        //stack pointer

    Byte A, X, Y;   //registers

    Byte C : 1;     //status flags
    Byte Z : 1;
    Byte I : 1;
    Byte D : 1;
    Byte B : 1;
    Byte V : 1;
    Byte N : 1;
    
    void Reset(Mem& memory)
    {
        PC = 0xFFFC;
        SP = 0xFF;
        C = Z = I = D = B = V = N = 0;
        A = X = Y = 0;

        memory.Initialise();
    }

    Byte FetchByte(u32& Cycles, Mem& memory)
    {
        Byte Data = memory[PC];
        PC++;
        Cycles--;
        return Data;
    }

    Word FetchWord (u32& Cycles, Mem& memory)
    {
        // 6502 is little endian 
        Word Data = memory[PC];
        PC++;
        Data |= (memory[PC] << 8);
        PC++;
        
        Cycles -= 2;
        // to handle endianness, swap bytes here 
        // if (is_PLATOFRM_ENDIAN_BIG) 
        // swapBytesInWord(Data)
        return Data;
    }

    Byte ReadByte(
        u32& Cycles,
        Byte Addr, 
        Mem& memory)
    {
        Byte Data = memory[Addr];
        Cycles--;
        return Data;
    }

    static constexpr Byte 
        INS_LDA_IM = 0xA9, 
        INS_LDA_ZP = 0xA5,
        INS_LDA_ZPX = 0xB5,
        INS_JSR = 0x20;

    void LDASetStatus() 
    {
        Z = (A == 0);
        N = (A & 0b10000000) > 0;
    }

    void Execute(u32 Cycles, Mem& memory)
    {
        while (Cycles > 0)
        {
            Byte Ins = FetchByte(Cycles, memory);
            switch (Ins)
            {
                case INS_LDA_IM:
                {
                    Byte Value = FetchByte(Cycles,memory);
                    A = Value;
                    LDASetStatus();
                } break;

                case INS_LDA_ZP:
                {
                    Byte ZeroPageAddr = FetchByte(Cycles, memory);
                    A = ReadByte(Cycles, ZeroPageAddr, memory);
                    LDASetStatus();
                } break;

                case INS_LDA_ZPX:
                {
                    Byte ZeroPageAddr = FetchByte(Cycles, memory);
                    ZeroPageAddr += X;
                    Cycles--;
                    A = ReadByte(Cycles, ZeroPageAddr, memory);
                    LDASetStatus();
                } break;
                
                case INS_JSR:
                {
                    Word SubAddr = FetchWord(Cycles, memory);
                    memory.WriteWord(PC - 1, SP, Cycles);
                    PC = SubAddr;
                    SP--;
                    Cycles--;
                } break;
                
                
                
                default:
                {
                    printf("Instruction not handled %d", Ins);
                } break;
                    
            }   
            
        }
    }
};
