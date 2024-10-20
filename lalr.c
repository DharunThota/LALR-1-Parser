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
    char action[10];
    int next;
} Action;

Production prod[MAX];
ItemSet items[MAX];
Transition transitions[MAX * 5];
Action action[MAX][MAX];
char term[MAX];

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
    printf("Number of items: %d\n", n_items);
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

void printItem(ItemSet *set){
    printf("Item %d\n", set->index);
    printf("Size: %d\n", set->size);
    for(int j=0; j<set->size; j++){
        printf("%c->%s, %s - %d\n", set->item[j].lhs, set->item[j].rhs, set->item[j].lookahead, set->item[j].dot);
    }
    printf("\n");
}

void printTransitions(){
    printf("Numer of transitions: %d\n", n_transitions);
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

void closure(ItemSet *temp) {
    int i = 0; // Loop over items in the current set
    while (i < temp->size) { 
        if (isNonTerminal(temp->item[i].rhs[temp->item[i].dot])) {
            // Iterate over all productions to find matching non-terminals
            for (int j = 0; j <= n; j++) {
                if (prod[j].lhs == temp->item[i].rhs[temp->item[i].dot]) {
                    Item t;
                    // Create a new item based on the production
                    t.lhs = prod[j].lhs;
                    strcpy(t.rhs, prod[j].rhs);
                    t.dot = 0;  // Dot starts at 0 for the new item
                    
                    // Compute lookaheads
                    char set[10];
                    set[0] = '\0';
                    if (temp->item[i].dot + 1 >= strlen(temp->item[i].rhs)) {
                        strcpy(t.lookahead, temp->item[i].lookahead); // Inherit the lookahead from the current item
                    } else {
                        first(set, temp->item[i].rhs[temp->item[i].dot + 1]);
                        strcpy(t.lookahead, set);
                    }

                    // Check if the new item is already present in the set
                    int found = 0;
                    for (int k = 0; k < temp->size; k++) {
                        // Compare by lhs, rhs, lookahead, and dot position
                        if (temp->item[k].lhs == t.lhs && 
                            strcmp(temp->item[k].rhs, t.rhs) == 0 && 
                            temp->item[k].dot == t.dot) {  // Now also compare the dot position
                            
                            // Add any new lookahead symbols if they are not already present
                            for (int m = 0; t.lookahead[m] != '\0'; m++) {
                                if (!strchr(temp->item[k].lookahead, t.lookahead[m])) {
                                    strncat(temp->item[k].lookahead, &t.lookahead[m], 1);
                                }
                            }
                            found = 1;
                            break;
                        }
                    }

                    // If the item is not found, add it as a new item
                    if (!found) {
                        temp->item[temp->size++] = t;
                    }
                }
            }
        }
        i++;
    }

    // Update set meta-data after closure
    temp->merged = 0;
    temp->index = n_items;
    //printItem(temp);
    items[n_items] = *temp;
    n_items++;
}

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
        mergeStates();
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
            if(items[i].item[j].dot < strlen(items[i].item[j].rhs) && items[i].merged == 0){
                char next = items[i].item[j].rhs[items[i].item[j].dot];
                goTo(&items[i], next);
            }
        }
    }
}

void updateTransitions(){
    for(int i=0; i<n_transitions; i++){
        for(int j=0; j<n_items; j++){
            //printf("f-%d t-%d i-%d j-%d\n", transitions[i].from, transitions[i].to , items[j].index, j);
            if(transitions[i].from == items[j].index){
                transitions[i].from = j;
                //printf("updated from\n");
            }
            if(transitions[i].to == items[j].index){
                transitions[i].to = j;
                //printf("updated to\n");
            }
        }
        
    }
}

void removeMergedItems(){
    ItemSet temp[MAX];
    int j = 0;
    for(int i=0; i<n_items; i++){
        if(items[i].merged == 0){
            //items[i].index = j;
            temp[j++] = items[i];
        }
    }
    n_items = j;
    memset(items, 0, sizeof(items));
    for(int i=0; i<n_items; i++){
        items[i] = temp[i];
    }
    updateTransitions();
}

void getTermNonTerm(){
    term[0] = '\0';
    for(int i=1; i<=n; i++){
        for(int k=0; prod[i].rhs[k]!='\0'; k++){
            if(!isNonTerminal(prod[i].rhs[k])){
                addToSet(term, prod[i].rhs[k]);
            }
        }
    }
    addToSet(term, '$');
    for(int i=1; i<=n; i++){
        addToSet(term, prod[i].lhs);
    }
}

int getIndex(char c){
    for(int i=0; term[i]!='\0'; i++){
        if(term[i] == c){
            return i;
        }
    }
}

int getProdIndex(Item item){
    for(int i=1; i<=n; i++){
        if(prod[i].lhs == item.lhs && strcmp(prod[i].rhs, item.rhs) == 0){
            return i;
        }
    }
}

