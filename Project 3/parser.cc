/*
 * Copyright (C) Rida Bazzi, 2016
 * Edited by Joshua Elkins, 2023
 *
 * Do not share this file with anyone
 */
#include <iostream>
#include <istream>
#include <vector>
#include <string>
#include <cctype>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "inputbuf.h"
#include "parser.h"
using namespace std;

// Functions outside the class
struct scopeResolve {
 char* scope;
 scopeResolve* next;
};

struct sTableEntry {
    char* name;
    char* scope;
    int pubpriv;
    int binNo;
    int type;
};

struct sTable {
sTableEntry* item;
sTable *prev;
sTable *next;
};

sTable* symbolTable;
char* currentScope;
char* lResolve;
char* rResolve;
scopeResolve* scopeTable;
int currentPrivPub = 0;
int currBinNo = 1;
int binNoToUse = 1;
TokenType statement_type;

struct sTableEntry *lvalue;

void addScope(void){
    if(scopeTable == NULL){
        scopeResolve* newScopeItem = (scopeResolve *) malloc(sizeof(scopeResolve));
        newScopeItem->scope = (char *)malloc(sizeof(currentScope));
        memcpy(newScopeItem->scope,currentScope,sizeof(currentScope));
        newScopeItem->next = NULL;
        scopeTable = newScopeItem;
    }else{
        scopeResolve* tempTable = scopeTable;
        while(tempTable->next != NULL){
            tempTable = tempTable->next;
        }   
        scopeResolve* newScopeItem = (scopeResolve *) malloc(sizeof(scopeResolve));
        newScopeItem->scope = (char *)malloc(sizeof(currentScope));
        memcpy(newScopeItem->scope,currentScope,sizeof(currentScope));
        newScopeItem->next = NULL;
        tempTable->next = newScopeItem;
    }
}

void deleteScope(void){
        scopeResolve* tempTable = scopeTable;
        if(tempTable != NULL){
            if(tempTable->next == NULL){
                tempTable = NULL;
            }else{
                while(tempTable->next->next != NULL){
                    tempTable = tempTable->next;
                }
                currentScope = (char *)malloc(sizeof(tempTable->scope));
                memcpy(currentScope,tempTable->scope,sizeof(tempTable->scope));
                tempTable->next = NULL;   
            }
        }
}

void addList(char* lexeme, int binNo){
   if(symbolTable == NULL)
   {
      sTable* newEntry = (sTable *) malloc(sizeof(sTable));
      sTableEntry* newItem = (sTableEntry *)malloc(sizeof(sTableEntry));
      newItem->name = lexeme;
      newItem->scope = currentScope;
      newItem->pubpriv = currentPrivPub;
      newItem->binNo = binNo;
      newItem->type = -1;

      newEntry->item = newItem;
      newEntry->next = NULL;
      newEntry->prev = NULL;
      symbolTable = newEntry;
   }
   else
   {
         sTable* temp = symbolTable;
         while(temp->next != NULL){
            temp = temp->next;
         }
         sTable* newEntry = (sTable *) malloc(sizeof(sTable));
         sTableEntry* newItem = (sTableEntry *)malloc(sizeof(sTableEntry));
         newItem->name = lexeme;
         newItem->scope = currentScope;
         newItem->pubpriv = currentPrivPub;
         newItem->binNo = binNo;
         newItem->type = -1;

         newEntry->item = newItem;
         newEntry->next = NULL;
         newEntry->prev = temp;
         temp->next = newEntry;
   }
}

void printScope(void){
    scopeResolve* temp = scopeTable;

    cout << "\n Printing Scope Table \n";
    while(temp->next != NULL){
        cout << " Scope " << temp->scope << " -> ";
        temp = temp->next;
    }
    cout << " Scope " << temp->scope << " \n";
}


void printList(int binNo){
    sTable* temp = symbolTable;

    int type = -1;
    int num_print = 0;
    while(temp != NULL)
    {
        struct sTableEntry *s = temp->item;
        if( s->binNo == binNo) {
            type = s->type;
            if (s->type == 0) {
                printf("%s: int #\n", s->name);
            } else if (s->type == 1) {
                printf("%s: real #\n", s->name);
            } else if (s->type == 2) {
                printf("%s: bool #\n", s->name);
            }
            else{
                if(num_print != 0)
                    printf(", ");
                num_print++;

                printf("%s", s->name);
            }
        }
        temp = temp->next;
    }
    if(type == -1 && num_print != 0)
    {
        printf(": ? #\n");
    }
}


void printList(void){
    sTable* temp = symbolTable;

    for(int i=0; i <= currBinNo; i++)
        printList(i);
}


