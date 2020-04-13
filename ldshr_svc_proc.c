#include <stdio.h>
#include <string.h>
#include <rpc/rpc.h>
#include <math.h>
#include "ldshr.h"

static double *ldp = NULL;

double reduction(int x, int y, int z);
double sqroot(double num);
double local_sum(double num1, double num2);
void map(double (*f)(double), struct node *list);
double reduce(double (*f)(double, double), struct node *list);


double *
getload_1_svc(char **srvname, struct svc_req *rqp){
  double loadavg[3] = {-1, -1, -1};
  if (getloadavg(loadavg, 3) != -1)
  {
    ldp = (double *) malloc(sizeof(double*));
    ldp = &loadavg[0];
    printf("%f\n", loadavg[0]);
    return ((double *)ldp);
  }
  else{
  	printf("get load failed\n");
  	return &(loadavg[0]);
  }
}


double *
sumqroot_gpu_1_svc(struct gpu_struct *param, struct svc_req *rqp){
	double  *result = (double *)malloc(sizeof(double *));
    *result =reduction(param->N, param->mean, param->seed);
  	return result;
}


double *
sumqroot_lst_1_svc(struct node *param, struct svc_req *rqp){
  double  *result = (double *)malloc(sizeof(double));
  struct node *local = param;
  map(sqroot, local);
  *result = reduce(local_sum, local);
  return result;
}


double sqroot(double num){
  return sqrt(sqrt(num));
}


double local_sum(double num1, double num2){
  return num1 + num2;
}


void map(double (*f)(double), struct node *list){
  struct node *tmp = list;
  while(tmp){
    tmp->num = (*f)(tmp->num);
    tmp = tmp->next;
  }
}


double reduce(double (*f)(double, double), struct node *list){
  struct node *tmp = list;
  double val = 0;
  while(tmp){
    val = (*f)(val, tmp->num);
    tmp = tmp->next;
  }
  return val;
}
