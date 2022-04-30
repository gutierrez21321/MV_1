#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tipos.h"

//2 operandos

void setCC(int a){ //setea registro CC
  int reg;
  if(a == 0)
    reg = 0x80000001;
  else
    reg = 0x80000000;
  if(a < 0)
    reg = reg & 0x80000001;
  else
    reg = reg & 0x00000001;
  //printf("CC: %0X",reg);
  registros[CC] = reg;
}

void modificarRegistro(int a,int valor){ //testear
  int esp,aux,reg;
  esp = a & 0x0f;  //accedo registro
  if(esp>=0xA && esp<=0xF){
    //printf("registro: %d",esp);
  aux = (a>>4) & 0x3;  //sector de registro
  //printf("sector: %d",aux);
  //printf("valor: %d",valor);
  switch(aux){
      case 0: registros[esp] = registros[esp] & (~E);
              reg = valor & E;
              break;
      case 1: registros[esp] = registros[esp] & (~L);
              reg = valor & L;

              break;
      case 2: registros[esp] = registros[esp] & (~H);
              reg = (valor & L)<<8;

              break;
      case 3: registros[esp] = registros[esp] & (~X);
              reg = valor & X;

              break;
    }
  registros[esp] = registros[esp] | reg;
  //printf("registro final: %d",registros[esp]);
  }else if(esp>=0x0 && esp<=0x9){
    registros[esp] = valor;
  }
}

int valor(int tipo,int operador){ //devuelve el valor para operar
  int valor,aux;
  if(tipo==1 ){ //registro
    if((operador&0x00f)>=0xA && (operador&0x00f)<=0xF){
      aux = operador & 0xf; //accedo registro
      valor = registros[aux];
      //printf("cual registro: %d",valor);
      aux = (operador >> 4) & 0x3; //sector de registro
      //printf("sector registro: %d\n",aux);
      switch(aux){
      case 0: valor = valor & E;
              break;
      case 1: valor = valor & L;
              valor = valor<<24; //propago signo
              valor = valor>>24;
              break;
      case 2: valor = (valor & H)>>8;
              valor = valor<<24; //propago signo
              valor = valor>>24;
              break;
      case 3: valor = valor & X;
              valor = valor<<16; //propago signo
              valor = valor>>16;
              break;
    }

    }else if((operador&0x00f)>=0x0 && (operador&0x00f)<=0x9){
      valor = registros[operador];
    }

  }else if(tipo==2) //memoria
    valor = memoria[registros[DS] + operador];
  else //inmediato
    valor = operador;
  //printf("valor B: %d",valor);
  return valor;
}

void mov(int opA,int a,int opB,int b){
  int valorB;
  if(opA!=3 && opA!=0 && opB!=3){//si es valido
   valorB = valor(opB,b);
   if(opA==1) //registro
     modificarRegistro(a,valorB);
   else{      //memoria
      //printf("valorB: %d",valorB);
      memoria[registros[DS]+a] = valorB;
    }
  }
}

void add(int opA,int a,int opB,int b){
  int suma,valorA,valorB;
  if(opA!=3 && opA!=0 && opB!=3){//si es valido
    //printf("valor a: %d valor b: %d",valor(opA,a),valor(opB,b));
    valorA = valor(opA,a);
    valorB = valor(opB,b);
    suma = valorA + valorB;
    //printf("a: %d b: %d",valor(opA,a),valor(opB,b));
    //printf("suma: %d",suma);
    if(opA==1)  //registro
      modificarRegistro(a,suma);
    else //memoria
      memoria[registros[DS]+a] = suma;
    setCC(suma);
  }
}

void sub(int opA,int a,int opB,int b){
  int resta;
  if(opA!=3 && opA!=0 && opB!=3){//si es valido
    resta = valor(opA,a) - valor(opB,b);
    if(opA==1)  //registro
      modificarRegistro(a,resta);
    else //memoria
      memoria[registros[DS]+a] = resta;
    setCC(resta);
  }
}