void deleteList(void){
    sTable* temp = symbolTable;
    
    if(temp!= NULL){
        while(temp->next != NULL && strcmp(temp->item->scope,currentScope) != 0){
            temp = temp->next;
        }
        if(strcmp(temp->item->scope,currentScope) == 0){
            if(strcmp(currentScope,"::") != 0){
            temp = temp->prev;
            temp->next = NULL;
            }else{
                temp = NULL;
            }
        }
    }
}


struct sTableEntry* find_symbol(char* name)
{
    sTable* temp = symbolTable;
    while(temp != NULL)
    {
        if( strcmp(temp->item->name, name) == 0)
        {
            return temp->item;
        }
        temp = temp->next;
    }
    return NULL;
}



void searchList(char* iD, int lR) // add an argument to accept a type
{
    bool found = false;
    
    sTable* temp = symbolTable;
    if(temp == NULL)
    {
        if(lR == 0){
            lResolve = (char *) malloc(1);
            memcpy(lResolve, "?", 1);
        }else{
            rResolve = (char *) malloc(1);
            memcpy(rResolve, "?",1);
        }
    }
    else
    {
        int count = 0;
        while(temp->next != NULL)
        {
            temp = temp->next;
            count++;
        }
        if(strcmp(temp->item->name, iD) == 0)
        {
                if(strcmp(temp->item->scope, currentScope) == 0 )
                {
                    found = true;
                    if(lR == 0){
                        lResolve = (char *) malloc(sizeof(currentScope));
                        memcpy(lResolve, currentScope, sizeof(currentScope));
                    }else{
                        rResolve = (char *) malloc(sizeof(currentScope));
                        memcpy(rResolve, currentScope, sizeof(currentScope));
                    }
                }
                else if(temp->item->pubpriv == 0)
                {
                    found = true;
                    if(lR == 0)
                    {
                        lResolve = (char *) malloc(sizeof(temp->item->scope));
                        memcpy(lResolve, temp->item->scope, sizeof(temp->item->scope));
                    }
                    else
                    {
                        rResolve = (char *) malloc(sizeof(temp->item->scope));
                        memcpy(rResolve, temp->item->scope, sizeof(temp->item->scope));
                    }
                    
                }
                else
                {
                    found = false;
                }
        }
        while(strcmp(temp->item->name, iD) != 0 || !found)
        {
            if(temp->prev == NULL && strcmp(temp->item->name, iD) != 0)
            {
                if(lR == 0)
                {
                    lResolve = (char *) malloc(1);
                    memcpy(lResolve, "?", 1);
                }
                else
                {
                    rResolve = (char *) malloc(1);
                    memcpy(rResolve, "?", 1);
                }
                found = false;
                
                break;
            }
            else
            {
                found = true;
            }
            if(strcmp(temp->item->name, iD) == 0)
            {
                if(strcmp(temp->item->scope, currentScope) == 0 )
                {
                    found = true;
                    if(lR == 0)
                    {
                        lResolve = (char *) malloc(sizeof(currentScope));
                        memcpy(lResolve, currentScope, sizeof(currentScope));
                    }
                    else
                    {
                        rResolve = (char *) malloc(sizeof(currentScope));
                        memcpy(rResolve, currentScope, sizeof(currentScope));
                    }
                    break;
                }
                else
                {
                    if (temp->item->pubpriv == 0) {
                        found = true;
                        if (lR == 0) {
                            lResolve = (char *) malloc(sizeof(temp->item->scope));
                            memcpy(lResolve, temp->item->scope, sizeof(temp->item->scope));
                        } else {
                            rResolve = (char *) malloc(sizeof(temp->item->scope));
                            memcpy(rResolve, temp->item->scope, sizeof(temp->item->scope));
                        }
                        break;
                    } else {
                        found = false;
                        temp = temp->prev;
                    }
                }
            }
            else
            {
                found = false;
                temp = temp->prev;
            }
        }
    }
    // if the third argument is a valid type --> while loop explore through all the variables in the list that are defined and update their pubpriv.
}

void updateSymbolTableType( int binNo, int type)
{
    sTable* temp = symbolTable;
    while(temp != NULL)
    {
        struct sTableEntry *s = temp->item;
        if( s->binNo == binNo)
        {
            s->type = type;
        }

        temp = temp->next;
    }
    return;
}

