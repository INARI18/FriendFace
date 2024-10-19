#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_LEN 50
#define INT_MAX 20000000

// vertex
typedef struct user {
    int id;
    char *username;
    bool visited;
}user;

// edge
typedef struct link {
    int weight;
    user *source;
    user *destination;
    struct link *next;
}link;

// graph
typedef struct FriendFace {
    int num_users;
    user *users;
    link **adj;
}FriendFace;
 
typedef struct queue {
    int *users;
    int front;
    int rear;
    int size;
} queue;

// create graph
FriendFace *createFriendFace(int num) {
    FriendFace *graph = (FriendFace*)malloc(sizeof(FriendFace));
    if (graph == NULL) {
        printf("Error allocating memory to graph\n");
        return NULL;
    }
    
    if(num <= 0) {
        graph->users = NULL;
        graph->adj = NULL;
        graph->num_users = 0;
        return graph;
    }
    graph->num_users = 0;
    graph->users = (user*)malloc(sizeof(user)*num);
    graph->adj = (link**)malloc(sizeof(link*)*num);

    if(graph->users == NULL) {
        printf("Error allocating memory to users\n");
        free(graph);
        return NULL;
    }

    if(graph->adj == NULL) {
        printf("Error allocating memory to adj\n");
        free(graph->users);
        free(graph);
        return NULL;
    }

    for(int i = 0; i < num; i++) {
        graph->adj[i] = NULL;
    }
    return graph;
}

// utility function 
int getUserID(FriendFace *graph, const char *username) {
    for (int i = 0; i < graph->num_users; i++) {
        if (strcmp(graph->users[i].username, username) == 0) {
            return i;
        }
    }
    return -1;
}
// utility function
char *tolowerUsername(const char *username) {
    char *username_copy = (char*)malloc(sizeof(char)*(strlen(username)+1));
    if (username_copy == NULL) {
        printf("Error allocating memory for username_copy\n");
        return NULL;
    }
    strcpy(username_copy, username);
    for (int i = 0; i < (int)strlen(username_copy); i++) {
        username_copy[i] = tolower(username_copy[i]);
    }
    return username_copy;
}

// add vertex (user)
void addNewUser(FriendFace *graph, const char *name) {
    char *name_copy = tolowerUsername(name);
    if (name_copy == NULL) return;

    int verified = getUserID(graph, name_copy);
    if(verified != -1) {
        printf("Username already exists\n");
        free(name_copy);
        return;
    }

    // realloc adj list
    link **temp_adj = (link**)realloc(graph->adj, sizeof(link*)*(graph->num_users+1));
    if (temp_adj == NULL) {
        printf("Error reallocating memory for adjacency list\n");
        free(name_copy);
        return;
    }
    graph->adj = temp_adj;

    // realloc users 
    user *temp = (user*)realloc(graph->users, sizeof(user)*(graph->num_users+1));
    if (temp == NULL) {
        printf("Error reallocating memory for users\n");
        free(name_copy);
        return;
    }
    graph->users = temp;
    graph->adj[graph->num_users] = NULL;

    user *newUser = (user*)malloc(sizeof(user));
    if (newUser == NULL) {
        printf("Error allocating memory for newUser\n");
        free(name_copy);
        return;
    }
    newUser = &graph->users[graph->num_users];
    newUser->username = strdup(name_copy);
    if (newUser->username == NULL) {
        printf("Error allocating memory for username\n");
        free(name_copy);
        return;
    }

    newUser->id = graph->num_users;
    newUser->visited = false;
    graph->num_users++;
    //printf("User %s added\n", newUser->username);
    free(name_copy);
}

// add edge (friendship)
void addLink(FriendFace *graph, const char *username_source, const char *username_destination, int weight) {
    int source_id = -1, destination_id = -1;
    if (strlen(username_source) == 0 || strlen(username_destination) == 0) {
        printf("Invalid username\n");
        return;
    }
    char *username_source_copy = tolowerUsername(username_source);
    char *username_destination_copy = tolowerUsername(username_destination);
    source_id = getUserID(graph, username_source_copy);
    destination_id = getUserID(graph, username_destination_copy);

    if (source_id == -1 || destination_id == -1 || source_id == destination_id) {
        printf("Invalid link\n");
        return;
    }

    // check if link already exists
    link *current = graph->adj[source_id];
    while(current != NULL) {
        if(current->destination == &graph->users[destination_id]) {
            printf("Link already exists\n");
            free(username_source_copy);
            free(username_destination_copy);
            return;
        }
        current = current->next;
    }
    
    // add link
    link *newLink = (link*)malloc(sizeof(link));
    if(newLink == NULL) {
        printf("Error allocating memory to newLink\n");
        free(username_source_copy);
        free(username_destination_copy);
        return;
    }

    newLink->source = &graph->users[source_id];
    newLink->destination = &graph->users[destination_id];
    newLink->weight = weight;
    newLink->next = graph->adj[source_id];
    graph->adj[source_id] = newLink;
    free(username_source_copy);
    free(username_destination_copy);
    //printf("Link %s -> %s added\n", graph->users[source_id].username, graph->users[destination_id].username);
    return;
}

