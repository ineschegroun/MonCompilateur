// TP2 : Extension du compilateur (Expressions booléennes, comparateurs, variables globales, instructions)

#include <iostream>
#include <string>
#include <map>
#include <cstdlib>

using namespace std;

char current, lookedAhead;
int NLookedAhead = 0;
map<char, bool> declared;

void ReadChar() {
    if (NLookedAhead > 0) {
        current = lookedAhead;
        NLookedAhead--;
    } else {
        while (cin.get(current) && isspace(current));
    }
}

void LookAhead() {
    while (cin.get(lookedAhead) && isspace(lookedAhead));
    NLookedAhead++;
}

void Error(const string& msg) {
    cerr << "Erreur : " << msg << endl;
    exit(1);
}

void Expect(char expected) {
    if (current != expected) Error(string("caractère attendu : ") + expected);
    ReadChar();
}

void Digit() {
    if (!isdigit(current)) Error("Chiffre attendu");
    int value = current - '0';
    ReadChar();
    while (isdigit(current)) {
        value = value * 10 + (current - '0');
        ReadChar();
    }
    cout << "\tpush $" << value << endl;
}

void Letter() {
    if (current < 'a' || current > 'z') Error("Lettre attendue");
    if (!declared[current]) Error(string("Variable non déclarée : ") + current);
    cout << "\tpush " << current << endl;
    ReadChar();
}

void Factor();
void Expression();
void SimpleExpression();

void Factor() {
    if (current == '(') {
        ReadChar();
        Expression();
        Expect(')');
    } else if (isdigit(current)) {
        Digit();
    } else if (current == '!') {
        ReadChar();
        Factor();
        cout << "\tpop %rax\n\tcmp $0, %rax\n\tsete %al\n\tmovzbq %al, %rax\n\tpush %rax" << endl;
    } else {
        Letter();
    }
}

void MultiplicativeOperator() {
    if (current == '*' || current == '/' || current == '%') {
        ReadChar();
    } else if (current == '&') {
        ReadChar();
        if (current != '&') Error("l'opérateur ET s'écrit '&&'");
        ReadChar();
    } else {
        Error("Opérateur multiplicatif attendu");
    }
}

void Term() {
    Factor();
    while (current == '*' || current == '/' || current == '%' || current == '&') {
        char op = current;
        MultiplicativeOperator();
        Factor();
        cout << "\tpop %rbx\n\tpop %rax" << endl;
        switch (op) {
            case '*':
                cout << "\timul %rbx, %rax\n\tpush %rax" << endl;
                break;
            case '/':
                cout << "\tmov $0, %rdx\n\tidiv %rbx\n\tpush %rax" << endl;
                break;
            case '%':
                cout << "\tmov $0, %rdx\n\tidiv %rbx\n\tpush %rdx" << endl;
                break;
            case '&':
                cout << "\tand %rbx, %rax\n\tpush %rax" << endl;
                break;
        }
    }
}

void AdditiveOperator() {
    if (current == '+' || current == '-') {
        ReadChar();
    } else if (current == '|') {
        ReadChar();
        if (current != '|') Error("l'opérateur OU s'écrit '||'");
        ReadChar();
    } else {
        Error("Opérateur additif attendu");
    }
}

void SimpleExpression() {
    Term();
    while (current == '+' || current == '-' || current == '|') {
        char op = current;
        AdditiveOperator();
        Term();
        cout << "\tpop %rbx\n\tpop %rax" << endl;
        if (op == '+') cout << "\tadd %rbx, %rax" << endl;
        else if (op == '-') cout << "\tsub %rbx, %rax" << endl;
        else if (op == '|') cout << "\tor %rbx, %rax" << endl;
        cout << "\tpush %rax" << endl;
    }
}

string RelationalOperator() {
    string op;
    op += current;
    LookAhead();
    if ((current == '=' && lookedAhead == '=') ||
        (current == '!' && lookedAhead == '=') ||
        (current == '<' && lookedAhead == '=') ||
        (current == '>' && lookedAhead == '=')) {
        op += lookedAhead;
        ReadChar();
    }
    ReadChar();
    return op;
}

void Expression() {
    SimpleExpression();
    if (current == '=' || current == '!' || current == '<' || current == '>') {
        string op = RelationalOperator();
        SimpleExpression();
        cout << "\tpop %rbx\n\tpop %rax\n\tcmp %rbx, %rax" << endl;
        if (op == "==") cout << "\tsete %al" << endl;
        else if (op == "!=") cout << "\tsetne %al" << endl;
        else if (op == "<") cout << "\tsetl %al" << endl;
        else if (op == "<=") cout << "\tsetle %al" << endl;
        else if (op == ">") cout << "\tsetg %al" << endl;
        else if (op == ">=") cout << "\tsetge %al" << endl;
        cout << "\tmovzbq %al, %rax\n\tpush %rax" << endl;
    }
}

void Assignment() {
    char var = current;
    if (!declared[var]) Error("Variable non déclarée");
    ReadChar();
    Expect('=');
    Expression();
    cout << "\tpop " << var << endl;
}

void Statement() {
    Assignment();
}

void StatementPart() {
    Statement();
    while (current == ';') {
        ReadChar();
        Statement();
    }
    Expect('.');
}

void DeclarationPart() {
    Expect('[');
    while (current >= 'a' && current <= 'z') {
        declared[current] = true;
        cout << current << ":\t.quad 0" << endl;
        ReadChar();
        if (current == ',') ReadChar();
        else break;
    }
    Expect(']');
}

int main() {
    cout << ".data" << endl;
    ReadChar();
    if (current == '[') DeclarationPart();

    cout << ".text\n.globl main\nmain:\n\tmov %rsp, %rbp" << endl;
    StatementPart();
    cout << "\tmov %rbp, %rsp\n\tret" << endl;
    return 0;
}