void updateSymbolTableBinNo( int binNo1, int binNo2)
{
    int min_BinNo = binNo1, max_BinNo = binNo2;

    if( binNo1 > binNo2 )
    {
        min_BinNo = binNo2;
        max_BinNo = binNo1;
    }

    sTable* temp = symbolTable;
    while(temp != NULL)
    {
        struct sTableEntry *s = temp->item;
        if( s->binNo == max_BinNo)
        {
            s->binNo = min_BinNo;
        }

        temp = temp->next;
    }
    return;
}

// parse var_list
int Parser::parse_varlist(void){
    token = lexer.GetToken();
    int tempI;
    
    char* lexeme = (char*)malloc(sizeof(token.lexeme)+1);
    memcpy(lexeme, (token.lexeme).c_str(), (token.lexeme).size()+1);
    struct sTableEntry* val = find_symbol(lexeme);
    if( val == NULL)
    {
        addList(lexeme, binNoToUse);

        val = find_symbol(lexeme);
    }
    binNoToUse = val->binNo;

    // Check First set of ID
    if(token.token_type == ID){
        token = lexer.GetToken();
        if(token.token_type == COMMA){
            //cout << "\n Rule Parsed: var_list -> ID COMMA var_list \n";
            tempI = parse_varlist();
        }else if(token.token_type == COLON){
            tempTokenType = lexer.UngetToken(token);
            //cout << "\n Rule Parsed: var_list -> ID \n";
        }else{
            cout << "\n Syntax Error \n";
            exit(0);
        }
    }else{
        cout << "\n Syntax Error \n";
        exit(0);
    }    
    return(0);
}

int Parser::parse_unaryOperator(void){
    token = lexer.GetToken();
    
    if(token.token_type == NOT){
        //cout << "\n Rule parsed: unary_operator -> NOT";
        return(2);
    }else{
        cout << "\n Syntax Error\n";
        exit(0);
    }
    return(0);
}

int Parser::parse_binaryOperator(void){
    token = lexer.GetToken();
    //keep track of the number of bin operations in binNo
    int type = -1;
    if(token.token_type == PLUS  ){
        ; //cout << "\n Rule parsed: binary_operator -> PLUS\n";
    }else if(token.token_type == MINUS ){
        ; //cout << "\n Rule pared: binary_operator -> MINUS \n";
    }else if(token.token_type == MULT){
        ; //cout << "\n Rule parsed: binary_operator -> MULT\n";
    }else if(token.token_type == DIV ){
        ; //cout << "\n Rule parsed: binary_operator -> DIV \n";
    }else if(token.token_type == GREATER){
        type = 2;
        ; //cout << "\n Rule parsed: binary_operator -> GREATER \n";
    }else if(token.token_type == LESS  ){
        type = 2;
        ; // cout << "\n Rule parsed: binary_operator -> LESS\n";
    }else if(token.token_type == GTEQ ){
        type = 2;
        ; // cout << "\n Rule parsed: binary_operator -> GTEQ \n";
    }else if(token.token_type == LTEQ){
        type = 2;
        ; // cout << "\n Rule parsed: binary_operator -> LTEQ\n";
    }else if(token.token_type == EQUAL ){
        ; // cout << "\n Rule parsed: binary_operator -> EQUAL \n";
    }else if(token.token_type == NOTEQUAL){
        type = 2;
        ; // cout << "\n Rule parsed: binary_operator -> NOTEQUAL \n";
    }else{
        cout << "\n Syntax Error \n";
        exit(0);
    }
    return(type);
}



int Parser::parse_primary(void){
    token = lexer.GetToken();

    int type = -1;
    if(token.token_type == ID  ){
        // search list for the token. If token available then return the type of the token. if not then add the token to the list
        // make its scope = "h" and make its type = -1;
        char* lexeme = (char*)malloc(sizeof(token.lexeme)+1);
        memcpy(lexeme, (token.lexeme).c_str(), (token.lexeme).size()+1);

        struct sTableEntry* val = find_symbol(lexeme);
        if( val == NULL)
        {
            addList(lexeme, binNoToUse);

            val = find_symbol(lexeme);
        }
        binNoToUse = val->binNo;
        type = val->type;

        ; //cout << "\n Rule parsed: primary -> ID\n";
    }else if(token.token_type == NUM ){
        type = 10;
        ; // cout << "\n Rule parsed: primary -> NUM \n";
    }else if(token.token_type == REALNUM){
        type = 11;
        ; // cout << "\n Rule parsed: primary -> REALNUM\n";
    }else if(token.token_type == TR ){
        type = 12;
        ; //cout << "\n Rule parsed: primary -> TRUE \n";
    }else if(token.token_type == FA){
        type = 12;
        ; // cout << "\n Rule parsed: primary -> FALSE \n";
    }else{
        cout << "\n Syntax Error \n";
        exit(0);
    }
    return(type);
}

