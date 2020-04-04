/*

raw statement = (2>3 | 1<0) && (1==1)
LISP structure: 
    rawState -> (2>3 | 1<0) && (1==1)
    CAR - &&
    CDR - { (, { | ,{ {>, 2,3}, {<,1,0} } } }

    IF LEFT SIDE 
        state1 (left) -> (2>3 | 1<0)
        state1 = CAR(CDR(rawState)) - { (, { |, {>, 2,3} , {<, 1, 0} } } 
        - CAR(state1) = (
        - CDR(state1) = { |, {>, 2,3} , {<, 1, 0} } } 
        
        state1A (left) -> 2>3
        state1A = CAR(CDR(state1)) - {>, 2,3} 
        - CAR(state1A) = >
        - CDR(state1A) = 2,3

        state1B (right) -> 1<0
        state1B = CAR(CDR(CDR(state1)) - {<, 1, 0}
        - CAR(state1B) = <
        - CDR(state1B) = 1,0

    IF RIGHT SIDE
        state2 (right) -> (1==1)
        state2 = CAR(CDR(CDR(rawState))) -{ ( , {== , 1, 1})
        - CAR(state2) = (
        - CDR(state2) = {== , 1, 1}

        state2b (final) -> 1==1
        state2b = CAR(CDR(state2)) - {== , 1, 1}
        - CAR(state2b) = ==
        - CDR(state2b) = 1,1
*/

#include <stdio.h>
#include <string.h>

void inCollection(char ** collection, char * check){
    int nCollection = 15;
    for(int i=0; i<15; i++){
        if(strcmp(check, collection[i]) == 0){
            printf("%s is in collection... pos %d\n",check, i+1);
            return;
        }
    }
    printf("%s not in the collection...\n", check);
}

int main(){
    char * collection [] = {
    "+", "-", "*", "/", "==", "!=", "<", ">", "<=", ">=",
    "&", "|", "&&", "||", "!" };

    inCollection(collection, ">");
    inCollection(collection, ".");
    return 0;
}

void extractStatement(SEXP statement){
	int finish;
	

	SEXP auxStatement = statement;

	do{
		SEXP stateCar = auxStatement;
		printf("\n\nTYPEOF(stateCar) %d\n", TYPEOF(stateCar));
		switch (TYPEOF(stateCar))
		{
			case LANGSXP:
				puts("Its a lang");
				printf("value _ %s\n", CHAR(PRINTNAME(CAR(stateCar))));
				
				if(strncmp(CHAR(PRINTNAME(CAR(stateCar))),">", 1) == 0) {
					puts("Inside...");
					SEXP left_val = CAR(CDR(stateCar));
					SEXP right_val = CAR(CDR(CDR(stateCar)));

					printf("lval type %d\n", TYPEOF(left_val));
					printf("rval type %d\n", TYPEOF(right_val));
					compareStatement(CAR(stateCar), left_val, right_val);
					
					// double leftVal = REAL(left_val)[0];
					// double rightVal = REAL(right_val)[0];
					// printf("Result %s\n", (leftVal > rightVal) ? "TRUE" : "FALSE");
					
				}
				break;
			case REALSXP:
				printf("just a number %.2f\n", REAL(stateCar)[0]);
				
				break;
			default:
				break;
		}
		if(CDR(auxStatement) == NILSXP)
			break;
		
	}while(finish != 1);
	
}