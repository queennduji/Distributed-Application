/*
 * rdb.x: remote database access protocol
 */
/* preprocessor directives */
%#define DATABASE "personnel.dat"	/* '%' passes it through */

struct gpu_struct{
  int N;
  int mean;
  int seed;
};

struct node{
	double num;
	struct node *next;
};

/* program definition, no union or typdef definitions needed */
program RDBPROG { /* could manage multiple servers */
	version RDBVERS {
		double GETLOAD(string) = 1;
		double SUMQROOT_GPU(struct gpu_struct) = 2;
		double SUMQROOT_LST(struct node) = 3;
	} = 1;
} = 0x20003997;  /* program number ranges established by ONC */