int Parser::parse_expression(void){
    int tempI;
    int tempI1, tempI2;
    token = lexer.GetToken();
    
    if(token.token_type == ID || token.token_type == NUM || token.token_type == REALNUM || token.token_type == TR || token.token_type == FA ){
        ; // cout << "\n Rule parsed: expression -> primary \n";
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_primary();

        char* lexeme = (char*)malloc(sizeof(token.lexeme)+1);
        memcpy(lexeme, (token.lexeme).c_str(), (token.lexeme).size()+1);
        struct sTableEntry* val = find_symbol(lexeme);
        if(val != NULL && lvalue != NULL)
        {
            char* lexeme = (char*)malloc(sizeof(token.lexeme)+1);
            memcpy(lexeme, (token.lexeme).c_str(), (token.lexeme).size()+1);
            if(lvalue->type != 2)
                updateSymbolTableBinNo(val->binNo, lvalue->binNo);
        }
        if(tempI >= 10)
        {
            int new_type = tempI - 10;

            if(lvalue != NULL && lvalue->type != -1 && lvalue->type != new_type)
            {
                printf("TYPE MISMATCH %d C2\n", token.line_no );
                exit(0);
            }
            if(lvalue != NULL && new_type != 2)
                updateSymbolTableType(binNoToUse, new_type);

        }
        else if( lvalue != NULL && lvalue->type == -1 )
        {
            lvalue->type = tempI;
        }
        else if( lvalue != NULL && lvalue->type != -1 && lvalue->type != 2 && val != NULL && val->type == -1)
        {
            val->type = lvalue->type;
            tempI = lvalue->type;
        }

        return(tempI);
    }
    else if(token.token_type == PLUS || token.token_type == MINUS || token.token_type == MULT || token.token_type == DIV || token.token_type == GREATER
            || token.token_type == LESS || token.token_type == GTEQ || token.token_type == LTEQ || token.token_type == EQUAL || token.token_type == NOTEQUAL)
    {
        ; // cout << "\n Rule parsed: expression -> binary_operator expression expression \n";
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_binaryOperator();
        // in searchList and addList add an input argument that is the binNo
        // in each ID for which you do addList or searchList just update binNo.
        // if plus minus multiply divide
        if(token.token_type == PLUS || token.token_type == MINUS || token.token_type == MULT || token.token_type == DIV)
        {
            tempI1 = parse_expression();

            if( lvalue != NULL && lvalue->type != -1 && tempI1 == -1)
            {
                tempI1 = lvalue->type;
                updateSymbolTableType(binNoToUse, lvalue->type);
            }

            if( lvalue != NULL && lvalue->type == -1 && tempI1 != -1 && tempI1 != 2)
            {
                lvalue->type = tempI1;
                updateSymbolTableType(binNoToUse, tempI1);
            }


            tempI2 = parse_expression();

            if( lvalue != NULL && lvalue->type != -1 && tempI2 == -1)
            {
                tempI2 = lvalue->type;
                updateSymbolTableType(binNoToUse, lvalue->type);
            }

            if( lvalue != NULL && lvalue->type == -1 && tempI2 != -1 && tempI2 != 2)
            {
                lvalue->type = tempI2;
                tempI1 = tempI2;
                updateSymbolTableType(binNoToUse, tempI2);
            }

            if( lvalue != NULL && lvalue->type != -1 && tempI2 == -1)
            {
                tempI2 = lvalue->type;
                updateSymbolTableType(binNoToUse, lvalue->type);
            }

            if(tempI1 == -1 && tempI2 != -1 && tempI2 != 2)
                tempI1 = tempI2;

            if(statement_type == SWITCH && tempI1 == -1)
            {
                tempI1 = 0;
                updateSymbolTableType(binNoToUse, 0);
            }
            if(statement_type == SWITCH && tempI2 == -1)
            {
                tempI2 = 0;
                updateSymbolTableType(binNoToUse, 0);
            }
            if(tempI1 >= 10 || tempI2 >= 10) // one of the operands are primary type
            {
                int new_type = -1;
                if(tempI1 >= 10) new_type = tempI1 - 10;
                if(tempI2 >= 10) new_type = tempI2 - 10;
                if (tempI1 == -1 || tempI2 == -1)
                {
                    if(lvalue != NULL)
                         updateSymbolTableType(binNoToUse, new_type);
                }
                tempI1 = new_type;
                tempI2 = new_type;
            }

            if(statement_type == SWITCH && (tempI1 != 0 || tempI2 != 0))
            {
                printf("TYPE MISMATCH %d C5\n", token.line_no );
                exit(0);
            }
            if(tempI1 != tempI2)
            {
                if(statement_type == SWITCH )
                {
                    printf("TYPE MISMATCH %d C5\n", token.line_no);
                }
                else
                {
                    printf("TYPE MISMATCH %d C1\n", token.line_no);
                }
                exit(0);
            }


            if(lvalue != NULL && lvalue->type == -1)
            {
                lvalue->type = tempI1;
            }

            return(tempI1);
            //if(tempI1 != tempI2) type mismatch token.lineno C2 (this is not true if tempI1 == -1 or tempI2 == -1 )
        }
        else // GT LT GTE LTE
        {
            if(lvalue != NULL && lvalue->type == -1 && tempI == 2 && token.token_type != NOTEQUAL)
            {
                lvalue->type = 2;
            }
            if(lvalue != NULL && lvalue->type != tempI)
            {
                printf("TYPE MISMATCH %d C1\n", token.line_no );
                exit(0);
            }
            // if gt gteq ---------------
            tempI1 = parse_expression();
            tempI2 = parse_expression();


            if(tempI2 == -1 && (tempI1 == 0 or tempI1 == 1)) //xxxx1 change
            {
                updateSymbolTableType(binNoToUse, tempI1);
                tempI2 = tempI1;
            }

            if(tempI1 == -1 && (tempI2 == 0 or tempI2 == 1)) //xxxx1 change
            {
                updateSymbolTableType(binNoToUse, tempI2);
                tempI1 = tempI2;
            }



            if( tempI1 != tempI2 )
            {
                printf("TYPE MISMATCH %d C2\n", token.line_no );
                exit(0);
            }

           return(2);

            //if(tempI1 != tempI2) type mismatch token.lineno C2 (this is not true if tempI1 == -1 or tempI2 == -1 )

            // both tempI1 and tempI2 are -1
            // is lType -1? if so then do nothing
            // if not then it implies that lType is 0 1 or 2. Then check if parser_expression returned an ID or a INT NUm or REAL.
            // if parse expression returned from INT NUM REAL do nothing
            // else if it returned from an ID, then searchList for the ID update the type of ID to lType. --> search for all IDs which have binNo same as binNo of the current ID
            // for each such ID if its type is -1 then change their types to lType, if not type mismatch token.line_no C2

            // if only one of tempI1 or tempI2 is -1
            // if tempI1 is -1, did tempI1 return from ID? if so then searchList for ID and make its type = tempI2 --> search for all IDs which have binNo same as binNo of the current ID
            // for each such ID if its type is -1 then change their types to lType, if not type mismatch token.line_no C2
        }
    }
    else if(token.token_type == NOT)
    {
        ; //cout << "\n Rule parsed: expression -> unary_operator expression \n";
        tempTokenType = lexer.UngetToken(token);
        if(lvalue != NULL && lvalue->type == -1)
            lvalue->type = 2;

        tempI1 = parse_unaryOperator();
        tempI2 = parse_expression();
        if(tempI2 == 12)
        {
            tempI2 = 2;
            if(lvalue->type == -1)
            {
                lvalue->type = 2;
            }
        }

        if( lvalue != NULL && ( lvalue->type != 2 || tempI2 != 2))
        {
            printf("TYPE MISMATCH %d C3\n", token.line_no );
            exit(0);
        }
        return(2);
        //if parse expression returns an ID and type of that ID is -1 then make it 2 by using search list
        // if tempI2 != 2 and != -1 then Type mismatch token.line_no C3????
    }else{
        cout << "\n Syntax Error \n";
        exit(0);
    }
    return(0);
}


