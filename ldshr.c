#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <rpc/rpc.h>
#include <pthread.h>
#include "ldshr.h"
#include <math.h>

#define MACHINE_NUM 5
CLIENT *cl[MACHINE_NUM];
int min_idx[2];
char *srvname[] = {
  "bach", 
  "davinci",
  "degas",
  "arthur",
  "chopin"
};

struct load_passing{
  CLIENT *cl;
  char *srvname;
  double load;
};

struct gpu_passing{
  CLIENT *cl;
  struct gpu_struct *p;
  double *dp;
};

struct lst_passing{
  CLIENT *cl;
  struct node *p;
  double dp;
};

struct file_data{
	double num;
	char c;
};

void check_input(int argc, char *argv[]);
void check_load();
void *getload(void *p);
void *gpu(struct gpu_passing *p);
void *lst(void *p);
void run_gpu(char *arg[], int idx1, int idx2);
void run_lst(char *arg[], int idx1, int idx2);
void read_file(char *filename, struct node **l1, struct node **l2);
void write_file(char *filename);


main(argc, argv)
  int             argc;
  char           *argv[];
{
  char *srvrun[2];
  // Check user input.
  check_input(argc, argv);
  // Get workload of remote servers.
  check_load();
  // Run gpt
  if(strcmp(argv[1], "-gpu")==0){
    run_gpu(argv, min_idx[0], min_idx[1]);
  }
  if(strcmp(argv[1], "-lst")==0){
    run_lst(argv, min_idx[0], min_idx[1]);
  }  
}

void run_lst(char *argv[], int idx1, int idx2){
	// Variables
	struct node *list1;
	struct node *list2, *tmp;
	struct lst_passing p1, p2;
	pthread_t p_id1, p_id2;
	// Contents
	write_file(argv[2]);
	read_file(argv[2], &list1, &list2);
	if (list1==NULL && list2==NULL)
	{
		printf("No data in %s.\n", argv[2]);
	}
	else if (list1!=NULL && list2==NULL)
	{
		p1.cl = cl[idx1];
		p1.p = list1;
		p1.dp = -1;
		if(pthread_create(&p_id1, NULL, lst, (void *)&p1) !=0){
	      printf("Create new thread failed! Program terminates!\n");
	      exit(0);
	  	}
	  	pthread_join(p_id1, NULL); 
	  	printf("%s returns %.3f. Sum is %.3f\n", srvname[idx1], p1.dp, p1.dp);
	}
	else if (list1==NULL && list2!=NULL)
	{
		p2.cl = cl[idx2];
		p2.p = list2;
		p2.dp = -1;
		if(pthread_create(&p_id2, NULL, lst, (void *)&p2) !=0){
	      printf("Create new thread failed! Program terminates!\n");
	      exit(0);
	  	}
	  	pthread_join(p_id2, NULL); 	
	  	printf("%s returns %.3f. Sum is %.3f\n", srvname[idx2], p2.dp, p2.dp);	
	}
	else if (list1!=NULL && list2!=NULL)
	{
		p1.cl = cl[idx1];
		p1.p = list1;
		p1.dp = -1;
		p2.cl = cl[idx2];
		p2.p = list2;
		p2.dp = -1;
		if(pthread_create(&p_id1, NULL, lst, (void *)&p1) !=0){
	      printf("Create new thread failed! Program terminates!\n");
	      exit(0);
	  	}	  	
		if(pthread_create(&p_id2, NULL, lst, (void *)&p2) !=0){
	      printf("Create new thread failed! Program terminates!\n");
	      exit(0);
	  	}
	  	pthread_join(p_id1, NULL); 
	  	pthread_join(p_id2, NULL);
	  	printf("\nData sent to %s\n", srvname[idx1]);
	  	tmp = list1;
	  	while(tmp){
	  		printf("%.3f, ", tmp->num);
	  		tmp = tmp->next;
	  	}
	  	printf("\n%s returns %.3f.\n", srvname[idx1], p1.dp);
	  	printf("\nData sent to %s\n", srvname[idx2]);
	  	tmp = list2;
	  	while(tmp){
	  		printf("%.3f, ", tmp->num);
	  		tmp = tmp->next;
	  	}
	  	printf("\n%s returns %.3f.\n", srvname[idx2], p2.dp); 	
	  	printf("\nSum is %.3f\n\n", p1.dp + p2.dp);
	}
}


void *lst(void *p){ 
	struct lst_passing *local = (struct lst_passing *)p;
	local->dp = *(sumqroot_lst_1(local->p, local->cl));
}


void read_file(char *filename, struct node **l1, struct node **l2){	
	struct node *l1_h = NULL;
	struct node *l1_e = NULL;
	struct node *l2_h = NULL;
	struct node *l2_e = NULL;
	struct node *tmp;
	struct file_data cell;
	int flag = 1;
	FILE *fptr;
	if ((fptr = fopen(filename,"r")) == NULL){
       printf("Error! Read file");
       exit(0);
   	}
   	while(fread(&cell, sizeof(struct file_data), 1, fptr) == 1){
   		tmp = (struct file_data *)malloc(sizeof(struct file_data));
   		tmp->num = cell.num;
   		tmp->next = NULL;
   		if (flag == 1)
   		{
   			flag = 2;
   			if (l1_h == NULL)
   			{
   				l1_h = tmp;
   				l1_e = tmp;
   			}
   			else{
   				l1_e->next = tmp;
   				l1_e = l1_e->next;
   			}
   		}
   		else{
   			flag = 1;
   			if (l2_h == NULL)
   			{
   				l2_h = tmp;
   				l2_e = tmp;
   			}
   			else{
   				l2_e->next = tmp;
   				l2_e = l2_e->next;
   			}
   		}
   	}
   fclose(fptr); 
   *l1 = l1_h;
   *l2 = l2_h;
}

