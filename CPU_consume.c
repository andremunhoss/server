#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int max , t ;

void *codigo_thread (void *arg)
{
	int i , j , k ;
	int vet [100];

	printf ( " \nProcesso %d , Thread %u : Inicio \n " , getpid() , (unsigned int)pthread_self());
	printf ( " \nProcesso %d , Thread %u : Processando \n " , getpid() , (unsigned int)pthread_self());

	for ( i =0; i < max ; i ++)
		for ( j =0; j < max ; j ++)
			for ( k =0; k < max ; k ++)
				vet [(i + j + k) %100] = i *j *k ;

	printf ( " Processo %d , Thread %u : Finalizando \n " , getpid() ,(unsigned int)pthread_self());
	return NULL ;
	}

int main (int argc ,char *argv[])
{	
	pthread_t *thread;
	int *status , i , arg =0;

	if (argc!=3) {
		printf ( " \nMain : Ops %d . Errou o comando : \n %s < numero maximo de interacoes > < numero de threads >\n \n " , argc , argv[0]);
		exit (0);
	}
	max = atoi(argv[1]);
	t = atoi(argv[2]);

	thread = (pthread_t *) malloc (t * sizeof ( pthread_t ) );
	status = (int *) malloc ( t * sizeof ( int ) );
	// Criando as threads

	for ( i =0; i < t ; i ++) {
		printf ( " \nMain : Programa principal " );
		printf ( " \nMain : .... Criando Thread %d ", i);
		status[i] = pthread_create(&thread[i],NULL, &codigo_thread, &arg);
		if (status[i] != 0) {
			printf ( " \nMain : .... ops .... erro ... pthread _create retornou o erro %d \n " , status[i]);
			exit (-1);
		}
	}

	printf ( "\nMain : Espera pelas tarefas completarem e libera seus recursos" );
	for ( i =0; i <t ; ++ i ) {
		pthread_join(thread[i], &status[i]);
	}

	printf ( "\nMain : Confere se o status de finalizacao das threads está correto" );
	for ( i =0; i <t ; ++ i ) {
		if ( status[i] != NULL ) {
			printf ( "\nMain: Status da thread %d inesperado - o status retornado tem cogido %d\n" , i , status[i]);
		}
	}

	printf ( " \nMain : Fim \n \n " );

	return (0);
}




















