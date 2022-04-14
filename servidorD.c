// Universidade Federal do ParanÃ¡
// TE355 - Sistemas Operacionais Embarcados
// Base para o Trabalho 1 - 2022
// Prof. Pedroso

#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>

#define PROTOPORT       9551           /* default protocol port number */
#define QLEN            6               /* size of request queue        */

int     visits      =   0;              /* counts client connections    */
char    msg [1000][1000];
int     ditados=0;

		
void LeDitado()
{
    FILE *arq;
 
    if ( (arq=fopen("Ditados.txt","r")) == NULL ) { printf("\n Erro lendo arquivo ...\n\n");exit(0);}
    while (!feof(arq)) {
       
       fgets(msg[ditados],999,arq); 
       // para debug
       // printf("%d %s",ditados,msg[ditados]);
       ditados=(ditados+1)%1000;
    }
    fclose(arq);
    printf("\n\nCarregou %d ditados",ditados);   
     
}
							
int words(const char sentence[ ])
{
	int contado = 0; // resultado
	const char* it = sentence;
	int inword = 0;

	do switch(*it) {
		case '\0':
		case ' ': case '\t': case '\n': case '\r': //Procura por esses caracteres
			if (inword) { inword = 0; contado++; }// conta
			break;
		default: inword = 1;
	} while(*it++);
	return contado; //retorna resultado
}


char uppercase(char *input) {
    char i=0;
    for(i = 0; (i<strlen(input)) && (i<1000); i++) {
        input[i] = toupper(input[i]);
    }
    return i;
}