void check_input(int argc, char *argv[]){
  if(argc!=3 && argc!=6){
    printf("Wrong number for parameters!\n");
    exit(0);
  }
  if(strcmp(argv[1], "-gpu") != 0 && strcmp(argv[1], "-lst") != 0 ){
    printf("Wrong command!\n");
    exit(0);
  }
  if(strcmp(argv[1], "-gpu")==0){
    if(argc!=6)
      printf("Wrong parameter input\n");
  }
  if(strcmp(argv[1], "-lst")==0){
    if(argc!=3)
      printf("Wrong parameter input\n");
  }
}


void check_load(){
  pthread_t p_id[MACHINE_NUM];
  int i;
  double tmp;
  struct load_passing load_para[MACHINE_NUM];
  // Create communication.
  for (i=0; i<MACHINE_NUM; i++)
  {
    if (!(cl[i] = clnt_create(srvname[i], RDBPROG, RDBVERS, "tcp"))) {
      clnt_pcreateerror(srvname[i]);
      exit(1);
    }
  }
  // Get load of remote servers.
  for (i=0; i<MACHINE_NUM; i++)
  {
    double a = -1;
    load_para[i].cl = cl[i];
    load_para[i].srvname = srvname[i];
    load_para[i].load = -1;
    if(pthread_create(&p_id[i], NULL, getload, (void *)&load_para[i]) !=0){
      printf("Create new thread failed! Program terminates!\n");
      exit(0);
    } 
  }
  for (i=0; i<MACHINE_NUM; i++)
  {
    pthread_join(p_id[i], NULL);
  }
  // Pick out two servers with minimum workload.
  min_idx[0] = 0;
  for (i=1; i<MACHINE_NUM; i++){
    if(load_para[i].load < load_para[min_idx[0]].load){
      min_idx[0] = i;
    }
  }
  if(min_idx[0] == 0){
    min_idx[1] = 1;
  }
  else{
    min_idx[1] = 0;
  }
  for (i=0; i<MACHINE_NUM; i++){
    if(i == min_idx[0]){
      continue;
    }
    if(load_para[i].load < load_para[min_idx[1]].load){
      min_idx[1] = i;
    }
  }  
  // Print results.
  printf("\n");
  for (i=0; i<MACHINE_NUM; i++)
  {
    printf("%s %.3f\t", srvname[i], load_para[i].load);
  }
  printf("\n");
  printf("Execute on %s and %s\n\n", srvname[min_idx[0]], srvname[min_idx[1]]);
}


void *getload(void *p){ 
  struct load_passing *ptr = (struct load_passing *)p;
  ptr->load = *(getload_1(&(ptr->srvname), ptr->cl));
  return;
}


void run_gpu(char *argv[], int idx1, int idx2){
  int N = atoi(argv[2]);
  int mean = atoi(argv[3]);
  int seed_1 = atoi(argv[4]);
  int seed_2 = atoi(argv[5]);
  struct gpu_struct g1, g2;
  struct gpu_passing p1, p2;
  pthread_t p_id1, p_id2;
  g1.N = N-1;
  g1.mean = mean;
  g1.seed = seed_1;
  p1.cl = cl[idx1];
  p1.p = &g1;
  p1.dp = (double *)malloc(sizeof(double*));
  g2.N = N-1;
  g2.mean = mean;
  g2.seed = seed_2;
  p2.cl = cl[idx2];
  p2.p = &g2;
  p2.dp = (double *)malloc(sizeof(double*));
  if(pthread_create(&p_id1, NULL, gpu, (void *)&p1) !=0){
      printf("Create new thread failed! Program terminates!\n");
      exit(0);
  }     
  if(pthread_create(&p_id2, NULL, gpu, (void *)&p2) !=0){
      printf("Create new thread failed! Program terminates!\n");
      exit(0);
  } 
  pthread_join(p_id1, NULL);
  pthread_join(p_id2, NULL);
  printf("%s returns %.3f, %s returns %.3f, sum is %.3f\n\n", srvname[idx1], *(p1.dp), srvname[idx2], *(p2.dp), *(p1.dp)+*(p2.dp));
}


void *gpu(struct gpu_passing *p){ 
  p->dp = sumqroot_gpu_1(p->p, p->cl);
}


void write_file(char *filename){
	struct file_data cell;
	time_t t;
	int i;
	srand((unsigned) time(&t));
	cell.c = ' ';
	FILE *fptr;
	if ((fptr = fopen(filename,"w")) == NULL){
       printf("Error! Write file");
       exit(0);
   }
   printf("Write to file\n");  
   for(i=0; i<10; i++)
   {
      cell.num = (double)(rand() % 50);
      printf("%.3f, ", cell.num);
      fwrite(&cell, sizeof(struct file_data), 1, fptr); 
   }
   printf("\n");
   fclose(fptr); 
}