void updateLink(FriendFace *graph, const char *username_source, const char *username_destination, int weight) {
    int source_id = -1, destination_id = -1;
    source_id = getUserID(graph, username_source);
    destination_id = getUserID(graph, username_destination);

    // check if link exists and update it
    link *current = graph->adj[source_id];
    while(current != NULL) {
        if(current->destination == &graph->users[destination_id]) {
            current->weight = weight;
            return;
        }
        current = current->next;
    }
    printf("Link not found\n");
    return;
}

// create graph, add users from csv and add links
FriendFace *FriendFaceCSV(const char *filename) {
    FILE *file = fopen(filename, "r");
    if(file == NULL) {
        printf("Error opening CSV file\n");
        return NULL;
    }
    printf("CSV file opened\n");

    int num_users = 0;
    char line[MAX_LEN];
    while (fgets(line, MAX_LEN, file)) {
        num_users++;
    }
    rewind(file); // back to the beginning of the file

    FriendFace *graph = createFriendFace(num_users);
    if (graph == NULL) {
        printf("Error creating graph with CSV file\n");
        fclose(file);
        return NULL;
    }

    // add new users
    while(fgets(line, MAX_LEN, file)) {
        line[strcspn(line, "\n")] = 0; // remove newline
        addNewUser(graph, line);
    }
    fclose(file);

    // add new links
    for (int i = 0; i < graph->num_users; i++) {
        for (int j = 0; j < graph->num_users; j++) {
            if (i != j && rand()%75 == 0) { // 1,3% chance of being friends
                /*int random;
                if(rand()%3 == 0) { // 33% chance of having a negative weight
                    int value = rand()%15;
                    random = rand()%25+1;
                    random =- value;
                } else {
                    random = rand()%25+1;
                }*/
                addLink(graph, graph->users[i].username, graph->users[j].username, rand()%30+1);
            }
        }
    } 
    return graph;
}

void createFileGraphviz(FriendFace *graph) {
    FILE *file = fopen("graph.dot", "w");
    if(graph->users == NULL || graph->adj == NULL) {
        printf("Error!");
    }

    if(file == NULL) {
        printf("Error opening file\n");
        return;
    }

    fprintf(file, "digraph FriendFace {\n");
    for(int i = 0; i < graph->num_users; i++) {
        link *current = graph->adj[i];
        while (current != NULL) {
            fprintf(file, "\"%s\" -> \"%s\" [label=\"%d\"];\n", graph->users[i].username, current->destination->username, current->weight);
            current = current->next;
        }
    }

    fprintf(file, "}");
    fclose(file);
    printf("\nFile graph.dot created\n");
}

void printGraph(FriendFace *graph) {
    printf("\nGraph:\n");
    for (int i = 0; i < graph->num_users; i++) {
        link *current = graph->adj[i];
        while (current != NULL) {
            printf("%s -[%d]-> %s\n", graph->users[i].username, current->weight, current->destination->username);
            current = current->next;
        }
    }
    printf("\n");
}

// DFS algorithm #0
void dfs(FriendFace *graph, int start_user, int *result, int *index) {
    graph->users[start_user].visited = true;
    result[(*index)++] = start_user;

    link *current = graph->adj[start_user];
    while (current != NULL) {
        if (!graph->users[current->destination->id].visited) {
            dfs(graph, current->destination->id, result, index);
        }
        current = current->next;
    }
}

