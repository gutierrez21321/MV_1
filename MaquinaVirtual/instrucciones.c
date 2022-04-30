#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tipos.h"

void procesar(){
  int f,instruccion,mnemo,opA,a,opB,b;
  definirFunciones();
  registros[IP] = 0; //instruction pointer arranca en 0
  registros[CC] = 0x00000000;
  registros[AC] = 0x00000000;
  for(f=2;f<cantArg;f++){//si esta la flag [-c] la ejecuto
     if(strcmp(argGlobales[f],"[-c]") == 0)
      system("cls");
  }

  for(f=2;f<cantArg;f++){
    if(strcmp(argGlobales[f],"[-d]") == 0) //disassembler
      ejecutarDisassembler();
  }

  while(registros[IP]>=0 && registros[IP] < registros[DS]){
    instruccion = memoria[registros[IP]];  //leo instruccion
    leerInstruccion(instruccion,&mnemo,&opA,&a,&opB,&b); //decodifico instruccion y operandos
    instruccionActual = registros[IP]++; //aumento IP
    if(mnemo>=0x00000000 && mnemo<=0x0000000b)//2 operandos
        (*fun2[mnemo])(opA,a,opB,b);           //ejecutar instruccion 2 operandos
    else if(mnemo>=0x000000f0 && mnemo<=0x000000fb){
        mnemo = mnemo & 0x0000000f;
        (*fun1[mnemo])(opA,a); //ejecutar instruccion 1 operando
    }else if(mnemo==0x000000ff)     //1 operando
        (*fun0[0])(); //ejecutar instruccion 0 operandos (HARDCODEO)
    if(breakpointActivado ){
      if(((instruccion&0xff000000)>>24)!=0xf0) //si no es un sys
        breakpoint();
      else if(((instruccion&0xff000000)>>24)!=0xf0 && valor(opA,a)!=0xf) //si es sys pero no breakpoint
        breakpoint();
    } //si esta activado breakpoint dsp de cada operacion


  }

}

void leerArchivo(){ //segmento de codigo
  int instruccion,mnemo,opA,a,opB,b;
  int tamanoCodigo,i;
  char filename[MAXSTRING];
  strcpy(filename,argGlobales[1]);
  FILE *archivo;

  if((archivo = fopen(filename,"rb")) != NULL){ //existe el archivo
    for(i=0;i<6;i++){ //lee header
      if(fread(&instruccion,sizeof(int),1,archivo))
        if(i==1) tamanoCodigo = instruccion;
    }

    if(tamanoCodigo<MAXMEMORIA){ //capaz de almacenar las intrucciones
      i = 0;
      while(fread(&instruccion,sizeof(int),1,archivo)){ //lee archivo binario
        memoria[i++] = instruccion;
        leerInstruccion(instruccion,&mnemo,&opA,&a,&opB,&b);
        //printf("%08X %04X %d %d %d %d\n",instruccion,mnemo,opA,a,opB,b);
        //disassembler(i++,instruccion,mnemo,opA,a,opB,b); //disassembler de la instruccion
      }
      registros[DS] = i; //arranca segmento de datos
    }else
      registros[DS] = 0;

    fclose(archivo);
  }else //no pudo abrir el archivo
    registros[DS] = 0;
}

void leerInstruccion(int num,int *mnemo,int *opA,int *a,int *opB,int *b){
  *mnemo = num >> 28;          //leo el mnemonico
  *mnemo = *mnemo & 0x0000000f;
  if(*mnemo<=12){ //2 operandos
    *opA = num >> 26;
    *opA = *opA & 0x00000003;
    *a = (num>>12) & 0x00000fff;
    if(*opA==1){    //registro
      if((*a&0x0000000f)>=0x0 && (*a&0x0000000f)<=0x9){ //registros de 0 a 9
        *a = *a & 0x0000000f;
      }else if((*a&0x0000000f)>=0xA && (*a&0x0000000f)<=0xF){ //registros de EAX a EFX
        *a = *a & 0x0000003f;
      }
    }else
      *a = *a & 0x00000fff;

    *opB = (num>>24) & 0x00000003;
    *b = num & 0x00000fff;
    if(*opB==1){//registro
     if((*b&0x0000000f)>=0x0 && (*b&0x0000000f)<=0x9){ //registros de 0 a 9
        *b = *b & 0x0000000f;
      }else if((*b&0x0000000f)>=0xA && (*b&0x0000000f)<=0xF){ //registros de EAX a EFX
        *b = *b & 0x0000003f;
      }
    }else
      *b = *b & 0x00000fff;
    //propago signo si son inmediatos
    if(*opA==0){
     *a = *a<<20;
     *a = *a>>20;
    }
    if(*opB==0){
     *b = *b<<20;
     *b = *b>>20;
    }
    //printf("2 operandos\n");
    //printf("operando A: %d  operando B: %d\n",*opA,*opB);
    //printf("valor A: %d valor B: %d\n",*a,*b);
  }else if(*mnemo == 0x0000000f){ //1 operando o ninguno
     *mnemo = (*mnemo<<4) & 0x000000f0;
     *mnemo = *mnemo | (0x0000000f & (num >> 24));
     *mnemo = *mnemo & 0x000000ff;
     if((*mnemo & 0x0000000f) <= 12){ //1 operando
      *opA = (num >> 22) & 0x00000003;
      *opB = *b = 0b11;
      if(*opA==1){//registro
       if((*a&0x0000000f)>=0x0 && (*a&0x0000000f)<=0x9){ //registros de 0 a 9
        *a = *a & 0x0000000f;
       }else if((*a&0x0000000f)>=0xA && (*a&0x0000000f)<=0xF){ //registros de EAX a EFX
        *a = *a & 0x0000003f;
       }
      }else
        *a = num & 0x0000ffff;
      //propago signo si es inmediato
      if(*a==0){
        *a = *a<<16;
        *a = *a>>16;
      }
      //printf("1 operando\n");
      //printf("operando A: %d\n",*opA);
      //printf("valor A: %d\n",*a);
     }else if(*mnemo==0x000000ff){ //no operandos
      *opA = *a = *opB = *b = 0x00000003;
      //printf("0 operandos\n");
     }
  }
  //printf("mnemonico: %d\n",*mnemo);
}

void configurarRegistros(){
  for(int i=0xA;i<=0xF;i++)
    registros[i] = 0x00000000;
}
