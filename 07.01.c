#include <at89c51rc2.h>
//osnovno
char start = 0;
char mod = 1;

//poruke
char *serijski_start = "Serijski startovan\r\n";
char *serijski_stop = "Serijski stopiran\r\n";
char *serijski_mod1 = "Ukljucen mod1\r\n";
char *serijski_mod2 = "Ukljucen mod2\r\n";
char *greska_serijskaPor = "Greska prilikom slanja komande\r\n";
char *greska_serijskaPorOverflow = "Greska prilikom slanja komande - Overflow\r\n";
char greska_serijska = 0;
char* pok;
char buffer[20];
char buffer_it = 0;

//nitne
char pomeriti_nitni = 0;
char izbrojanih_nitni = 0;

//za brojanje nitni
unsigned char trenutnoStanjeSoftP00 = 1;   	//on off
unsigned char trenutnoStanjeSoftP00_zaMain = 1;
unsigned char prethodnoStanjeSoftP00_zaMain = 1;

//za krug motora
unsigned char trenutnoStanjeSoftP01 = 1;   	//on off
unsigned char trenutnoStanjeSoftP01_zaMain = 1;
unsigned char prethodnoStanjeSoftP01_zaMain = 1;
char krug_motora =0;			//nije obisao pun krug, =1 jeste 
char izbrojanih_ispis = 0;
char poslati_komandu = 0;

void parsiraj_poruku(){

	if(buffer[0] == '(' && buffer[1] == 'S' && buffer[2] == 'T' && buffer[3] == 'A' && buffer[4] == 'R' && buffer[5] == 'T' && buffer[6] == ')'){
		start = 1;
		mod = 1;
		pok = serijski_start;
	}
	else if(buffer[0] == '(' && buffer[1] == 'S' && buffer[2] == 'T' && buffer[3] == 'O' && buffer[4] == 'P' && buffer[5] == ')'){
		start = 0;
		izbrojanih_nitni = 0;
		pomeriti_nitni = 0;
		pok = serijski_stop;
	}
	else if(buffer[0] == '(' && buffer[1] == 'M' && buffer[2] == '1' && buffer[3] == ')'){
		mod = 1;
		pok = serijski_mod1;
	}
	//ako ce broj nitni biti jednocifren
	else if(buffer[0] == '(' && buffer[1] == 'M' && buffer[2] == '2' && buffer[3] == ':' && buffer[5] == ')'){
		if(buffer[4] - 48 < 0 || buffer[4] - 48 > 9){
			greska_serijska = 1;
			pok = greska_serijskaPor;
		}												   
		else{
			mod = 2;
			pomeriti_nitni = buffer[4] - 48;
		 	pok = serijski_mod2;	
		}
		 
	}
	//dvocifren
	else if(buffer[0] == '(' && buffer[1] == 'M' && buffer[2] == '2' && buffer[3] == ':' && buffer[6] == ')'){
		if(buffer[4] - 48 < 0 || buffer[4] - 48 > 9 || buffer[5] - 48 < 0 || buffer[5] - 48 > 9){
			greska_serijska = 1;
			pok = greska_serijskaPor;
		}
		else{
			mod = 2;
			pomeriti_nitni = (buffer[4] - 48)*10 + (buffer[5] - 48) ;			//desetice*10 + jedinice
			pok = serijski_mod2;
		}
	}
	//trocifren
	else if (buffer[0] == '(' && buffer[1] == 'M' && buffer[2] == '2' && buffer[3] == ':' && buffer[7] == ')'){
		if(buffer[4] - 48 < 0 || buffer[4] - 48 > 9 || buffer[5] - 48 < 0 || buffer[5] - 48 > 9 || buffer[6] - 48 < 0 || buffer[6] - 48 > 9){
			greska_serijska = 1;
			pok = greska_serijskaPor;
		}
		else{
			//za slucaj da se unese broj >255
			if(((buffer[4] - 48)*100 + (buffer[5] - 48)*10 + (buffer[6] - 48)) <= 255){
				pomeriti_nitni = (buffer[4] - 48)*100 + (buffer[5] - 48)*10 + (buffer[6] - 48);
				pok = serijski_mod2;
				mod = 2;
			}
			else{
				greska_serijska = 1;
				pok = greska_serijskaPorOverflow;
			} 
		}	
	}
	
	else{
		pok = greska_serijskaPor;
		greska_serijska = 1;	
	}
	SBUF = *pok;
	buffer[0] = '\0';
	buffer_it = 0;
}


void serijska_interrupt(void) interrupt 4{
	if(TI){
		TI = 0;
		pok++;
		if(*pok != '\0'){
			SBUF = *pok;
		}
	}
	if(RI){
		char prijem;
		RI = 0;
		prijem = SBUF;
		if(prijem == '('){
			buffer_it = 0;
		}
		buffer[buffer_it] = prijem;
		if(prijem == ')'){
			parsiraj_poruku();
		}
		buffer_it = (buffer_it + 1) % 20;
	}
}


void prekid_timer0(void) interrupt 1
{	
	
	TH0 = 76;																		  
	TL0 = 1;
	

}

void main(void){

	TH0 = 0xEE;
	TL0 = 0;
	TMOD = 0x01;
	ET0 = 1;
	TR0 = 1;
	PCON = 0x80;
	SCON = 0x50;
	BDRCON = 0x1C;	 					//za 19200
	BRL = 253;
	EA = 1;
	ES = 1;
	
	while(1){

	trenutnoStanjeSoftP00_zaMain = P0_0;
	
	if(start == 1 && mod == 1){
		if(P0_1 == 1 && start == 1){ 				//nije obisao pun krug 
			if(prethodnoStanjeSoftP00_zaMain > trenutnoStanjeSoftP00_zaMain){	 //detektovana je opadajuca ivica
				izbrojanih_nitni++;
				izbrojanih_ispis = izbrojanih_nitni;
			}
		}
		else{ 		
			//izbrojanih_ispis = izbrojanih_nitni;
			izbrojanih_nitni = 0;		  //TODO: ispis se nije resetovao tako da ga treba iskoristiti za LCD

		}
	}
	else if (start == 1 && mod == 2){		//implementacija za mod 2
		//TODO mod2
		//poslati komandu koliko nitni treba
		poslati_komandu = pomeriti_nitni;
		  //continue;
	}
	else{									//stopiran je
		

		continue;
	}



	prethodnoStanjeSoftP00_zaMain = trenutnoStanjeSoftP00_zaMain;		//azuriranje vrednosti
	
	
	
	
	//prethodnoStanjeSoftP01_zaMain = trenutnoStanjeSoftP01_zaMain;		//azuriranje vrednosti
	


	
	} 		//od while(1)
}		  //od main
 
