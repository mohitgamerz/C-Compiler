#include "code_gen.c"

int main ()
{
	statements ();
	int status = system("python asmScript.py");
}