void mul(int opA,int a,int opB,int b){
  int multiplicacion;
  if(opA!=3 && opA!=0 && opB!=3){//si es valido
    multiplicacion = valor(opA,a) * valor(opB,b);
    if(opA==1)  //registro
      modificarRegistro(a,multiplicacion);
    else //memoria
      memoria[registros[DS]+a] = multiplicacion;
    setCC(multiplicacion);
  }
}

void division(int opA,int a,int opB,int b){
  if(opA!=3 && opA!=0 && opB!=3){//si es valido
    div_t resultado = div(valor(opA,a),valor(opB,b));
    if(opA==1) //registro
      modificarRegistro(a,resultado.quot); //resultado.quot cociente entero
    else      //memoria
      memoria[registros[DS]+a] = resultado.quot;
    modificarRegistro(AC,abs(resultado.rem));  //resto entero positivo
    setCC(resultado.quot);
  }
}

void swap(int opA,int a,int opB,int b){
  int valorA,valorB;
  if(opA!=3 && opA!=0 && opB!=3 && opB!=0){//si es valido
    valorA = valor(opA,a);
    valorB = valor(opB,b);
    //printf("SWAP valorA: %0X valorB: %0X",valorA,valorB);

    if(opA==1)   //registro
      modificarRegistro(a,valorB);
    else  //memoria
      memoria[registros[DS]+a] = valorB;

    if(opB==1){   //registro
      modificarRegistro(b,valorA);
    }else  //memoria
      memoria[registros[DS]+b] = valorA;
  }
}

void cmp(int opA,int a,int opB,int b){ //supongo que a puede ser inmediato
  int aux;
  if(opA!=3 && opB!=3)
    aux = valor(opA,a) - valor(opB,b);
  //printf("CMPPPPPPPP %d",aux);
  setCC(aux);
}

void and(int opA,int a,int opB,int b){
  int andBit;
  if(opA!=3 && opA!=0 && opB!=3){//si es valido
    andBit = valor(opA,a) & valor(opB,b);
    if(opA==1) //registro
      modificarRegistro(a,andBit);
    else       //memoria
      memoria[registros[DS]+a] = andBit;
  }
  setCC(andBit);
}

void or(int opA,int a,int opB,int b){
  int orBit;
  if(opA!=3 && opA!=0 && opB!=3){//si es valido
    orBit = valor(opA,a) | valor(opB,b);
    if(opA==1) //registro
      modificarRegistro(a,orBit);
    else       //memoria
      memoria[registros[DS]+a] = orBit;
  }
  setCC(orBit);
}

void xor(int opA,int a,int opB,int b){
  int xorBit;
  if(opA!=3 && opA!=0 && opB!=3){//si es valido
    xorBit = valor(opA,a) ^ valor(opB,b);
    if(opA==1) //registro
      modificarRegistro(a,xorBit);
    else       //memoria
      memoria[registros[DS]+a] = xorBit;
  }
  setCC(xorBit);
}

void shl(int opA,int a,int opB,int b){
  int corrimiento,valorA,valorB;
  if(opA!=3 && opA!=0 && opB!=3){//si es valido
    valorA = valor(opA,a);
    valorB = valor(opB,b);
    corrimiento =  valorA << valorB;
    //printf("shl operando a: %d",valor(opA,a));
    if(opA==1)  //registro
      modificarRegistro(a,corrimiento);
    else        //memoria
      memoria[registros[DS]+a] = corrimiento;
  }
  setCC(corrimiento);
}

void shr(int opA,int a,int opB,int b){
  int corrimiento;
  if(opA!=3 && opA!=0 && opB!=3){//si es valido
    corrimiento = valor(opA,a) >> valor(opB,b);
    if(opA==1) //registro
      modificarRegistro(a,corrimiento);
    else    //memoria
      memoria[registros[DS]+a] = corrimiento;
  }
  setCC(corrimiento);
}

//1 operando

