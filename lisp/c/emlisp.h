/* emlisp --embedded lisp
 (c) Toshihiro Matsui, 2018
*/


#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_HEAP (30*1024)
#define MAX_BIX_BITS 3
#define MAX_BIX 7
#define MAX_CID_BITS 5
#define MAX_CID 31
#define MAX_REFC_BITS 3
#define MAX_REFC 7  /* 2**MAX_REFC_BITS -1*/

typedef struct cell *pointer;
struct cellheader {
   unsigned int mark:1;
   unsigned int b:1;	/*buddy*/
   unsigned int m:1;	/*memory*/
   unsigned int bix: MAX_BIX_BITS; 	/*buddy index - 3 bits*/
   unsigned int elmt:2; /*element type*/
   unsigned int refc: MAX_REFC_BITS; /*reference count 3 bits*/
   /* unsigned int extra:1; */
   unsigned int cid: MAX_CID_BITS;	/*class id 5 bits*/
  }  /*16 bits=short word in total*/

/* structs for object oriented programming */
struct object {
  pointer iv[2];};	/*at least two instance variables*/


struct cons {
    pointer car,	/*cons is made of a car and a cdr*/
	    cdr;};	/*what a familiar structure!*/

struct propertied_object {
    pointer plist;};

struct symbol {
    pointer plist,	/*inherited from prop_obj*/
	    speval,
	    vtype,	/*const,var,special*/
	    spefunc,
	    pname,
	    homepkg;};

struct string {		/*resembles with vector*/
    pointer length;	/*boxed*/
    byte chars[1];};	/*long word aligned*/

struct foreign {
    pointer length;
    byte *chars; };

struct package {
    pointer plist;
    pointer names;	/*package name at car, nicknames in cdr*/
    pointer use;	/*spreaded use-package list*/
    pointer symvector;	/*hashed obvector*/
    pointer symcount;	/*number of interned symbols in this package*/
    pointer intsymvector;
    pointer intsymcount;
    pointer shadows;
    pointer used_by;
    };

struct code {
    pointer codevec;
    pointer quotevec;
    pointer subrtype;	/*function,macro,special*/
    pointer entry;	/*offset from beginning of codevector*/
    };

struct fcode {		/*foreign function code*/
    pointer codevec;
    pointer quotevec;
    pointer subrtype;
    pointer entry;
  pointer entry2;    /* kanehiro's patch 2000.12.13 */
    pointer paramtypes;
    pointer resulttype;};

struct ldmodule {	/*foreign language object module*/
    pointer codevec;
    pointer quotevec;
    pointer subrtype;	/*function,macro,special*/
    pointer entry;
    pointer symtab;
    pointer objname;
    pointer handle;};	/* dl's handle */

struct closure {
    pointer codevec;
    pointer quotevec;
    pointer subrtype;	/*function,macro,special*/
    pointer entry;	/*offset from beginning of codevector*/
    pointer env0;	/*upper closure link*/
    pointer *env1;	/*argument pointer:	argv*/
    pointer *env2;};	/*local variable frame:	local*/

struct stream {
    pointer plist;
    pointer direction;
    pointer buffer;
    pointer count;
    pointer tail;};

struct filestream {
    pointer plist;
    pointer direction;
    pointer buffer;
    pointer count;
    pointer tail;
    pointer fd;
    pointer fname;};

struct iostream {
    pointer plist;
    pointer in,out;};

struct labref {		/*used for reading labeled forms: #n#,#n=*/
    pointer label;
    pointer value;
    pointer unsolved;
    pointer next; };

struct vector {
    pointer size;
    pointer v[1];};

struct intvector {
    pointer length;
    eusinteger_t iv[1];};

struct floatvector {
    pointer length;
    eusfloat_t fv[1];};

struct arrayheader {
  pointer plist;
  pointer entity,
	  rank,
	  fillpointer,
	  offset,
	  dim[ARRAYRANKLIMIT];};

struct _class {
  pointer plist;
  pointer name;		/*class name symbol*/
  pointer super;	/*super class*/
  pointer cix;
  pointer vars;		/*var names including inherited ones*/
  pointer types;
  pointer forwards;
  pointer methods;	/*method list*/
  };

struct vecclass {	/*vector class*/
  pointer plist;
  pointer name;
  pointer super;
  pointer cix;
  pointer vars;
  pointer types;
  pointer forwards;
  pointer methods;
  pointer elmtype;
  pointer size;};

struct readtable {
  pointer plist;
  pointer syntax;
  pointer macro;
  pointer dispatch;
  pointer readcase;};

/* extended numbers */
struct ratio {
  pointer numerator;
  pointer denominator;};

struct complex {
  pointer real;
  pointer imaginary;};

struct bignum {
  pointer size;
  pointer bv;}; /*bignum vector*/




/******************************************************************/

struct cell {
   struct cellheader h;
   union cellunion {
      struct cons cons;
      struct symbol sym;
      struct string str;
      struct foreign foreign;
      struct stream stream;
      struct filestream fstream;
      struct iostream iostream;
      struct code code;
      struct fcode fcode;
      struct ldmodule ldmod;
      struct closure clo;
      struct labref lab;
      struct arrayheader ary;
      struct vector vec;
      struct floatvector fvec;
      struct intvector ivec;
      struct object obj;
      struct _class cls;
      struct vecclass vcls;
      struct readtable rdtab;
      struct threadport thrp;
      struct ratio ratio;
      struct complex cmplx;
      struct bignum  bgnm;
      } c;
    } cell;

#define inc_refc(x) (x->h.refc=min(x->h.refc+1, MAX_REFC))

