// A compiler from a very simple Pascal-like structured language LL(k)
// to 64-bit 80x86 Assembly langage
// Copyright (C) 2019 Pierre Jourlin

#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

char current;      // caractère courant
char lookahead;    // caractère suivant (pour LL(2))

void ReadChar() {
    current = lookahead;
    do {
        lookahead = cin.get();
    } while (isspace(lookahead));
}

void Error(string s) {
    cerr << "Erreur : " << s << endl;
    exit(-1);
}

void AdditiveOperator() {
    if (current == '+' || current == '-') {
        ReadChar();
    } else {
        Error("Opérateur additif attendu");
    }
}

void Digit() {
    if (current < '0' || current > '9') {
        Error("Chiffre attendu");
    } else {
        cout << "\tpush $" << current << endl;
        ReadChar();
    }
}

void ArithmeticExpression(); // déclaration anticipée

void Term() {
    if (current == '(') {
        ReadChar();
        ArithmeticExpression();
        if (current != ')') {
            Error("')' attendu");
        } else {
            ReadChar();
        }
    } else if (current >= '0' && current <= '9') {
        Digit();
    } else {
        Error("'(' ou chiffre attendu");
    }
}

void ArithmeticExpression() {
    char adop;
    Term();
    while (current == '+' || current == '-') {
        adop = current;
        AdditiveOperator();
        Term();
        cout << "\tpop %rbx" << endl;
        cout << "\tpop %rax" << endl;
        if (adop == '+')
            cout << "\taddq %rbx, %rax" << endl;
        else
            cout << "\tsubq %rbx, %rax" << endl;
        cout << "\tpush %rax" << endl;
    }
}

string ReadRelOp() {
    string op;
    op += current;

    if ((current == '<' || current == '>') && lookahead == '=') {
        op += lookahead;
        ReadChar();
    } else if (current == '<' && lookahead == '>') {
        op += lookahead;
        ReadChar();
    }

    ReadChar();
    return op;
}

void Expression() {
    ArithmeticExpression();

    if (current == '=' || current == '<' || current == '>') {
        string op = ReadRelOp();
        ArithmeticExpression();

        cout << "\tpop %rbx" << endl; // operande droite
        cout << "\tpop %rax" << endl; // operande gauche
        cout << "\tcmp %rbx, %rax" << endl;

        if (op == "=")
            cout << "\tsete %al" << endl;
        else if (op == "<>")
            cout << "\tsetne %al" << endl;
        else if (op == "<")
            cout << "\tsetl %al" << endl;
        else if (op == "<=")
            cout << "\tsetle %al" << endl;
        else if (op == ">")
            cout << "\tsetg %al" << endl;
        else if (op == ">=")
            cout << "\tsetge %al" << endl;

        cout << "\tmovzbq %al, %rax" << endl;
        cout << "\tpush %rax" << endl;
    }
}

int main() {
    cout << "\t\t\t# This code was produced by the CERI Compiler" << endl;
    cout << "\t.text\t\t# The following lines contain the program" << endl;
    cout << "\t.globl main\t# The main function must be visible from outside" << endl;
    cout << "main:\t\t\t# The main function body :" << endl;
    cout << "\tmovq %rsp, %rbp\t# Save the position of the stack's top" << endl;

    ReadChar(); // initialise current et lookahead
    ReadChar();

    Expression();

    cout << "\tmovq %rbp, %rsp\t\t# Restore the position of the stack's top" << endl;
    cout << "\tret\t\t\t# Return from main function" << endl;

    if (cin.get(current)) {
        cerr << "Caractères en trop à la fin du programme : [" << current << "]" << endl;
        Error(".");
    }

    return 0;
}