int Parser::parse_assstmt(void){
    int tempI;
    token = lexer.GetToken();

    if(token.token_type == ID){
        // search for the token in the searchList --> the token is available, leftType = type of the available token
        // it is not available in the searchList, add the token to the list, make its type = -1; make its scope = "h".
        char* lexeme = (char*)malloc(sizeof(token.lexeme)+1);
        memcpy(lexeme, (token.lexeme).c_str(), (token.lexeme).size()+1);

        struct sTableEntry* lval = find_symbol(lexeme);
        if( lval != NULL)
        {
            lvalue = lval;
        }
        else
        {
            addList(lexeme, binNoToUse);

            lval = find_symbol(lexeme);
            lvalue = lval;
        }
        binNoToUse = lval->binNo;
        token = lexer.GetToken();
        if(token.token_type == EQUAL){
             token = lexer.GetToken();  
             if(token.token_type == ID || token.token_type == NUM || token.token_type == REALNUM || token.token_type == TR || token.token_type == FA || token.token_type == PLUS || token.token_type == MINUS || token.token_type == MULT || token.token_type == DIV || token.token_type == LESS || token.token_type == GREATER || token.token_type == GTEQ || token.token_type == LTEQ || token.token_type == EQUAL || token.token_type == NOTEQUAL || token.token_type == NOT){
                tempTokenType = lexer.UngetToken(token);
                tempI = parse_expression();
                if(tempI >= 10)
                    tempI = tempI - 10;


                if(lvalue != NULL && lvalue->type != tempI )
                {
                    printf("TYPE MISMATCH %d C1\n", token.line_no);
                    exit(0);
                    if(lvalue->type != -1 && tempI != -1) {
                        printf("TYPE MISMATCH %d C1\n", token.line_no);
                        exit(0);
                    }
                }
                if(lvalue != NULL & lvalue->type != -1 && lvalue->type != 2)
                {
                    updateSymbolTableType(lvalue->binNo, lvalue->type);
                }
                //rType right type of an assigment tempI.
                //check for C1. if ltype == rtype then fine if not then TYPE MISMATCH token.line_no C1
                // if any one of lType or rType is -1 then should not throw type mismatch. 
                // if lType != -1 && rType is -1 then you search for left ID token to extract its type. searchList should return type. 
                // you have to use search list again with the right token to update the right token's type to lType 
                token = lexer.GetToken();
                if(token.token_type == SEMICOLON){
                    currBinNo++;
                    binNoToUse = currBinNo;
                    lvalue = NULL;

                    ; // cout << "\n Rule parsed: assignment_stmt -> ID EQUAL expression SEMICOLON \n";
                }else{
                    cout << "\n HI Syntax Error " << token.token_type << " \n";
                    exit(0);
                 }
             }else{
                 cout << "\n Syntax Error \n";
                 exit(0);
             }  
        }else{
            cout << "\n Syntax Error \n";
            exit(0);
        }  
    }else{
        cout << "\n Syntax Error \n";
        exit(0);
    }
    return(0);
}