// init queue
queue *createQueue(int size) {
    queue *q = (queue*)malloc(sizeof(queue));
    if (q == NULL) {
        printf("Error allocating memory for queue\n");
        return NULL;
    }
    q->users = (int*)malloc(sizeof(int)*size);
    q->front = -1;
    q->rear = -1;
    q->size = size;

    return q;
}
// add user to queue
void enqueue(queue *q, int user) {
    if (q->rear == q->size) {
        printf("Queue is full\n");
        return;
    }
    // first element
    if (q->front == -1) {
        q->front = 0;
    }
    // add user
    q->rear++;
    q->users[q->rear] = user;
}
// remove user from queue
int dequeue(queue *q) {
    if (q->front == -1) {
        printf("Queue is empty\n");
        return -1;
    }
    // remove user 
    int user = q->users[q->front];
    q->front++;
    // reset queue
    if (q->front > q->rear) {
        q->front = q->rear = -1;
    }
    return user;
}
// BFS algorithm (#1)
void bfs(FriendFace *graph, int start_user, int *result, int *index) {
    queue *q = createQueue(graph->num_users);
    graph->users[start_user].visited = true;
    enqueue(q, start_user);
    
    while (q->front != -1) {
        int user = dequeue(q);
        result[(*index)++] = user;

        link *current = graph->adj[user];
        while (current != NULL) {
            if (!graph->users[current->destination->id].visited) {
                graph->users[current->destination->id].visited = true;
                enqueue(q, current->destination->id);
            }
            current = current->next;
        }
    }
    free(q->users);
    free(q);
}

// search in friends network
int* friendsNetwork(FriendFace *graph, const char *username, int *size, int algorithm) {
    int start_user_id;
    start_user_id = getUserID(graph, username);

    if (start_user_id == -1) {
        printf("User not found\n");
        *size = 0;
        return NULL;
    }
    // reset visited
    for (int i = 0; i < graph->num_users; i++) {
        graph->users[i].visited = false;
    }

    int *result = (int*)malloc(graph->num_users*sizeof(int));
    int index = 0;
    // choose algorithm
    if (algorithm == 0) {
        dfs(graph, start_user_id, result, &index);
    } 
    else if (algorithm == 1) {
        bfs(graph, start_user_id, result, &index);
    } 
    
    else {
        printf("Invalid algorithm number\n");
        *size = 0;
        return NULL;
    }
    *size = index;
    return result;
}

// print djikstra path
void pathDj(FriendFace *graph, int prev[], int j) {
    if (prev[j] == -1)
        return;
    pathDj(graph, prev, prev[j]);
    printf(" -> %s", graph->users[j].username);
}
// Djikstra algorithm
void Djikstra(FriendFace *graph, const char *start_user, const char *end_user) {
    int *dist = (int*)malloc(graph->num_users * sizeof(int));
    int *prev = (int*)malloc(graph->num_users * sizeof(int));
    
    int start_user_id, end_user_id;
    start_user_id = getUserID(graph, start_user);
    end_user_id = getUserID(graph, end_user);
    // reset visited
    for (int i = 0; i < graph->num_users; i++) {
        dist[i] = INT_MAX;
        prev[i] = -1;
        graph->users[i].visited = false;
    }
    
    dist[start_user_id] = 0;
    for (int i = 0; i < graph->num_users; i++) {
        int min = INT_MAX;
        int min_index = -1;
        for (int j = 0; j < graph->num_users; j++) {
            if (!graph->users[j].visited && dist[j] < min) {
                min = dist[j];
                min_index = j;
            }
        }
        if (min_index == -1) {
            break;
        }
        graph->users[min_index].visited = true;
        link *current = graph->adj[min_index];
        while (current != NULL) {
            if (!graph->users[current->destination->id].visited && dist[min_index] + current->weight < dist[current->destination->id]) {
                dist[current->destination->id] = dist[min_index] + current->weight;
                prev[current->destination->id] = min_index;
            }
            current = current->next;
        }
    }
    printf("\nDjisktra =\n");
    printf("From %s to %s: %d\n", graph->users[start_user_id].username, graph->users[end_user_id].username, dist[end_user_id]);
    printf("Path: %s", graph->users[start_user_id].username);
    pathDj(graph, prev, end_user_id);
    printf("\n\n");

    free(dist);
    free(prev);
}

