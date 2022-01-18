#include "display.h"

//osnovno
char start = 0;
char mod = 1;
char razmakni = 0;

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

//lcd poruke
char *mod1 = "ON MOD1\r\n";
char* mod2 = "ON MOD2\r\m";
char* off = "OFF\r\n";
char* rez = "";
char* parametri = ".;";  		//za ispis parametara za mod1


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
char* izbrojanih_ispis = "0";
char poslati_komandu = 0;

//za vreme
unsigned int counterForSec = 0;
unsigned char sekundi = 0;
char* sekunde_ispis = "000";



char* brojUstring(unsigned char broj){
	 char reversed[10] = '\0';
	 char good[10] = '\0';
	 int i, j = 0;
	 int help = 0;
	 char neg = 0;

	 if(broj < 0){
	 	good[j] = '-';
		j++;
	 	broj = -broj;
	 }

	 if(broj == 0){
	 	reversed[i] = '0';
		i++;
	 }

	//zadat npr -123
	 while(broj != 0){	   
	 	help = broj%10;
		broj = broj/10;
		reversed[i] = help + 48;
		i++;
	 }

	 //sad je dobijen 321, mora da se obrne

	  //trenutno i pokazuje na mesto posle poslednjeg karakters
	  while(i != 0){
	  	 i--;		//sad pokazuje na 3
		 good[j] = reversed[i];
		 j++;
	  }
	  good[j] = '\0';



	return good;
}



void parsiraj_poruku(){

	if(buffer[0] == '(' && buffer[1] == 'S' && buffer[2] == 'T' && buffer[3] == 'A' && buffer[4] == 'R' && buffer[5] == 'T' && buffer[6] == ')'){
		start = 1;
		P2_0 = 0;			//signal gramofonu da krene da radi
		mod = 1;
		pok = serijski_start;
	}
	else if(buffer[0] == '(' && buffer[1] == 'S' && buffer[2] == 'T' && buffer[3] == 'O' && buffer[4] == 'P' && buffer[5] == ')'){
		start = 0;
		P2_0 = 1;			//signal gramofonu da stane
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
			sekundi = 0;	
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
			sekundi = 0;
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
				sekundi = 0;
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




char* num2str(char num){
	 char okrenut[10] = '\0';
	 char dobar[10] = '\0';
	 int i, j = 0;
	 int pomocni = 0;
	 char negativan = 0;

	 if(num < 0){
	 	dobar[j] = '-';
		j++;
	 	num = -num;
	 }

	 if(num == 0){
	 	okrenut[i] = '0';
		i++;
	 }

	//zadat npr -123
	 while(num != 0){	   
	 	pomocni = num%10;
		num = num/10;
		okrenut[i] = pomocni + 48;
		i++;
	 }

	 //sad je dobijen 321, mora da se obrne

	  //trenutno i pokazuje na mesto posle poslednjeg karakters
	  while(i != 0){
	  	 i--;		//sad pokazuje na 3
		 dobar[j] = okrenut[i];
		 j++;
	  }
	  dobar[j] = '\0';



	return dobar;
}



char* konkatenacija(char* prvi, char* drugi){
	char i = 0;
	char j = 0;
	char duzina = 0;
	
	for(i = 0; prvi[i] != '\0'; i++){
		rez[i] = prvi[i];
	}
	rez[i] = " ";
	i++;
	for(j = 0; drugi[j] != '\0'; j++){
		rez[i] = drugi[j];
		i++;
	}
	return rez;
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


void prekid_timer1(void) interrupt 3{	
	
	TH1 = 0xEE;																		  
	TL1 = 0;
	counterForSec++;
	if(counterForSec == 200){
		counterForSec = 0;
		sekundi++;
	}
}

void main(void){

	TH1 = 0xEE;
	TL1 = 0;
	TMOD = 0x10;
	ET1 = 1;
	TR1 = 1;
	PCON = 0x80;
	SCON = 0x50;
	BDRCON = 0x1C;	 					//za 19200
	BRL = 253;
	CKCON0 = CKCON0 & (~0x01);
	EA = 1;
	ES = 1;
	
	initDisplay();
	clearDisplay();

	while(1){

	trenutnoStanjeSoftP00_zaMain = P0_0;
	trenutnoStanjeSoftP01_zaMain = P0_1;

	
	//DONE za mod1: fali ispis drugog reda
	if(start == 1 && mod == 1){	 			//broji nitne i meri vreme
		clearDisplay();
		writeLine(mod1);
		newLine();
		
		if(prethodnoStanjeSoftP01_zaMain > trenutnoStanjeSoftP01_zaMain){ 				// obisao pun krug 
			sekunde_ispis = brojUstring(sekundi);
			razmakni = 1;
			//izbrojanih_ispis = num2str(izbrojanih_nitni);
			
			parametri = konkatenacija(izbrojanih_ispis, sekunde_ispis);
			writeLine(parametri);
			sekundi = 0;	   		//resetuje broj sekundi cim obidje pun krug
			izbrojanih_nitni = 0;
		}
		else{ 		//nije obisao pun krug	
			if(prethodnoStanjeSoftP00_zaMain > trenutnoStanjeSoftP00_zaMain){	 //detektovana je opadajuca ivica
				izbrojanih_nitni++;
				izbrojanih_ispis = num2str(izbrojanih_nitni);
			}		  

		}
		prethodnoStanjeSoftP00_zaMain = trenutnoStanjeSoftP00_zaMain;
		prethodnoStanjeSoftP01_zaMain = trenutnoStanjeSoftP01_zaMain;
	}
	//DONE: za mod2: pomeranje za pomeriti_nitni 
	//DONE: reset sekundi na promenu mod-a	   
	//DONE:ispis


	else if (start == 1 && mod == 2){		//implementacija za mod 2
		clearDisplay();
		writeLine(mod2);
		newLine();
		if(prethodnoStanjeSoftP00_zaMain > trenutnoStanjeSoftP00_zaMain){	//detektovana opadajuca ivica
			pomeriti_nitni--;
		}
		if(pomeriti_nitni == 0){
			P2_0 = 1; 				//da stane
			sekunde_ispis = num2str(sekundi);
			writeLine(sekunde_ispis);
			sekundi = 0;
		}
	
	
		prethodnoStanjeSoftP00_zaMain = trenutnoStanjeSoftP00_zaMain;
		prethodnoStanjeSoftP01_zaMain = trenutnoStanjeSoftP01_zaMain;	  
	}
	else{
		clearDisplay();									//stopiran je
		writeLine(off);
		TH1 = 0xEE;
		TL1 = 0;				//reset na stop
		sekundi = 0;
		

		prethodnoStanjeSoftP00_zaMain = trenutnoStanjeSoftP00_zaMain;
		prethodnoStanjeSoftP01_zaMain = trenutnoStanjeSoftP01_zaMain;
	}
	
	} 		//od while(1)
}		  //od main
 
