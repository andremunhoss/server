#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <semaphore.h>
#include <ctype.h>
#include <time.h>

# define PROTOPORT 9550
# define QLEN 6

int visits = 0;
char msg [1000][1000];
int ditados =0;
int readcount =0 , wreadcount =0;
int alteracoes =0;
int id = 0;
sem_t read ;
sem_t wread ;
sem_t m ;
sem_t m0;

void LeDitado ()
{
	FILE *arq ;

	if (( arq = fopen ("Ditados.txt" ,"r" )) == NULL ) { printf ( "\n Erro lendo arquivo ...\n \n " ); exit (0);}
	
	while (!feof(arq)) {
		fgets (msg[ ditados ] ,999 , arq ); 
		printf ( "%d%s" , ditados , msg[ditados]);
		ditados =( ditados +1)%1000;

	}
}

char uppercase(char *input) {
    char i=0;
    for(i = 0; (i<strlen(input)) && (i<1000); i++) {
        input[i] = toupper(input[i]);
    }
    return i;
}


// Funcao para contar numero de palavras em um string
int words(const char sentence [ ])
{
	int contado = 0; // resultado
	const char *it = sentence ;
	int inword = 0;

	do switch (*it) {
		case '\0':
		case ' ': case '\t': case '\n': case '\r': // Procura por esses caracteres
		if (inword) {inword = 0; contado ++;} // conta
			break ;
		default: inword = 1;
	} while (*it++);
	
	return contado ; // retorna resultado
}

void leitor ( char *str , int sd ){
	//int sd = *(int *)sd2; 
	
	char *endptr , *local ;
	char cpy [1000]; // usado para armazenar ditado
	char cpy1 [1000]; // usado para armazenar ditado
	char haystack [1000]; // usado no search
	char needle [1000]; // usado no search
	int i =0 , j , b , val , val1 , n , numerototal = 0;

	sem_wait(&read);
	readcount ++;
	if ( readcount ==1)
	{
		sem_wait(&wread);
	}
	if (!strncmp (str,"GETR",4)) {
		sem_wait(&m); 
		sprintf (str , "\n Ditado %d : %s " , visits%ditados , msg[visits%ditados]);
		send(sd,str , strlen (str) ,0);
		sem_post(&m); 
	}
	else
	if (!strncmp ( str , "GETN" ,4)) {
		sem_wait (&m);
		b = recv ( sd , str ,999 ,0);
		str [b]=0;
		val = strtol( str , &endptr , 10);
		if ( endptr == str ) {
			sprintf ( str , " \nFALHA " );
			// continue ;
			}
		else
		{ send(sd, msg[val], strlen(msg[val]), 0);
		}
			sem_post (&m);

	} else
	if (!strncmp ( str , "SEARCH" ,6)) { 
		sem_wait(&m);
		j = 0;
		b = recv(sd, str, 999, 0);
		str[b] = 0;
		uppercase(str);
		j = strlen(str); 
		j = j - 2;
		strncpy(needle, str, j);
		strcat(needle, "\0");
		for ( i =0; i <=1000; i ++)
		{
			strcpy(haystack ,msg[i]);
			uppercase(haystack);
			local = strstr(haystack ,needle);
			if ( local != NULL )
			{
				send(sd, msg[i], strlen(msg[i]), 0);
			}
			else
			{}
		}
	j = j + 2;
	for ( i =0; i <= j ; i ++)
	{
		needle [ i ] = '\0';
		j = 0;
	}
	sem_post(&m);
	} else
	if (! strncmp(str ,"PALAVRASD" , 9)) {
		sem_wait(&m );
		b = recv(sd ,str, 999, 0);
		str[b] = 0;
		val = strtol(str, &endptr ,10);
		if ( endptr == str ) { sprintf(str, " \nFALHA " );
		// continue ;
		}
		char result[20];
		// chama funcao que conta e monta nova string
		n = sprintf(result , " Palavras : %d \n " , words(msg[val]));
		// Envia resultado
		send(sd ,result ,strlen(result), 0);
		sem_post (&m);
	} else
	if (!strncmp(str, "PALAVRAST", 9)) {
		sem_wait (&m);
		char result [30];

		for ( i = 0; i <= 1000; i ++){
			numerototal = numerototal + words(msg[i]);
		}
		n = sprintf(result , " Total Palavras : %d \n " , numerototal );
		send(sd , result , strlen(result) ,0);
		sem_post (&m);

		}
	readcount --;
		if ( readcount ==0)
		{
		sem_post(&wread);
		}
	sem_post(&read);

}