int Parser::parse_case(void){
    int tempI;
    token = lexer.GetToken();

    if(token.token_type == CASE ){
        token = lexer.GetToken();
        if(token.token_type == NUM){
            token = lexer.GetToken();
            if(token.token_type == COLON){
                ; // cout << "\n Rule parsed: case -> CASE NUM COLON body";
                tempI = parse_body();
            }else{
                cout << "\n Syntax Error \n";
                exit(0);
                return -99;
            }
        }else{
            cout << "\n Syntax Error \n";
            exit(0);
            return -99;
        }
    }else{
        cout << "\n Syntax Error \n";
        exit(0);
        return -99;
    }
    return 0;
}

int Parser::parse_caselist(void){
    
    int tempI;
    token = lexer.GetToken();
    if(token.token_type == CASE){
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_case();
        token = lexer.GetToken();
        if(token.token_type == CASE){
            tempTokenType = lexer.UngetToken(token);
            ; // cout << "\n Rule parsed: case_list -> case case_list \n";
            tempI = parse_caselist();
        }else if(token.token_type == RBRACE){
            tempTokenType = lexer.UngetToken(token);
            ; // cout << "\n Rule parsed: case_list -> case  \n";
        }
    }
    return(0);
}


int Parser::parse_switchstmt(void){
    int tempI;
    
    token = lexer.GetToken();
    if(token.token_type == SWITCH){
        token = lexer.GetToken();
        if(token.token_type == LPAREN){
            tempI = parse_expression();

            if(tempI == -1)
            {
                char* lexeme = (char*)malloc(sizeof(token.lexeme)+1);
                memcpy(lexeme, (token.lexeme).c_str(), (token.lexeme).size()+1);
                struct sTableEntry* val = find_symbol(lexeme);
                if(val != NULL)
                {
                    val->type = 0;
                    updateSymbolTableType(val->binNo, val->type);
                }
            }
            // if tempI != INT then throw type error
            // else if tempI = -1 ==> parse_expresssion retunred an ID, then go and change using searchList the type of ID to 1.
            token = lexer.GetToken();
            if(token.token_type == RPAREN){
                token = lexer.GetToken();
                if(token.token_type == LBRACE){
                    tempI = parse_caselist();
                    token = lexer.GetToken();
                    if(token.token_type == RBRACE){
                        ; // cout << "\n Rule parsed: switch_stmt -> SWITCH LPAREN expression RPAREN LBRACE case_list RBRACE \n";
                    }else{
                        cout << "\n Syntax Error \n";
                        exit(0);
                    }   
                }else{
                    cout << "\n Syntax Error \n";
                    exit(0);
                }
                
            }else{
                cout << "\n Syntax Error \n";
                exit(0);
            }
        }else{
            cout << "\n Syntax Error \n";
            exit(0);
        }    
    }else{
        cout << "\n Syntax Error \n";
        exit(0);
    }
    return(0);
}