// create graphviz file with DFS
void graphvizDFS(FriendFace *graph, const char *username) {
    FILE *file = fopen("graph_DFS.dot", "w");
    if (file == NULL) {
        printf("Error opening graph_DFS file\n");
        return;
    }

    int size = 0;
    int *network = friendsNetwork(graph, username, &size, 0);
    // print users (first to last visited)
    printf("DFS = %s: ", username);
    for (int i = 0; i < size; i++) {
        printf("%s, ", graph->users[network[i]].username);
    }

    fprintf(file, "digraph DFS {\n");
    for (int i = 0; i < size; i++) {
        int user_id = network[i];
        link *current = graph->adj[user_id];
        while (current != NULL) {
            if (graph->users[current->destination->id].visited) {
                fprintf(file, "\"%s\" -> \"%s\" [label=\"%d\"];\n",graph->users[user_id].username, graph->users[current->destination->id].username, current->weight);
            }
            current = current->next;
        }
    }
    fprintf(file, "}");
    fclose(file);
    free(network);
    printf("\nFile graph_DFS.dot created\n\n");
}

// create graphviz file with BFS
void graphvizBFS(FriendFace *graph, const char *username) {
    FILE *file = fopen("graph_BFS.dot", "w");
    if (file == NULL) {
        printf("Error opening graph_BFS file\n");
        return;
    }

    int size = 0;
    int *network = friendsNetwork(graph, username, &size, 1);
    printf("BFS = %s: ", username);
    for (int i = 0; i < size; i++) {
        printf("%s, ", graph->users[network[i]].username);
    }

    fprintf(file, "digraph BFS {\n");
    for (int i = 0; i < size; i++) {
        int user_id = network[i];
        link *current = graph->adj[user_id];
        while (current != NULL) {
            if (graph->users[current->destination->id].visited) {
                fprintf(file, "\"%s\" -> \"%s\" [label=\"%d\"];\n",graph->users[user_id].username, graph->users[current->destination->id].username, current->weight);
            }
            current = current->next;
        }
    }
    fprintf(file, "}");
    fclose(file);
    free(network);
    printf("\nFile graph_BFS.dot created\n\n");
}

// free graph memory
void freeFriendFace(FriendFace *graph) {
    for (int i = 0; i < graph->num_users; i++) {
        free(graph->users[i].username);
        link *current = graph->adj[i];
        while (current) {
            link *temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(graph->users);
    free(graph->adj);
    free(graph);
}

int main(void) {
    srand(time(NULL));
    // [test with CSV file]:
    FriendFace *graph = FriendFaceCSV("users.csv");
    printGraph(graph);
    
    graphvizDFS(graph, "kadabra");
    graphvizBFS(graph, "pikachu");

    addNewUser(graph, "electrode");
    addNewUser(graph, "hitmonlee");
    addLink(graph, "electrode", "hitmonlee", 16);
    addLink(graph, "hitmonlee", "kadabra", 7);
    addLink(graph, "electrode", "pikachu", 3);

    createFileGraphviz(graph);
    printGraph(graph);
    freeFriendFace(graph);

    // [manual test]:
    /*FriendFace *graph2 = createFriendFace(4);
    addNewUser(graph2, "Krabby");
    addNewUser(graph2, "squirtle");
    addNewUser(graph2, "Mankey");
    addNewUser(graph2, "Kadabra");

    addLink(graph2, "Krabby", "squirtle", 8);
    addLink(graph2, "Krabby", "Mankey", 5);
    addLink(graph2, "squirtle", "kadabra", 10);
    addLink(graph2, "squirtle", "mankey", 2);
    addLink(graph2, "Mankey", "Kadabra", 4);
    addLink(graph2, "Kadabra", "mankey", 3);
    addLink(graph2, "Kadabra", "squirtle", 40);

    createFileGraphviz(graph2);
    printGraph(graph2);
    updateLink(graph2, "krabby", "squirtle", 25);
    updateLink(graph2, "squirtle", "mankey", 7);
    printGraph(graph2);
    createFileGraphviz(graph2);

    Djikstra(graph2, "krabby", "kadabra");
    graphvizDFS(graph2, "krabby");
    graphvizBFS(graph2, "krabby");

    addNewUser(graph2, "pikachu");
    addNewUser(graph2, "charmander");
    addNewUser(graph2, "bulbasaur");
    
    addLink(graph2, "pikachu", "squirtle", 13);
    addLink(graph2, "squirtle", "krabby", 5);
    addLink(graph2, "krabby", "kadabra", 9);
    addLink(graph2, "charmander", "Krabby", 55);
    addLink(graph2, "charmander", "bulbasaur", 10);
    addLink(graph2, "bulbasaur", "Mankey", 27);

    createFileGraphviz(graph2);
    printGraph(graph2);
    freeFriendFace(graph2);*/
    
    return 0;
}