void printActionTable(char *term, int numStates) {
    int colWidth = 10;  // Fixed width for each column
    printf("\nAction Table:\n");

    // Print the header row with terminals and non-terminals
    printf("State\t");
    for (int i = 0; term[i] != '\0'; i++) {
        printf("%c\t", term[i]);
    }
    printf("\n");

    // Separator for header and body
    for (int i = 0; i <= strlen(term); i++) {
        printf("-------");
    }
    printf("\n");

    // Print each state's actions
    for (int state = 0; state < numStates; state++) {
        // Print state number
        printf("%d\t", state);

        // Print the actions for each terminal/non-terminal
        for (int i = 0; term[i] != '\0'; i++) {
            if (strcmp(action[state][i].action, "") == 0) {
                // Empty cell if no action
                printf(" \t");
            } else if (isNonTerminal(term[i])) {
                // If the symbol is a non-terminal, just print the state number for "goto" actions
                printf("%d\t", action[state][i].next);
            } else if (strcmp(action[state][i].action, "shift") == 0) {
                // Shift action for terminals (no gap between "s" and state number)
                printf("S%d\t", action[state][i].next);
            } else if (strcmp(action[state][i].action, "reduce") == 0) {
                // Reduce action for terminals (no gap between "r" and production number)
                printf("r%d\t", action[state][i].next);
            } else if (strcmp(action[state][i].action, "accept") == 0) {
                // Accept action for terminals
                printf("acc\t");
            }
        }
        printf("\n");
    }
    printf("\n");
}

void createActionTable(){
    getTermNonTerm();

    memset(action, 0, sizeof(action));

    //add shift actions
    for(int i=0; i<n_transitions; i++){
        Action a;
        int index = getIndex(transitions[i].symbol);
        a.next = transitions[i].to;
        strcpy(a.action, "shift");
        //check for conflicts
        action[transitions[i].from][index] = a;
    }
    
    //add reduce actions
    for(int i=0; i<n_items; i++){
        if(items[i].size == 1 && items[i].item[0].dot == strlen(items[i].item[0].rhs)){
            Item item = items[i].item[0];
            for(int j=0; j<strlen(item.lookahead); j++){
                int index = getIndex(item.lookahead[j]);
                Action a;
                strcpy(a.action, "reduce");
                if(item.lhs == 'Z'){
                    strcpy(a.action, "accept");
                }
                else{
                    int p = getProdIndex(item);
                    a.next = p;
                }
                //check for conflicts
                if(strcmp(action[i][index].action, "") != 0){
                    printf("Conflict at state %d, symbol %c\n", i, term[index]);
                    printf("Existing: %s %d\n", action[i][index].action, action[i][index].next);
                    printf("New: %s %d\n", a.action, a.next);
                    exit(0);
                }
                action[i][index] = a;
            }
        }
    }
    printActionTable(term, n_items);
}

void printParseState(char *stack, int top, char *str, int lookahead) {
    printf("Stack: ");
    for (int i = 0; i <= top; i++) {
        printf("%c", stack[i]);
    }
    printf("\tInput: ");
    for (int i = lookahead; str[i] != '\0'; i++) {
        printf("%c", str[i]);
    }
    printf("\tAction: ");
    int index = getIndex(str[lookahead]);
    printf("%s %d", action[stack[top] - '0'][index].action, action[stack[top] - '0'][index].next);

    printf("\n");
}

int parse(char *str){
    char stack[MAX];
    int top = 0;
    int lookahead = 0;
    int len = strlen(str);
    str[len++] = '$';
    str[len] = '\0';
    stack[top] = '$';
    stack[++top] = '0';

    printf("Parsing:\n");
    while(lookahead < len){
        printParseState(stack, top, str, lookahead);
        char t = stack[top];
        char symbol = str[lookahead];
        Action a;
        if(isdigit(t)){
            int index = getIndex(symbol);
            a = action[t - '0'][index];
            //printf("%s %d\n", a.action, a.next);
            if(strcmp(a.action, "shift") == 0){
                stack[++top] = str[lookahead];
                stack[++top] = a.next + '0';
                lookahead++;
            }
            else if(strcmp(a.action, "reduce") == 0){
                int pop_len = 2 * strlen(prod[a.next].rhs);
                for(int i=0; i<pop_len; i++){
                    top--;
                }
                stack[++top] = prod[a.next].lhs;
            }
            else if(strcmp(a.action, "accept") == 0){
                return 1;
            }
            else{
                return 0;
            }
        }
        else if(isNonTerminal(t)){
            int prev = top - 1;
            int index = getIndex(t);
            stack[++top] = action[stack[prev] - '0'][index].next + '0';
        }
    }
}

void lalr(char *str){
    init();
    removeMergedItems();
    // printItems();
    // printTransitions();
    createActionTable();
    if(parse(str)){
        printf("\nString accepted\n");
    }
    else{
        printf("\nString not accepted\n");
    }
}

int main(){
    printf("Enter the number of productions: ");
    scanf("%d", &n);
    printf("Enter the productions: \n");
    for(int i = 1; i <= n; i++){
        scanf(" %c->%s", &prod[i].lhs, prod[i].rhs);
    }
    
    //test productions
    // n = 3;
    // prod[1].lhs = 'A';
    // strcpy(prod[1].rhs, "BB");
    // prod[2].lhs = 'B';
    // strcpy(prod[2].rhs, "aB");
    // prod[3].lhs = 'B';
    // strcpy(prod[3].rhs, "b");
    // char* start = "A";

    // prod[1].lhs = 'S';
    // strcpy(prod[1].rhs, "AB");
    // prod[2].lhs = 'A';
    // strcpy(prod[2].rhs, "a");
    // prod[3].lhs = 'B';
    // strcpy(prod[3].rhs, "b");
    // char* start = "S";

    char *start;
    printf("Enter start symbol: ");
    scanf(" %c", start);
    prod[0].lhs = 'Z';
    strcpy(prod[0].rhs, start);

    char str[MAX];
    //strcpy(str, "abab");
    printf("Enter the string to parse: ");
    scanf("%s", str);
    
    lalr(str);
    return 0;
}