void *atendeConexao( void *sd2 )
{
	int *temp=sd2;
	int sd=*temp;
	char str[2000], *endptr,  *local;
	char cpy[1000];//usado para armazenar ditado
	char cpy1[1000];//usado para armazenar ditado
	char haystack[1000];//usado no search
	char needle[1000];//usado no search
	int i=0, b, j, val, val1, alteracoes=0;

        while (1) {	
		visits++;
		sprintf(str,"Requisição %d\n", visits);
		send(sd,str,strlen(str),0);
		b=recv(sd,str,999,0);
                str[b]=0;
                printf("\nComando recebido:%s",str);
	 
		uppercase(str);
	       	
		if (!strncmp(str,"GETR",4)) {
		     sprintf(str,"\nDitado %d: %s", rand() % ditados, msg[rand() % ditados]);
		     send(sd,str,strlen(str),0);
		}
		else
        	if (!strncmp(str,"GETN",4)) {
                     b=recv(sd,str,999,0);
                     str[b]=0;
                     val = strtol(str, &endptr, 10);
                     if (endptr==str)  {sprintf(str,"\nFALHA");continue;}
                     else  send(sd,msg[val],strlen(msg[val]),0);  
		}
		else
		if (!strncmp(str,"REPLACE",7)) {
		     b=recv(sd,str,999,0);
                     str[b]=0;
                     val = strtol(str, &endptr, 10);
                     if (endptr==str)  {sprintf(str,"\nFALHA");continue;}
                     else sprintf(str,"\nOK");
		     send(sd,str,strlen(str),0);
		     b=recv(sd,str,999,0);
                     str[b]=0;
                     strcpy(msg[val],str);
                     sprintf(str,"\nOK");
		     send(sd,str,strlen(str),0);  
                     printf("\nNovo ditado %d: %s",val,msg[val]);
               }
               else
		//------- Codigo para DEL ------------------
		if (!strncmp(str,"DEL",3)) {
			b=recv(sd,str,999,0);
				str[b]=0;
				val = strtol(str, &endptr, 10);
				if (endptr==str) {sprintf(str,"\nFALHA");continue;}
				else sprintf(str,"\nN mero do ditado apagado");
				strcpy(msg[val],""); 
				sprintf(str,"\nOK");
		//---Alteracao
			alteracoes++;
		}
		else
		//------- Codigo para ROTATE ----------------
		if (!strncmp(str,"ROTATE",6)) {/*trocar as posicoes do ditado*/
			b=recv(sd,str,999,0);
				str[b]=0;
				val = strtol(str, &endptr, 10);
			if (endptr==str) {sprintf(str,"\nFALHA");continue;}
				else sprintf(str,"\nOk");
				strcpy(cpy,msg[val]); //armazenar o ditado selecionado
			b=recv(sd,str,999,0);//ditado vai ser copiado para outro vetor
				str[b]=0;
				val1= strtol(str, &endptr, 10);//selecionando qual ditado vai receber
				if (endptr==str) {sprintf(str,"\nFALHA");continue;}
				else sprintf(str,"\nOk");
			strcpy(cpy1,msg[val1]);//copiando a segunda mensagem
				strcpy(msg[val1],cpy);//copiando a mensagem 1 na mensagem2
			strcpy(msg[val],cpy1);//copiando a mensagem 2 na mensagem1

			//---Alteracao
			alteracoes++;
			}
		else
		//------- Codigo para SEARCH ----------------
		if (!strncmp(str,"SEARCH",6)) {/*procurar uma frase nos ditados*/
				j = 0;
				b=recv(sd,str,999,0);
				str[b]=0;
				uppercase(str);
				j = strlen(str);//conta o tamanho do str
				j = j - 2;
				strncpy(needle,str,j);
				strcat(needle, "\0");

			for(i=0;i<=1000;i++){
				strcpy(haystack,msg[i]);
				uppercase(haystack);
				local = strstr(haystack, needle); 
				if(local != NULL){
				send(sd,msg[i],strlen(msg[i]),0);
				}
			}
				j = j + 2;

			for(i=0;i<=j;i++)
			{
				needle[i] = '\0';
				j = 0;
			}
		}
		else
		//------- Codigo para PALAVRASD ----------------
		if (!strncmp(str,"PALAVRASD",9)) {
				b=recv(sd,str,999,0);
				str[b]=0;
				val = strtol(str, &endptr, 10);
			if (endptr==str) {sprintf(str,"\nFALHA");continue;}
				char result [20];
					int n, i;
				// chama funcao que conta e monta nova string
					n=sprintf (result, "Palavras: %d\n", words(msg[val]));
					// Envia resultado
				send(sd,result,strlen(result),0);
		}
		else
		//------- Codigo para PALAVRAST ----------------
		if (!strncmp(str,"PALAVRAST",9)) {

				char result [30];
					int n, i, numerototal = 0;
					// loop que roda por todas as linhas do msg[] contando
				for( i = 0; i <= 1000; i++){
						numerototal = numerototal + words(msg[i]);
					}

				n=sprintf (result, "Total Palavras: %d\n", numerototal);
				send(sd,result,strlen(result),0);
		}
		else
		//------- Codigo para GRAVA ----------------
		if (!strncmp(str,"GRAVA",5)) {

		FILE *fptr;

			fptr = fopen("Ditados.txt", "w"); // abre arquivo escrita
			if(fptr == NULL)
				{
				printf("Erro ao abrir arquivo");
				exit(0);
				}
			//loop que roda por todo o msg gravando no arquivo
			for( i = 0; i <= 1000; i++){
				fprintf(fptr,"%s", msg[i]);
			}
			fclose(fptr); // fecha arquivo
			send(sd,"Gravado em Ditados.txt\n",strlen("Gravado em Ditados.txt\n"),0);
		}
		else
		//------- Codigo para LE ----------------
		if (!strncmp(str,"LE",2)) {
			// limpa o dicionario atual
			for( i = 0; i<=1000; i++){
				strcpy(msg[i], "");
			}

			FILE *arq;

			if ( (arq=fopen("Ditados.txt","r")) == NULL ) { printf("\n Erro lendo arquivo ...\n\n");exit(0);}
			//loop que roda por por todo o arquivo lendo e gravando em msg
			for( i = 0; !feof(arq); i++){
				fgets(msg[i],999,arq); printf("%d %s",i,msg[i]);
			}

			send(sd,"\n Arquivo Lido",strlen("\n Arquivo Lido"),0);

		}
		else
		//------- Codigo para ALTERACOES ----------------
		// note que foi adionado uma variavel global
		
		//E sempre que for feita uma alteracao (DEL, ROTATE e REPLACE) 163 //ele soma 1 contando assim as alteacoes.

		if (!strncmp(str,"ALTERACOES",10)) {
			char result [30];
			int n;
			//Junta a string e o resultado do contado
			n=sprintf (result, "Total Alteracoes: %d\n", alteracoes);
			//envia
			send(sd,result,strlen(result),0);
		}
		else
		if (!strncmp(str,"FIM",3)) {
		     sprintf(str,"\nAté Logo");
		     send(sd,str,strlen(str),0);
                     break;
               }
	       else
	    	if (!strncmp(str,"VER",3)) {
		     sprintf(str,"\nServidor de Ditados 2.0 Beta.\nTE355 2022 Primeiro Trabalho");
		     send(sd,str,strlen(str),0);
	       }
	       else{
	       		printf("\nComaninho:%s",str);
	       		sprintf(str,"\nErro de Protocolo\n");
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
	
	while (1) {
		alen = sizeof(cad);
		if ( (sd2=accept(sd, (struct sockaddr *)&cad, &alen)) < 0) {
			fprintf(stderr, "accept failed\n");
			exit(1);
		}
		printf ("\nServidor atendendo conexão %d", visits);
                pthread_create(&t, NULL,  atendeConexao, &sd2 );
	}
}
