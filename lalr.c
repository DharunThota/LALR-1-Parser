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
    int merged;
}ItemSet;

typedef struct {
    int from;  
    char symbol;     
    int to;    
} Transition;

typedef struct{
    char symbol;
    int state;
    char action[10];
    int next;
} Action;

Production prod[MAX];
ItemSet items[MAX];
Transition transitions[MAX * 5];

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
        if(items[i].merged) continue;
        printf("Item %d - %d\n", i, items[i].index);
        printf("Size: %d\n", items[i].size);
        for(int j=0; j<items[i].size; j++){
            printf("%c->%s, %s - %d\n", items[i].item[j].lhs, items[i].item[j].rhs, items[i].item[j].lookahead, items[i].item[j].dot);
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

// void closure(ItemSet *temp){
//     int i, k = temp->size;
//     for(i=0; i<k; i++){
//         if(isNonTerminal(temp->item[i].rhs[temp->item[i].dot])){
//             Item t;
//             for(int j=1; j<=n; j++){
//                 if(prod[j].lhs == temp->item[i].rhs[temp->item[i].dot]){
//                     t.lhs = prod[j].lhs;
//                     strcpy(t.rhs, prod[j].rhs);
//                     t.dot = 0;

//                     char set[10];
//                     set[0] = '\0';
//                     if(temp->item[i].dot+1 >= strlen(temp->item[i].rhs)){
//                         strcpy(t.lookahead, temp->item[i].lookahead);
//                     }
//                     else{
//                         first(set, temp->item[i].rhs[temp->item[i].dot+1]);
//                         strcpy(t.lookahead, set);
//                     }
//                     temp->item[k] = t;
//                     k++;
//                     temp->size++;
//                 }
//             }
//         }
//     }
//     temp->merged = 0;
//     temp->index = n_items;
//     items[n_items] = *temp;
//     n_items++;
// }

void closure(ItemSet *temp){
    int i = 0; // Use a loop variable to traverse the item set
    while (i < temp->size) { // Iterate over the current items in the set
        if(isNonTerminal(temp->item[i].rhs[temp->item[i].dot])) { // Non-terminal after dot
            Item t;
            for (int j = 1; j <= n; j++) { // Look for productions with this non-terminal as LHS
                if (prod[j].lhs == temp->item[i].rhs[temp->item[i].dot]) {
                    // Prepare new item based on production
                    t.lhs = prod[j].lhs;
                    strcpy(t.rhs, prod[j].rhs);
                    t.dot = 0;

                    // Compute lookaheads
                    char set[10];
                    set[0] = '\0';
                    if (temp->item[i].dot + 1 >= strlen(temp->item[i].rhs)) {
                        strcpy(t.lookahead, temp->item[i].lookahead); // Use lookahead from the item
                    } else {
                        first(set, temp->item[i].rhs[temp->item[i].dot + 1]);
                        strcpy(t.lookahead, set);
                    }

                    // Check if this item is already present
                    int found = 0;
                    for (int k = 0; k < temp->size; k++) {
                        if (temp->item[k].lhs == t.lhs && strcmp(temp->item[k].rhs, t.rhs) == 0 && strcmp(temp->item[k].lookahead, t.lookahead) == 0) {
                            found = 1;
                            break;
                        }
                    }
                    if (!found) { // Add the new item if not already present
                        temp->item[temp->size++] = t;
                    }
                }
            }
        }
        i++;
    }
    temp->merged = 0;
    temp->index = n_items;
    items[n_items] = *temp;
    n_items++;
}


// void goTo(ItemSet* src, char symbol) {
//     // Initialize the destination item set
//     ItemSet dst;
//     dst.size = 0;
    
//     for (int i = 0; i < src->size; i++) {
//         // Check if the dot is before the symbol
//         if (src->item[i].rhs[src->item[i].dot] == symbol) {
//             Item newItem;
//             newItem.lhs = src->item[i].lhs;
//             strcpy(newItem.rhs, src->item[i].rhs);
//             strcpy(newItem.lookahead, src->item[i].lookahead);
//             newItem.dot = src->item[i].dot + 1;

//             dst.item[dst.size++] = newItem;
//         }
//     }

//     // Apply closure on the destination item set
//     closure(&dst);

//     // Add the transition to the transition table
//     transitions[n_transitions].from = src->index; 
//     transitions[n_transitions].symbol = symbol;
//     transitions[n_transitions].to = dst.index; 
//     n_transitions++;
// }

void goTo(ItemSet* src, char symbol) {
    // Initialize the destination item set
    ItemSet dst;
    dst.size = 0;
    
    for (int i = 0; i < src->size; i++) {
        // Check if the dot is before the symbol
        if (src->item[i].rhs[src->item[i].dot] == symbol) {
            Item newItem;
            newItem.lhs = src->item[i].lhs;
            strcpy(newItem.rhs, src->item[i].rhs);
            strcpy(newItem.lookahead, src->item[i].lookahead);
            newItem.dot = src->item[i].dot + 1;

            dst.item[dst.size++] = newItem;
        }
    }

    // Apply closure only if the destination set is non-empty
    if (dst.size > 0) {
        closure(&dst);

        // Add the transition to the transition table
        transitions[n_transitions].from = src->index; 
        transitions[n_transitions].symbol = symbol;
        transitions[n_transitions].to = dst.index; 
        n_transitions++;
    }
}


int areCoresEqual(ItemSet *set1, ItemSet *set2) {
    if (set1->size != set2->size) return 0;
    for (int i = 0; i < set1->size; i++) {
        if (set1->item[i].lhs != set2->item[i].lhs ||
            strcmp(set1->item[i].rhs, set2->item[i].rhs) != 0 ||
            set1->item[i].dot != set2->item[i].dot) {
            return 0;
        }
    }
    return 1;
}

void mergeLookaheads(ItemSet *dst, ItemSet *src) {
    for (int i = 0; i < src->size; i++) {
        for (int j = 0; j < strlen(src->item[i].lookahead); j++) {
            char lookahead = src->item[i].lookahead[j];
            if (!strchr(dst->item[i].lookahead, lookahead)) {
                strncat(dst->item[i].lookahead, &lookahead, 1);
            }
        }
    }
}

// Merge equivalent states
void mergeStates() {
    for (int i = 0; i < n_items; i++) {
        for (int j = i + 1; j < n_items; j++) {

            // Check if item sets have the same core
            if (areCoresEqual(&items[i], &items[j])) {
                // Merge the lookahead sets
                mergeLookaheads(&items[i], &items[j]);

                // Redirect transitions pointing to items[j] to items[i]
                for (int k = 0; k < n_transitions; k++) {
                    if (transitions[k].to == items[j].index) {
                        transitions[k].to = items[i].index;
                    }
                }

                items[j].merged = 1;
            }
        }
    }
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
    mergeStates();
}

void createParseTable(){
    return;
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
    prod[1].lhs = 'A';
    strcpy(prod[1].rhs, "BB");
    prod[2].lhs = 'B';
    strcpy(prod[2].rhs, "aB");
    prod[3].lhs = 'B';
    strcpy(prod[3].rhs, "b");
    char* start = "A";
    //scanf(" %c", start);
    prod[0].lhs = 'Z';
    strcpy(prod[0].rhs, start);
    //closure(0);
    init();
    printItems();
    printTransitions();
}