void escritor( char *str , int  sd ){
	// int sd = *(int *)sd2; 
	char *endptr , *local ;
	char cpy [1000]; // usado para armazenar ditado
	char cpy1 [1000]; // usado para armazenar ditado
	char haystack [1000]; // usado no search
	char needle [1000]; // usado no search
	int i =0 , j , b , val , val1 , readcount =0;
	int novoditado = 0;
	
	if (! strncmp ( str , "DEL" ,3)) { 
		sem_wait (&wread );
		sem_wait (&m );
		b = recv ( sd , str ,999 ,0);
		str [ b ]=0;
		val = strtol ( str , &endptr , 10);
		if ( endptr == str ) { sprintf ( str , " \nFALHA " );
		// continue ;
		}
		else sprintf ( str , " \n N m e r o do ditado apagado " );
			strcpy ( msg [ val ] , " " );
			sprintf ( str , " \nOK " );
			alteracoes ++;
			sem_post (&m );
			sem_post (&wread ); 
	} else
	if (! strncmp ( str , "ROTATE" ,6)) { 
		sem_wait (&wread );
		sem_wait (&m );
		b = recv ( sd , str ,999 ,0);
		str [ b ]=0;
		val = strtol ( str , &endptr , 10);
		if ( endptr == str ) { sprintf ( str , " \nFALHA " );
		// continue ;
		}
		else sprintf ( str , " \nOk " );
			strcpy ( cpy , msg [ val ]); // armazenar o ditado selecionado
		b = recv ( sd , str ,999 ,0); // ditado vai ser copiado para outro vetor
			str [ b ]=0;
			val1 = strtol ( str , &endptr , 10); // selecionando qual ditado vai receber
			if ( endptr == str ) { sprintf ( str , " \nFALHA " );
			// continue ;
			}
			else sprintf ( str , " \nOk " );
				strcpy ( cpy1 , msg [ val1 ]); // copiando a segunda mensagem
				strcpy ( msg [ val1 ] , cpy ); // copiando a mensagem 1 na mensagem2
				strcpy ( msg [ val ] , cpy1 ); // copiando a mensagem 2 na mensagem1
				alteracoes ++;
				sem_post (&m );
				sem_post (&wread ); // operacao up
	} else
	if (! strncmp ( str , "REPLACE" ,7)) {
		sem_wait (&wread );
		sem_wait (&m );
		b = recv ( sd , str ,999 ,0);
		str [ b ]=0;
		val = strtol ( str , &endptr , 10);
			if ( endptr == str ) { sprintf ( str , " \nFALHA " );
			// continue ;
			}
			else sprintf ( str , " \nOK " );
		
		send ( sd , str , strlen ( str ) ,0);
		b = recv ( sd , str ,999 ,0);
		str [ b ]=0;
		strcpy ( msg [ val ] , str );
		sprintf ( str , " \nOK " );
		send ( sd , str , strlen ( str ) ,0);
		printf ( " \n Novo ditado %d : %s " ,val , msg [ val ]);
		alteracoes ++;
		sem_post (&m );
		sem_post (&wread ); 
	} else
	if (! strncmp ( str , "GRAVA" ,5)) {
		sem_wait (&wread );
		sem_wait (&m );

		FILE *fptr ;
		fptr = fopen ("Ditados.txt" , "w" );
		if ( fptr == NULL )
		{
			printf ( " Erro ao abrir arquivo " );
			exit (0);
		}
		// loop que roda por todo o msg gravando no arquivo
		for ( i = 0; i <= 1000; i ++){
			fprintf ( fptr , "%s " , msg [ i ]);
		}
		fclose ( fptr );
			send ( sd , "Gravado em Ditados.txt \n " , strlen ( " Gravado em Ditados . txt \n " ) ,0);;
			sem_post (&m );
			sem_post (&wread ); 
	} else
	if (! strncmp ( str , "LE" ,2)) {
		sem_wait (&wread );
		sem_wait (&m );

		for ( i = 0; i <=1000; i ++){
			strcpy ( msg [ i ] , " " );
		}
		FILE *arq ;
		if ( ( arq = fopen ( "Ditados.txt" ,"r" )) == NULL ) { printf ( " \n Erro lendo arquivo ...\n \n " );}
		// loop que roda por por todo o arquivo lendo e gravando em msg
		for ( i = 0; ! feof ( arq ); i ++){
		fgets ( msg [ i ] ,999 , arq ); printf ( " %d %s " ,i , msg [ i ]);
		}
		send ( sd , " \n Arquivo Lido " , strlen ( " \n Arquivo Lido " ) ,0);
		sem_post (&m );
		sem_post (&wread ); 
	} else
	if (! strncmp ( str , "ALTERACOES" ,10)) {
		sem_wait (&wread );
		sem_wait (&m );
		char result [30];
		int n ;
		n = sprintf ( result , " %d \n " , alteracoes );
		send ( sd , result , strlen ( result ) ,0);
		sem_post (&m );
		sem_post (&wread ); 	
	}
}


