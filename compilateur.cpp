#include <string>
#include <iostream>
#include <cstdlib>

using namespace std;

char current;       // caractère courant
char nextchar;      // prochain caractère (LL(2))

void ReadChar(void) {
    current = nextchar;
    while (cin.get(nextchar) && (nextchar == ' ' || nextchar == '\t' || nextchar == '\n'))
        ; // sauter les espaces
}

void Error(string s) {
    cerr << s << endl;
    exit(-1);
}

void AdditiveOperator(void) {
    if (current == '+' || current == '-')
        ReadChar();
    else
        Error("Opérateur additif attendu");
}

void Digit(void) {
    if (current < '0' || current > '9')
        Error("Chiffre attendu");
    else {
        cout << "\tpush $" << current << endl;
        ReadChar();
    }
}

void ArithmeticExpression(void);
void Term(void);

void Term(void) {
    if (current == '(') {
        ReadChar();
        ArithmeticExpression();
        if (current != ')')
            Error("')' attendu");
        else
            ReadChar();
    } else if (current >= '0' && current <= '9') {
        Digit();
    } else {
        Error("'(' ou chiffre attendu");
    }
}

void ArithmeticExpression(void) {
    char op;
    Term();
    while (current == '+' || current == '-') {
        op = current;
        AdditiveOperator();
        Term();
        cout << "\tpop %rbx" << endl;
        cout << "\tpop %rax" << endl;
        if (op == '+')
            cout << "\taddq %rbx, %rax" << endl;
        else
            cout << "\tsubq %rbx, %rax" << endl;
        cout << "\tpush %rax" << endl;
    }
}

// Expression ::= ArithmeticExpression [ OpRel ArithmeticExpression ]
void Expression(void) {
    ArithmeticExpression();

    if (current == '=' || current == '<' || current == '>') {
        string op;

        if (current == '<' && nextchar == '=') {
            op = "<="; ReadChar(); ReadChar();
        } else if (current == '<' && nextchar == '>') {
            op = "<>"; ReadChar(); ReadChar();
        } else if (current == '>' && nextchar == '=') {
            op = ">="; ReadChar(); ReadChar();
        } else {
            op = current; ReadChar(); // un seul caractère
        }

        ArithmeticExpression();

        cout << "\tpop %rbx" << endl; // droite
        cout << "\tpop %rax" << endl; // gauche
        cout << "\tcmp %rbx, %rax" << endl;

        if (op == "=") cout << "\tsete %al" << endl;
        else if (op == "<>") cout << "\tsetne %al" << endl;
        else if (op == "<") cout << "\tsetl %al" << endl;
        else if (op == "<=") cout << "\tsetle %al" << endl;
        else if (op == ">") cout << "\tsetg %al" << endl;
        else if (op == ">=") cout << "\tsetge %al" << endl;

        cout << "\tmovzbq %al, %rax" << endl;
        cout << "\tpush %rax" << endl;
    }
}

int main(void) {
    // Prologue assembleur
    cout << "\t\t\t# This code was produced by the CERI Compiler" << endl;
    cout << "\t.text\t\t# The following lines contain the program" << endl;
    cout << "\t.globl main\t# The main function must be visible from outside" << endl;
    cout << "main:\t\t\t# The main function body :" << endl;
    cout << "\tmovq %rsp, %rbp\t# Save the position of the stack's top" << endl;

    // Initialisation correcte
    nextchar = cin.get();
    ReadChar();

    // Analyse de l'expression
    Expression();

    // Épilogue assembleur
    cout << "\tmovq %rbp, %rsp\t# Restore the position of the stack's top" << endl;
    cout << "\tret\t\t# Return from main function" << endl;

    // Supprimer le warning GNU-stack
    cout << "\t.section .note.GNU-stack,\"\",@progbits" << endl;

    if (cin.get(current)) {
        cerr << "Caractères en trop à la fin du programme : [" << current << "]" << endl;
        Error(".");
    }

    return 0;
}
