#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tipos.h"

void ejecutarDisassembler(){
  int instruccion,mnemo,opA,a,opB,b,i;
  numIntruccion = 0;
  i = 0;
  while(i < registros[DS]){
    instruccion = memoria[i];
    leerInstruccion(instruccion,&mnemo,&opA,&a,&opB,&b);
    //printf("%08X %04X %d %d %d %d %d",instruccion,mnemo,opA,a,opB,b);
    disassembler(i++,instruccion,mnemo,opA,a,opB,b); //disassembler de la instruccion
  }
  mostrarRegistros();
}

void disassembler(int ip,int ins,int mnemo,int opA,int a,int opB,int b){
  char coma,simbolo,numString[MAXSTRING],instruccion[MAXSTRING],operandoA[MAXSTRING],operandoB[MAXSTRING];
  char* mnemo2[] = {"MOV","ADD","SUB","SWAP","MUL","DIV","CMP","SHL","SHR","AND","OR","XOR"};
  char* mnemo1[] = {"SYS","JMP","JZ","JP","JN","JNZ","JNP","JNN","LDL","LDH","RND","NOT"};
  char* mnemo0[] = {"STOP"};
  char* regChar[] = {"DS","","","","","IP","","","CC","AC","A","B","C","D","E","F"};
  char* sectorChar[] = {"X","L","H","X"};

  strcpy(operandoA,""); strcpy(operandoB,"");
    if(mnemo>=0x00000000 && mnemo<=0x0000000b)//2 operandos
        strcpy(instruccion,mnemo2[mnemo&0x0000000f]);
    else if(mnemo>=0x000000f0 && mnemo<=0x000000fb){ //1 operando
        mnemo = mnemo & 0x0000000f;
        strcpy(instruccion,mnemo1[mnemo]);
        opB=3;
    }else if(mnemo==0x000000ff)     //0 operandos
        strcpy(instruccion,mnemo0[0]); //hardcodeo

    if(opA!=3){//si es valido
      if(opA==1){      //registro
        if((a&0x00F)>=0xA && (a&0x00F)<=0xF){
          if(((a>>4)&0x00000003) == 00) //00 EXTENDED
            strcpy(operandoA,"E");
          else
            strcpy(operandoA,"");
        }
      strcat(operandoA,regChar[(a&0x0000000f)]); //cual registro
      if(opA==1 && (a&0x00F)>=0xA && (a&0x00F)<=0xF)
        strcat(operandoA,sectorChar[((a>>4)&0x00000003)]); //sector registro
      //printf("%s\n",operandoA);
    }else if(opA==2){      //memoria
      strcpy(operandoA,"[");
      itoa(a,numString,10); //de numero a string del numero
      strcat(operandoA,numString);
      strcat(operandoA,"]");
      //printf("%s",operandoA);
    }else if(opA==0){ //inmediato
      itoa(a,numString,10);
      strcpy(operandoA,numString);
    }
  }
  if(opB!=3){//si es valido
   if(opB==1){      //registro
      if((b&0x00F)>=0xA && (b&0x00F)<=0xF){
        if(((b>>4)&0x00000003) == 00) //00 EXTENDED
          strcpy(operandoB,"E");
        else
          strcpy(operandoB,"");
      }
      strcat(operandoB,regChar[b&0x0000000f]); //cual registro
      if(opB==1 && (b&0x00F)>=0xA && (b&0x00F)<=0xF)
        strcat(operandoB,sectorChar[((b>>4)&0x00000003)]); //sector registro
      //printf("%s\n",operandoB);
   }else if(opB==2){      //memoria
      strcpy(operandoB,"[");
      itoa(b,numString,10); //de numero a string del numero
      strcat(operandoB,numString);
      strcat(operandoB,"]");
      //printf("%s",operandoB);
   }else if(opB==0){ //inmediato
      itoa(b,numString,10);
      strcpy(operandoB,numString);
   }
  }
  if(opB!=3) coma= ','; else coma=' ';
  if(ip==registros[IP]) simbolo = '>'; else simbolo=' ';
  printf("%c[%04d]: %02X %02X %02X %02X      %d: %s         %s%c %s",simbolo,ip,(ins>>24)&0x000000ff,
         (ins>>16)&0x000000ff,(ins>>8)&0x000000ff,ins&0x000000ff,++numIntruccion,instruccion,operandoA,coma,operandoB);
  printf("\n");
}


void mostrarRegistros(){
  char por = '%';
  printf("\nRegistros:\n");
  printf("DS = %d\n",registros[DS]);
  printf("IP = %d\n",registros[IP]);
  printf("CC = %c%0X\n",por,registros[CC]);
  printf("AC = %c%0X\n",por,registros[AC]);
  printf("EAX = %c%0X\n",por,registros[0xA]);
  printf("EBX = %c%0X\n",por,registros[0xB]);
  printf("ECX = %c%0X\n",por,registros[0xC]);
  printf("EDX = %c%0X\n",por,registros[0xD]);
  printf("EEX = %c%0X\n",por,registros[0xE]);
  printf("EFX = %c%0X\n",por,registros[0xF]);
}

int stringToInt(char linea[],int ind){
  int i,k,cant,num;
  k = cant = num = 0;
  //printf("%s\n",linea);
  for(i=ind;i>=0;i--){
   k = linea[i] - '0';
   //printf("%d\n",k);
   k = k << cant++;
   num = num | k;
  }
  //printf("%d\n",num);
  return num;
}

void definirFunciones(){
  breakpointActivado = 0; //breakpoint desactivado por defecto
  fun0[0] = stop;
  //
  fun1[0] = sys;
  fun1[1] = jmp;
  fun1[2] = jz;
  fun1[3] = jp;
  fun1[4] = jumpn;
  fun1[5] = jnz;
  fun1[6] = jnp;
  fun1[7] = jnn;
  fun1[8] = ldl;
  fun1[9] = ldh;
  fun1[10] = rnd;
  fun1[11] = not;
  //
  fun2[0] = mov;
  fun2[1] = add;
  fun2[2] = sub;
  fun2[3] = swap;
  fun2[4] = mul;
  fun2[5] = division;
  fun2[6] = cmp;
  fun2[7] = shl;
  fun2[8] = shr;
  fun2[9] = and;
  fun2[10] = or;
  fun2[11] = xor;

}