void *atendeConexao(void *sd2 )
{
	int sd = *(int *)sd2;
	sem_post(&m0);
	char str [1000] , *endptr , *local ;
	int i = 0 , j , b , val , val1 ;
	int novoditado = 0;


	while (1) {
		visits ++;
		sprintf( str , "REQUISIÇÃO %d \n " , visits );
		send( sd , str , strlen(str) ,0);
		b = recv( sd , str ,999 ,0);
		str [b]=0;

	uppercase(str);

		if (!strncmp(str ,"GETR",4)) {
			leitor(str, sd);
		} else
		if (!strncmp(str ,"GETN" ,4)) {
			leitor(str, sd);
		}
		else
		if (!strncmp(str,"DEL",3)) { /* apagar ditado */
			escritor(str,sd);
		}
		else
		if (!strncmp(str,"ROTATE",6)) { /* trocar as p o s i s do ditado */
			escritor(str, sd);
		}
		else
		if (!strncmp(str,"SEARCH",6)) { /* procurar uma frase no ditado */
			leitor(str, sd);
		}
		else
		if (!strncmp(str,"REPLACE",7)) {
			escritor(str,sd);
		} else
		if (! strncmp ( str , "PALAVRASD" ,9)) {
			leitor ( str , sd );
		} else
		if (!strncmp(str,"PALAVRAST",9)) {
			leitor(str,sd);
		} else
		if (!strncmp(str,"GRAVA",5)) {
			escritor(str,sd);
		}
		else
		if (!strncmp(str,"LE",2)) {
			escritor(str,sd);
		} else
		if (!strncmp(str,"ALTERACOES",10)) {
			escritor(str,sd);
		}
		else
		if (!strncmp(str,"FIM",3)) {
			sprintf(str,"\n Até Logo ");
			send(sd,str,strlen(str),0);
		break ;
		}
		else
		if (!strncmp(str,"VER",3)) {
			sprintf(str," \nServidor de Ditados 1.0 Beta. " );
			send(sd,str,strlen(str),0);
		}
		else {
			sprintf(str," \nErro de Protocolo " );
			send(sd,str,strlen(str),0);
		}
	}
	close(sd);
}


int main(int argc, char **argv)
{
	struct  protoent *ptrp;  /* pointer to a protocol table entry   */
	struct  sockaddr_in sad; /* structure to hold server's address  */
	struct  sockaddr_in cad; /* structure to hold client's address  */
	int     sd, sd2;         /* socket descriptors                  */
	int     port;            /* protocol port number                */
	int     alen;            /* length of address                   */
        pthread_t t;
	
        srandom(time(NULL)); /* inicializa a semente do gerador de nÃºmeros aleatÃ³rios */
	
	memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
	sad.sin_family = AF_INET;         /* set family to Internet     */
	sad.sin_addr.s_addr = INADDR_ANY; /* set the local IP address   */
	
	/* Check command-line argument for protocol port and extract    */
	/* port number if one is specified.  Otherwise, use the default */
	/* port value given by constant PROTOPORT                       */
	
	if (argc > 1) {                 /* if argument specified        */
		port = atoi(argv[1]);   /* convert argument to binary   */
	} else {
		port = PROTOPORT;       /* use default port number      */
	}
	if (port > 0)                   /* test for illegal value       */
		sad.sin_port = htons((u_short)port);
	else {                          /* print error message and exit */
		fprintf(stderr,"bad port number %s\n",argv[1]);
		exit(1);
	}

        LeDitado();	
  
	/* Map TCP transport protocol name to protocol number */
	
	if ( ((ptrp = getprotobyname("tcp"))) == NULL) {
		fprintf(stderr, "cannot map \"tcp\" to protocol number");
		exit(1);
	}
	
	/* Create a socket */
	
	sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sd < 0) {
		fprintf(stderr, "socket creation failed\n");
		exit(1);
	}
	
	/* Bind a local address to the socket */
	
	if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
		fprintf(stderr,"bind failed\n");
		exit(1);
	}
	
	/* Specify size of request queue */
	
	if (listen(sd, QLEN) < 0) {
		fprintf(stderr,"listen failed\n");
		exit(1);
	}
	
	/* Main server loop - accept and handle requests */
	sem_init(&m, 0, 1);
	sem_init(&wread, 0, 1);
	sem_init(&read, 0, 1);
	sem_init(&m0, 0, 1);
	
	
	while (1) {
	
	
		alen = sizeof(cad);
		if ( (sd2=accept(sd, (struct sockaddr *)&cad, &alen)) < 0) {
			fprintf(stderr, "accept failed\n");
			exit(1);
		}
		printf ("\nServidor atendendo conexão %d", visits);
                pthread_create(&t, NULL,  &atendeConexao, &sd2 );
                sem_wait(&m0);
	}
}












































































