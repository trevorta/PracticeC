#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void changeK(int* k);
char* rtrim(char* str);
char* ltrim(char* str);
char* strrev(char* str);
char* rjust(char* str);

struct Animal {
	char name[20];
	char species[20];
	int numLegs;
	int age;
};

void printAnimal(struct Animal* an) {
	printf("Pointer. Hi! I'm %s. I'm a %s. I'm %d years old.\n", an->name, an->species, an->age);
}

typedef struct Fullname {
	int id;
	char last_name[20];
	char first_name[15];
} FULLNAME;

char* rtrim(char* str) {
	for (int n = strlen(str) - 1; n > 0; n--) {
		if (*(str + n) != ' ') {
			*(str + n + 1) = '\0';
			break;
		}
	}
	return str;
}

char* ltrim(char* str) {
	strrev(str);
	rtrim(str);
	strrev(str);
	return str;
}

char *strrev(char *str)
{
      char *p1, *p2;

      if (! str || ! *str)
            return str;
      for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
      {
            *p1 ^= *p2;
            *p2 ^= *p1;
            *p1 ^= *p2;
      }

      return str;
}

char* rjust(char* str) {
	int n = strlen(str);
	char* dup_str = strdup(str);
	rtrim(dup_str);
	sprintf(str, "%*.*s", n, n, dup_str);
	free(dup_str);
	return str;
}

void change(int* i_ptr) {
	*i_ptr = 3;
}

int add(int a, int b) {
	return a + b;
}

int multiply(int a, int b) {
	return a * b;
}

int main(int argc, char * argv[]) {
	auto int k = 3;
	printf("k: %d\n", k);
	changeK(&k);
	printf("k: %d\n", k);
	for (int i = 0; i < argc; i++) {
		printf("%d: %s\n", i, argv[i]);
	}

	struct Animal a1;
	strcpy(a1.name, "Tom");
	strcpy(a1.species, "Cat");
	a1.numLegs = 4;
	a1.age = 4;
	printAnimal(&a1);

	struct Animal a2 = {"Jerry", "Mouse", 4, 3};
	printAnimal(&a2);

	printf("line: %d\n", __LINE__); // print the current line number inside the file (this is pretty awesome)
	printf("file: %s\n", __FILE__);
	printf("date: %s\n", __DATE__);
	printf("time: %s\n", __TIME__);

	FULLNAME src;
	FULLNAME dest;
	src.id = 1;
	strcpy(src.last_name, "Sanchez");
	strcpy(src.first_name, "Rick");
	//memcpy(&dest, &src, sizeof(FULLNAME) * 2); // Abort trap: 6
	memcpy(&dest, &src, sizeof(FULLNAME));
	printf("Full name: %s %s\n", dest.first_name, dest.last_name);

	char helloString[] = "       Hello World!";
	ltrim(helloString);
	printf("Modified hello string: %s\n", helloString);
	char hello[5];
	strncpy(hello, helloString, 5);
	printf("Copy hello: %s\n", hello);
	char welcome[] = "Welcome!     ";
	rjust(welcome);
	printf("Right justify hello string: %s\n", welcome);

	int i, a[10], j;
	j = 5;
	printf("j: %d\n", j);
	for (int i = 0; i < 10; i++) {
		a[i] = i;
	}
	a[10] = 4;
	printf("elem of a: %d\n", *a + 5);

	printf("address a: %d\n", &a);
	printf("address j: %d\n", &j);
	printf("address i: %d\n", &i);
	for (int* k = &i; k < &a[10]; k++ ) {
		printf("%d\n", *k);
	}


	printf("address i");
	int* num = malloc(4); // forgot to malloc integer
	*num = 12;
	printf("num: %d\n", *num);
	change(num);
	printf("num: %d\n", *num);
	free(num);

	
	//char hey[3];
	char* heyString = "hey";
	char hey[strlen(heyString)];
	strcpy(hey, heyString);
	printf("%s\n", hey);
	// printf(hey); // Abort trap: 6

	int (*binaryOperation) (int a, int b);
	binaryOperation = add;
	int res = binaryOperation(3, 4);
	printf("res: %d\n", res);
	binaryOperation = multiply;
	res = binaryOperation(3, 4);
	printf("res: %d\n", res);

}

void changeK(int* k) {
	*k += 1;
}