void read(){
  char car,lectura[MAXSTRING];
  int i,ingreso,conf,base;
  int alm = (registros[0xD] & E); //almaceno desde posicion en memoria que esta en registro EDX
  conf = registros[0xA] & X;
  if(((conf>>8) & 0x00000001 ) == 0){ //interpreta dsp del enter
    for(i=0;i<(registros[0xC]&X);i++){
      if(((conf>>11) & 0x00000001) == 0) //escribe el prompt
        printf("[%04d]: ",alm); //alm = posicion de memoria en decimal
      scanf("%d",&ingreso); //leo
      printf("\n");
      if(((conf>>3) & 0x0001) == 1) //hexa
        base = 16;
      else if(((conf>>2) & 0x0001) == 1) //octal
        base = 8;
      else if((conf & 0x0001 ) == 1) //decimal
        base = 10;
      itoa(ingreso,lectura,base); //de decimal a string en base x
      ingreso = atoi(lectura); //de string a numero
      memoria[registros[DS] + alm++] = ingreso;
    }
  }else{ //lee caracter a caracter dsp del enter
    i = 0;
    if(((conf>>11) & 0x0001) == 0) //escribe el prompt
      printf("[%04d]: ",alm); //alm a binario
    scanf("%s",lectura);
    car = lectura[i]; //caracter
    while(car!=' ' && i<(registros[0xC]&X)){
      memoria[registros[DS] + alm++] = car;
      car = lectura[++i];
    }
    memoria[registros[DS] + alm] = 0x00000000;
  }
}

void write(){
  char porcentaje;
  int escribe,prompt,endline,i,caracter,conf;
  int alm = registros[0xD] & E; //leo desde posicion en memoria que esta en registro EDX
  porcentaje = '%';
  conf = registros[0xA] & X;
  endline = !((conf>>8) &0x00000001); //0 agregar endline
  prompt = !((conf>>11) & 0x00000001);//agregar prompt
  for(i=0;i<(registros[0xC]&X);i++){
   if(prompt) printf("[%04d]: ",alm);
   escribe = memoria[registros[DS] + alm];
   if(((conf>>4) & 0x00000001) == 1){ //caracter
    caracter = escribe & 0x000000ff; //byte menos significativo
    if(caracter<32 || caracter>126)
      caracter = '.';               //ascii no imprimible
    printf("%c ",caracter);
   }
   if(((conf>>3) & 0x00000001) == 1){ //hexadecimal
    printf("%c%0X ",porcentaje,escribe);
   }
   if(((conf>>2) & 0x00000001 )== 1){ //octal
    printf("@%o ",escribe);
   }
   if((conf & 0x00000001) == 1){//decimal
    printf("#%d ",escribe);
   }
   if(endline) printf("\n");
   alm++;
  }
}

void breakpoint(){
  int h,f;
  int operando1,operando2;
  char opA[MAXSTRING],opB[MAXSTRING],input[MAXSTRING];
  FILE* archivo;
  for(f=2;f<cantArg;f++){     //si esta la flag [-c] la ejecuto
    if(strcmp(argGlobales[f],"[-c]") == 0)
      system("cls");
  }
  for(f=2;f<cantArg;f++){
    if(strcmp(argGlobales[f],"[-d]") == 0){ //disassembler
      ejecutarDisassembler();
    }
  }
  for(f=2;f<cantArg;f++){
    if(strcmp(argGlobales[f],"[-b]") == 0){
      archivo = fopen("input.txt","wt"); //abre para escritura
      fflush(stdin);
      printf("[%04d] cmd: ",instruccionActual);
      fgets(input,100,stdin);  //entrada desde teclado
      fputs(input,archivo);    //escribe en el archivo la entrada
      fclose(archivo);
      archivo = fopen("input.txt","r"); //abre para lectura

      if(fscanf(archivo,"%s %s", opA,opB)==2){ //si son 2 operandos
        operando1 = atoi(opA);
        operando2 = atoi(opB);
        //printf("a %d b %d",operando1,operando2);
        if(!((opA[0]!='0' && operando1 == 0) || (opB[0]!='0' && operando2 == 0))){ // son numeros
          //printf("operando1: %d operando2: %d\n",operando1,operando2);
          breakpointActivado = 0;
          for(h=operando1;h<=operando2;h++)
            printf("[%04d]: %04X %04X %d\n",h,(memoria[h]&0xffff0000)>>16,memoria[h]&0x0000ffff,memoria[h]);
        }
      }else{      //1 operando
        //printf("operandoA: %s\n",opA);
        if(strcmp(opA,"r")==0){
          breakpointActivado = 0; //desactiva breakpoint la MV continua la ejecucion
          printf("\n");
        }else if(strcmp(opA,"p")==0){
          printf("\n");
          breakpointActivado = 1; //activa breakpoint dsp de cada instruccion
        }else{
          operando1 = atoi(opA);
          if(!(opA[0]!='0' && operando1==0)){ //si es numero
            //printf("NUMERO");
            breakpointActivado= 0;
            if(operando1>=0 && operando1<=4096)
              printf("[%04d]: %04X %04X %d\n",operando1,((memoria[operando1])&0xffff0000)>>16,
                     (memoria[operando1])&0x0000ffff,memoria[operando1]);
          }
        }
      }
    }
    //system("pause");
  }
}