int Parser::parse_whilestmt(void){
    int tempI;
    token = lexer.GetToken();

    if(token.token_type == WHILE){
        token = lexer.GetToken();
        if(token.token_type == LPAREN){
            tempI = parse_expression();
            if(tempI != 2 && tempI != -1)
            {
                printf("TYPE MISMATCH %d C4\n", token.line_no );
                exit(0);
            }


            // if tempI != bool then throw type error
            // else if tempI = -1 ==> parse_expresssion retunred an ID, then go and change using searchList the type of ID to 2.
            token = lexer.GetToken();
            if(token.token_type == RPAREN){
                ; // cout << "\n Rule parsed: whilestmt -> WHILE LPAREN expression RPAREN body \n";
                currBinNo++;
                binNoToUse++;
                tempI = parse_body();
            }else{
                cout << "\n Syntax Error \n";
                exit(0);
            }
        }else{
            cout << "\n Syntax Error \n";
            exit(0);
        }    
    }else{
        cout << "\n Syntax Error \n";
        exit(0);
    }
    return(0); // xxx1 changed from return(0)
}

int Parser::parse_ifstmt(void){
    int tempI;
    token = lexer.GetToken();

    if(token.token_type == IF){
        token = lexer.GetToken();
        if(token.token_type == LPAREN){
            tempI = parse_expression();
            if(tempI >= 10)
                tempI = tempI - 10;
            if(tempI != 2 && tempI != -1)
            {
                printf("TYPE MISMATCH %d C4\n", token.line_no );
                exit(0);
            }
            if(tempI == -1)
            {
                updateSymbolTableType(binNoToUse, 2);
            }
            // if tempI != bool then throw type error
            // else if tempI = -1 ==> parse_expresssion retunred an ID, then go and change using searchList the type of ID to 2.

            token = lexer.GetToken();
            if(token.token_type == RPAREN){
                ; // cout << "\n Rule parsed: ifstmt -> IF LPAREN expression RPAREN body \n";
                currBinNo++;
                binNoToUse++;
                lvalue = NULL;
                tempI = parse_body();
            }else{
                cout << "\n Syntax Error \n";
                exit(0);
            }
        }else{
            cout << "\n Syntax Error \n";
            exit(0);
        }    
    }else{
        cout << "\n Syntax Error \n";
        exit(0);
    }
    return(0);
}

int Parser::parse_stmt(void){
    int tempI;
    token = lexer.GetToken();

    if(token.token_type == ID){
        tempTokenType = lexer.UngetToken(token);
        ; // cout << "\n Rule parsed: stmt -> assignment_stmt \n";
        tempI = parse_assstmt();        
    }else if(token.token_type == IF){
        currBinNo++;
        binNoToUse = currBinNo;
        lvalue = NULL;

        tempTokenType = lexer.UngetToken(token);
        ; //cout << "\n Rule parsed: stmt -> if_stmt";
        tempI = parse_ifstmt();
    }else if(token.token_type == WHILE){
        currBinNo++;
        binNoToUse = currBinNo;
        lvalue = NULL;

        tempTokenType = lexer.UngetToken(token);
        ; // cout << "\n Rule parsed: stmt -> while_stmt";
        tempI = parse_whilestmt();
    }else if(token.token_type == SWITCH){
        tempTokenType = lexer.UngetToken(token);
        ; // cout << "\n Rule parsed: stmt -> switch_stmt";
        tempI = parse_switchstmt();
    }else{
        cout << "\n Syntax Error \n";
        exit(0);
    }
    return(0);
}

int Parser::parse_stmtlist(void){
    token = lexer.GetToken();
    int tempI;

    if(token.token_type == ID || token.token_type == IF || token.token_type == WHILE || token.token_type == SWITCH){
        statement_type = token.token_type;
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_stmt();
        token = lexer.GetToken();
        if(token.token_type == ID || token.token_type == IF || token.token_type == WHILE || token.token_type == SWITCH){
            tempTokenType = lexer.UngetToken(token);
            ; // cout << "\n Rule Parsed: stmt_list -> stmt stmt_list \n";
            tempI = parse_stmtlist();
        }else if (token.token_type == RBRACE){
            tempTokenType = lexer.UngetToken(token);
            ; //cout << "\n Rule parsed: stmt_list -> stmt \n";
        }
    }else{
        cout << "\n Syntax Error \n";
        exit(0);
    }
    return(0);
}



