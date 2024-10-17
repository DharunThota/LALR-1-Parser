/*CS22B1083 Dharun Thota*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX 20

typedef struct{
    char lhs;
    char rhs[MAX];
}Production;

typedef struct{
    char lhs;
    char rhs[MAX];
    int dot;
    char lookahead[MAX];
} Item;

typedef struct{
    Item item[MAX];
    int size;
    int index;
}ItemSet;

typedef struct {
    int from;  
    char symbol;     
    int to;    
} Transition;

Production prod[MAX];
ItemSet items[MAX];
Transition transitions[100];

int n;
int n_items = 0;
int n_transitions = 0;

int isNonTerminal(char c){
    return isupper(c);
}

void addToSet(char *set, char c){
    int i;
    for(i=0; set[i]!='\0'; i++){
        if(set[i] == c){
            return;
        }
    }
    set[i] = c;
    set[i+1] = '\0';
}

char *first(char *set, char c){
    char temp[10];
    temp[0] = '\0';
    if(!isNonTerminal(c)){
        addToSet(set, c);
        return set;
    }
    for(int i=0; i<=n; i++){
        if(prod[i].lhs == c){
            if(prod[i].rhs[0] == prod[i].lhs) continue;
            first(temp, prod[i].rhs[0]);
            for(int k=0;temp[k]!='\0';k++){
                addToSet(set, temp[k]);
            }
        }
    }
    return set;
}

void printItems(){
    for(int i=0; i<n_items; i++){
        printf("Item %d - %d\n", i, items[i].index);
        printf("Size: %d\n", items[i].size);
        for(int j=0; j<items[i].size; j++){
            printf("%c->%s, %s\n", items[i].item[j].lhs, items[i].item[j].rhs, items[i].item[j].lookahead);
        }
        printf("\n");
    }
}

void printTransitions(){
    for(int i=0; i<n_transitions; i++){
        printf("%d, %c -> %d\n", transitions[i].from, transitions[i].symbol, transitions[i].to);
    }
}

void print(){
    int i;
    for(i = 0; i <= n; i++){
        printf("%c->%s\n", prod[i].lhs, prod[i].rhs);
    }
}

void closure(ItemSet *temp){
    int i, k = temp->size;
    for(i=0; i<k; i++){
        //printf("%c->%s, %s %d\n", temp->item[i].lhs, temp->item[i].rhs, temp->item[i].lookahead, temp->item[i].dot);
        if(isNonTerminal(temp->item[i].rhs[temp->item[i].dot])){
            Item t;
            //printf("Non terminal %c\n", temp->item[i].rhs[temp->item[i].dot]);
            for(int j=1; j<=n; j++){
                if(prod[j].lhs == temp->item[i].rhs[temp->item[i].dot]){
                    //printf("%s\n", prod[j].rhs);
                    t.lhs = prod[j].lhs;
                    strcpy(t.rhs, prod[j].rhs);
                    t.dot = 0;
                    //calculate first of beta

                    char set[10];
                    set[0] = '\0';
                    // printf("j+1 %d\n", j+1);
                    // printf("%d\n", i);
                    if(temp->item[i].dot+1 >= strlen(temp->item[i].rhs)){
                        //printf("Lookahead %s\n", temp->item[i].lookahead);
                        strcpy(t.lookahead, temp->item[i].lookahead);
                    }
                    else{
                        //printf("First %c\n", temp->item[i].rhs[temp->item[i].dot+1]);
                        first(set, temp->item[i].rhs[temp->item[i].dot+1]);
                        //printf("First %s\n", set);
                        strcpy(t.lookahead, set);
                    }
                    //printf("closure lookahead %s\n", t.lookahead);
                    temp->item[k] = t;
                    k++;
                    temp->size++;
                }
            }
        }
    }
    temp->index = n_items;
    items[n_items] = *temp;
    n_items++;
}

void goTo(ItemSet* src, char symbol) {
    // Initialize the destination item set
    //printItems();
    ItemSet dst;
    dst.size = 0;
    
    // Iterate over items in the source item set
    for (int i = 0; i < src->size; i++) {
        // Check if the dot is before the symbol
        if (src->item[i].rhs[src->item[i].dot] == symbol) {
            // Create a new item with the dot moved forward
            //printf("lookahead %s\n", src->item[i].lookahead);
            Item newItem;
            newItem.lhs = src->item[i].lhs;
            strcpy(newItem.rhs, src->item[i].rhs);
            strcpy(newItem.lookahead, src->item[i].lookahead);
            newItem.dot = src->item[i].dot + 1;

            //printf("New item: %c->%s, %s\n", newItem.lhs, newItem.rhs, newItem.lookahead);

            // Add the new item to the destination item set
            dst.item[dst.size++] = newItem;
        }
    }

    // Apply closure on the destination item set
    closure(&dst);

    // Add the transition to the transition table
    transitions[n_transitions].from = src->index;  // Find the index of the source item set
    transitions[n_transitions].symbol = symbol;
    transitions[n_transitions].to = dst.index;  // Find or add the destination item set
    n_transitions++;
}


void init(){
    ItemSet temp;
    temp.item[0].lhs = 'Z';
    strcpy(temp.item[0].rhs, prod[0].rhs);
    temp.item[0].dot = 0;
    strcpy(temp.item[0].lookahead, "$");
    temp.size = 1;

    closure(&temp);
    for(int i=0; i<n_items; i++){
        for(int j=0; j<items[i].size; j++){
            if(items[i].item[j].dot < strlen(items[i].item[j].rhs)){
                char next = items[i].item[j].rhs[items[i].item[j].dot];
                goTo(&items[i], next);
            }
        }
    }
}



int main(){
    // printf("Enter the number of productions: ");
    // scanf("%d", &n);
    // printf("Enter the productions: \n");
    // for(int i = 1; i <= n; i++){
    //     scanf(" %c->%s", &prod[i].lhs, prod[i].rhs);
    // }
    // printf("Enter start symbol: ");
    n = 3;
    prod[1].lhs = 'S';
    strcpy(prod[1].rhs, "AB");
    prod[2].lhs = 'A';
    strcpy(prod[2].rhs, "a");
    prod[3].lhs = 'B';
    strcpy(prod[3].rhs, "b");
    char* start = "S";
    //scanf(" %c", start);
    prod[0].lhs = 'Z';
    strcpy(prod[0].rhs, start);
    //closure(0);
    init();
    printItems();
    printTransitions();
}