void sys(int opA,int a){
  if(a==0x00000001)//lectura
    read();
  else if(a==0x00000002) //escritura
    write();
  else if(a==0x0000000f){// breakpoint
     breakpoint();
  }
}

void jmp(int opA,int a){
  if(opA!=3)//si es valido
    registros[IP] = valor(opA,a);
}

void jp(int opA,int a){
  if(opA!=3)//si es valido
    if(registros[CC] == 0x00000000)
      registros[IP] = valor(opA,a);
}

void jumpn(int opA,int a){
  if(opA!=3)//si es valido
    if(registros[CC] == 0x80000000)
      registros[IP] = valor(opA,a);
}

void jz(int opA,int a){
  if(opA!=3)//si es valido
    if(registros[CC] == 0x00000001)
      registros[IP] = valor(opA,a);
}

void jnz(int opA,int a){ //jump not zero
  if(opA!=3)//si es valido
    if((registros[CC] & 0x00000001) == 0x0)
      registros[IP] = valor(opA,a);
}

void jnp(int opA,int a){ //jump no positive
   //printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
  if(opA!=3){
    if(registros[CC] == 0x80000000 || registros[CC] == 0x00000001) //negativo o 0
      registros[IP] = valor(opA,a);
  }
}

void jnn(int opA,int a){ //jump no negative
  if(opA!=3){
    if(registros[CC] == 0x00000000 || registros[CC] == 0x00000001) //positivo o 0
      registros[IP] = valor(opA,a);
  }
}

void ldh(int opA,int a){
  int operando,aux;
  operando = valor(opA,a) & 0x0000ffff; //los 2 bytes menos significativos del operando
  aux = registros[AC] & 0x0000ffff; //hace hueco entre los 2 bytes mas significativo de AC
  registros[AC] = (operando << 16) | aux;
}

void ldl(int opA,int a){
  int operando,aux;
  operando = valor(opA,a) & 0x0000ffff; //los 2 bytes menos significativos del operando
  aux = registros[AC] & 0xffff0000;     //los 2 bytes mas significativos de AC
  registros[AC] = operando | aux;
}

void rnd(int opA,int a){
  if(opA!=3 && opA!=0){
    int random = rand() % valor(opA,a); //de 0 hasta a chequear negativo
    registros[AC] = random;
  }
}

void not(int opA,int a){
  int negacion;
  if(opA!=3 && opA!=0){
    negacion = ~valor(opA,a);
    if(opA==1) //registros
      modificarRegistro(a,negacion);
    else //memoria
      memoria[registros[DS]+a] = negacion;
  }
  setCC(negacion);
}

//0 operandos

void stop(){ //detiene programa
  registros[IP] = registros[DS];
}