int Parser::parse_body(void){
    token = lexer.GetToken();
    int tempI;
    
        if(token.token_type == LBRACE){
            tempI = parse_stmtlist();
            token = lexer.GetToken();
            if(token.token_type == RBRACE){
                ; //  cout << "\n Rule parsed: body -> LBRACE stmt_list RBRACE \n";
                return(0);
            }else{
                cout << "\n Syntax Error\n ";
                exit(0);
            }    
        }else if(token.token_type == END_OF_FILE){
            tempTokenType = lexer.UngetToken(token);
        return(0);
        }else{
            cout << "\n Syntax Error \n ";
            exit(0);
        }
}




// parse scope end
int Parser::parse_typename(void){
    token = lexer.GetToken();
    int type = -1;
    if(token.token_type == INT || token.token_type == REAL || token.token_type == BOO)
    {
        switch(token.token_type)
        {
            case INT:
                    type = 0;
                    break;
            case REAL:
                    type = 1;
                    break;
            case BOO:
                    type = 2;
                    break;

        }
        ; // cout  << "\n Rule parse: type_name -> " << token.token_type << "\n";
    }

    return(type);// if Int ret 0 if float ret 1 if bool ret 2
}

int Parser::parse_vardecl(void){
    int tempI;
    token = lexer.GetToken();

    if(token.token_type == ID){
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_varlist();
        token = lexer.GetToken();
        if(token.token_type == COLON){
            tempI = parse_typename();
            updateSymbolTableType(binNoToUse, tempI);
            //use the searchList to update the types of variables that are already in the symbolTable

            token = lexer.GetToken();
            if(token.token_type == SEMICOLON)
            {
                currBinNo++;
                binNoToUse = currBinNo;
                lvalue = NULL;
                //cout << "\n Rule parsed: var_decl -> var_list COLON type_name SEMICOLON";
            }else{
                cout << "\n Syntax Error \n";
                exit(0);
            }
        } else{
            cout << "\n Syntax Error \n";
            exit(0);
        } 
    }else{
        cout << "\n Syntax Error \n";
        exit(0);
    }
    return(0);
}

int Parser::parse_vardecllist(void){
    int tempI;    
    token = lexer.GetToken();

    while(token.token_type == ID){
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_vardecl();
        token = lexer.GetToken();
        /*
        if(token.token_type != ID){
            cout << "\n Rule Parsed: var_decl_list -> var_decl \n";
        }else{
            cout << "\n Rule Parsed: var_decl_list -> var_decl var_decl_list \n";
        }
         */
    }
    tempTokenType = lexer.UngetToken(token);
    return(0);
}


string global = "::";
// parse global vars
int Parser::parse_globalVars(void){
    token = lexer.GetToken();
    int tempI;
    
    //check first set of var_list SEMICOLON
    if(token.token_type == ID){
        tempTokenType = lexer.UngetToken(token);
        currentPrivPub = 0;
        //cout << "\n Rule parsed: globalVars -> var_decl_list \n";
        tempI = parse_vardecllist();
    } else{
        cout << "Syntax Error";
        exit(0);
    }
    return(0);
}


int Parser::parse_program(void){
    token = lexer.GetToken();
    int tempI;

    while (token.token_type != END_OF_FILE)
    {
        // Check first set of global_vars scope
            if(token.token_type == ID){
            tempTokenType = lexer.UngetToken(token);
            //cout << "\n Rule parsed: program -> global_vars scope \n";
            tempI = parse_globalVars();
            tempI = parse_body();
            }else if(token.token_type == LBRACE ){
                tempTokenType = lexer.UngetToken(token);
                //cout << "\n Rule parsed: global_vars -> epsilon \n";
                tempI = parse_body();
            }
            else if(token.token_type == IF)
            {
                tempTokenType = lexer.UngetToken(token);
                //cout << "\n Rule parsed: global_vars -> epsilon \n";
                tempI = parse_ifstmt();
            }
            else if(token.token_type == END_OF_FILE)
            {
                return(0);
            }else
            {
                cout << "\n Syntax Error\n";
                exit(0);
            }
        token = lexer.GetToken();
    }
    printList();
    return 0;
}

char null[] = "NULL";
int main()
{
    int i;
    Parser* parseProgram = new Parser();
    i = parseProgram->parse_program();
   // cout << "\n End of Program \n";
}
