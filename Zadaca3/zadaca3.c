#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

int *Ulaz_u_odsjecak_zahtjev, *Broj_cekanja, *REZERVIRANO, brStolova, brDretvi;

/*Trazenje najveceg broja za red cekanja - klasicna for petlja za najveci broj*/
int max(){
	int velicina_reda = Broj_cekanja[0];
	for(int i = 0; i < brStolova; i++){
		if(Broj_cekanja[i] > velicina_reda){
			velicina_reda = Broj_cekanja[i];
		}
	}
	return velicina_reda;
}

/*Provjerava ima li jos ijedan slobodan stol*/
int sve_zauzeto(){
	for(int i = 0; i < brStolova; i++){
		if(REZERVIRANO[i] == -1){
			return 0;
		}
	}
	return 1;
}

/*Lamportov algoritam za ulaz u kriticni odsjecak*/
void udji_u_kriticni_odsjecak(int i){
	Ulaz_u_odsjecak_zahtjev[i] = 1;
	Broj_cekanja[i] = max() + 1;
	Ulaz_u_odsjecak_zahtjev[i] = 0;

	for(int j = 0;j < brStolova-1;j++){
		/*Ako nema zahtjeva ceka*/
		while(Ulaz_u_odsjecak_zahtjev[j]!=0 ){ 

		}
		/*Ako nije prvi u redu opet ceka*/
		while(Broj_cekanja[j] != 0 && (Broj_cekanja[j] < Broj_cekanja[i] || (Broj_cekanja[j] == Broj_cekanja[i] && j < i))){

		};
	}
}

/*Izlaz iz kriticnog odsjecka*/
void izadji_iz_kriticnog_odsjecka(int i){
	Broj_cekanja[i] = 0;
}

/*Provjerava je li stol slobodan*/
void *provjeri_stol(void *pdr){
	/*Ako je puno, gasi dretvu*/
	if(sve_zauzeto()){
	pthread_exit("\nSvi su stolovi zauzeti!!!\n");
	}

	int id_dretve = *((int *)pdr); //samo dohvacamo nas ID iz alocirane memorije za njih sve

	/*Dohvat random stola*/
	int random_stol = rand() % brStolova;
	printf("Dretva %d pokusava rezervirati stol %d\n", id_dretve + 1, random_stol + 1);
	udji_u_kriticni_odsjecak(random_stol);

	/*Ispis stanja stolova*/
	char stanje[brStolova]; //niz znakova za stanje

	for(int i = 0; i < brStolova; i++) {
		/*Ako je stol prazan stavi crtu*/
		if (REZERVIRANO[i] == -1) {
			stanje[i] = '-';
		} 
		/*Ako nije prazan onda stavi broj dretve koji ju je rezervirao (ali kao char pa ide + '0')*/
		else {
	        	stanje[i] = REZERVIRANO[i] + 1 + '0';
	    	}
	}
	stanje[brStolova] = '\0';

	/*Rezervacija ako je slobodan*/
	if (REZERVIRANO[random_stol] == -1) {
		REZERVIRANO[random_stol] = id_dretve;
		/*Update novog stanja stolova*/
		for (int i = 0; i < brStolova; i++) {
			/*Ako je slobodan napisi crtu*/
			if (REZERVIRANO[i] == -1) {
	        		stanje[i] = '-';
	        	} 
		        /*Ako nije prazan onda stavi broj dretve koji ju je rezervirao (ali kao char pa ide + '0')*/
		        else {
		        	stanje[i] = REZERVIRANO[i] + 1 + '0';
		        }
		}
		stanje[brStolova] = '\0';
		printf("Dretva %d je rezervirala stol %d, trenutno stanje: %s\n", id_dretve + 1, random_stol + 1, stanje);
	} 
	else {
	    	printf("Dretva %d nije uspjela rezervirati stol %d \n", id_dretve + 1, random_stol + 1);
	}

	izadji_iz_kriticnog_odsjecka(random_stol);

}

void main(){
	/*Unos podataka*/
	printf("Unesi broj dretvi: ");
	scanf("%d", &brDretvi);
	printf("Unesi broj stolova: ");
	scanf("%d", &brStolova);
	fflush(stdin);

	/*Zauzimanje memorije*/
	REZERVIRANO = (int*)malloc(sizeof(int) * brStolova);
        Broj_cekanja = (int*)malloc(sizeof(int) * brStolova);
        Ulaz_u_odsjecak_zahtjev = (int*)malloc(sizeof(int) * brStolova);

        /*Inicijalizacija*/
	for(int i = 0; i < brStolova; i++){
		REZERVIRANO[i] = -1; //slobodan stol
		Broj_cekanja[i] = 0; //ne ceka u redu
		Ulaz_u_odsjecak_zahtjev[i] = 0; //ne zeli pristupiti kriticnom odsjecku
	}

	/*Kreiranje dretvi*/
	pthread_t *ID_dretvi = malloc(brDretvi * sizeof(pthread_t)); //svi ID-evi dretvi

	while(!sve_zauzeto()){
	    for(int i = 0; i < brDretvi; i++){
	        int * pdr = (int * ) malloc (sizeof(int));
	        *pdr = i; //indeks dretve ide na za nju alociranu memoriju
	        pthread_create(&ID_dretvi[i], NULL, provjeri_stol, pdr); //kreiranje dretve koja pokrece "provjeri_stol" s ID-em "pdr"
	    }
	    usleep(1000000);
	}

	/*Cekanje da sve dretve zavrse*/
	for(int i = 0; i < brDretvi; i++){
	    pthread_join(ID_dretvi[i], NULL);
	}
	free(ID_dretvi);

	/*Oslobadjanje memorije*/
	free(REZERVIRANO);
	free(Broj_cekanja);
	free(Ulaz_u_odsjecak_zahtjev);
}
