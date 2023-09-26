#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ALPA_CARDINALITY 26
#define uint unsigned int

typedef struct Transition Transition;

typedef struct StateMachine
{
    uint alpha_card;     // cardinality of the alphabet
    uint states_card;    // cardinality of the states set
    uint initial;        // initial state
    uint finals_card;    // cardinality of the final states set
    uint *finals;        // final states
    uint transitions_num;// number of transitions
    Transition *transition;
} StateMachine;

struct Transition
{
    uint from;
    uint to;
    char a;
    Transition* next;
};


void
free_sm(StateMachine sm)
{
    // free final states (array)
    free(sm.finals);

    // free transitions (list)
    Transition *t = sm.transition;
    while (t)
    {
        Transition *next = t->next;
        free(t);
        t = next;
    }
}


// Read deterministic state machine from file
int 
load_sm_from_file(FILE *file, StateMachine *sm)
{    
    int r;
    memset(sm, 0, sizeof(*sm));

    // read alphabet's cardinality
    r = fscanf(file, "%u", &sm->alpha_card);
    if (r <= 0)
    {
        printf("Cardinality of alphabet (int) expected. (pos. %ld)\n", ftell(file));
        return -1;
    }
    if (sm->alpha_card > MAX_ALPA_CARDINALITY)
    {
       printf("Cardinality %u > %d.\n", sm->alpha_card, MAX_ALPA_CARDINALITY); 
       return -1;
    }

    // read states number
    r = fscanf(file, "%u", &sm->states_card);
    if (r <= 0)
    {
        printf("Cardinality of states set (int) expected. (pos. %ld)\n", ftell(file));
        return -1;
    }

    // read initial state
    r = fscanf(file, "%u", &sm->initial);
    if (r <= 0)
    {
        printf("Initial state (int) expected. (pos. %ld)\n", ftell(file));
        return -1;
    }
    if (sm->initial >= sm->states_card)
    {
        printf("Initial state %d >= %d (pos. %ld)\n", sm->initial, sm->states_card, ftell(file));
        return -1;
    }

    // read final states
    r = fscanf(file, "%u", &sm->finals_card);
    if (r <= 0)
    {
        printf("Number of final states (int) expected. (pos. %ld)\n", ftell(file));
        return -1;
    }
    sm->finals = malloc(sizeof(uint) * sm->finals_card);
    for (uint i = 0; i < sm->finals_card; ++i)
    {
        // read final state
        r = fscanf(file, "%u", &sm->finals[i]);
        if (r <= 0)
        {
            printf("Cannot read final state #%d (pos. %ld)\n", i + 1, ftell(file));
            free_sm(*sm);
            return -1;
        }
        if (sm->finals[i] >= sm->states_card)
        {
            printf("Final state %d >= %d (pos. %ld)\n", sm->finals[i], sm->states_card - 1, ftell(file));
            free_sm(*sm);
            return -1;
        }
    }

    // read transitions
    sm->transitions_num = 0;
    Transition* curr_transition = NULL;
    for (;;)
    {
        uint s0, s1;
        char a;

        r = fscanf(file, "%u %c %u", &s0, &a, &s1);
        if (r <= 0)
        {
            break;
        }
        if (r < 3)
        {
            printf("Expected transition rule \"s0 a s1\" (int char int) (pos. %ld)\n", ftell(file));
            if (curr_transition)
                printf(
                    "(Last processed transitions: %d %c %d)\n", 
                    curr_transition->from,
                    curr_transition->a,
                    curr_transition->to
                );
            else
                printf("(This is the first transition)\n");
            free_sm(*sm);
            return -1;
        }

        if (s0 >= sm->states_card || s1 >= sm->states_card)
        {
            printf(
                "Transition rule %u %c %u references invalid state. Ensure both are < %u (pos. %ld)\n", 
                s0,
                a, 
                s1,
                sm->states_card,
                ftell(file)
            );
            free_sm(*sm);
            return -1;
        }

        if (a >= 'a' + sm->alpha_card)
        {
            printf(
                "Transition rule %u %c %u references out-of-alphabet character (pos. %ld)\n", 
                s0,
                a, 
                s1,
                ftell(file)
            );
            free_sm(*sm);
            return -1;
        }

        // read transition
        Transition *new_transition = malloc(sizeof(Transition));
        memset(new_transition, 0, sizeof(Transition));
        new_transition->from = s0;
        new_transition->to = s1;
        new_transition->a = a;

        // check if this transition make s. m. non-deterministic
        int add = 1;
        for (Transition *t = sm->transition; t; t = t->next)
        {
            if (t->from != new_transition->from || t->a != new_transition->a)
                continue;
            
            // transition is either duplicate or ambiguity
            if (t->to != new_transition->to)
            {
                printf(
                    "Transition rule %u %c %u makes state machine non-deterministic (pos. %ld)\n", 
                    new_transition->from,
                    new_transition->a, 
                    new_transition->to,
                    ftell(file)
                );
                printf(
                    "It colides with rule %u %c %u.\n", 
                    t->from,
                    t->a, 
                    t->to
                );
                free(new_transition);
                free_sm(*sm);
                return -1;
            } else {
                printf(
                    "Warning: Transition rule %u %c %u is a duplicate (pos. %ld)\n", 
                    new_transition->from,
                    new_transition->a, 
                    new_transition->to,
                    ftell(file)
                );
                free(new_transition);
                new_transition = NULL;
            }
        }

        if (!new_transition)
            continue;

        if (curr_transition)
            curr_transition->next = new_transition;
        if (!sm->transition)
            sm->transition = new_transition;
        curr_transition = new_transition;
        ++sm->transitions_num;
    }

    return 0;
}

void
show_sm(StateMachine *sm)
{
    printf(
        "State Machine\n"
        "Alphabet card.:        %u\n"
        "States number:         %u\n"
        "Initial state:         %u\n"
        "Final states number:   %u\n"
        "Final states:          "
        , 
        sm->alpha_card,
        sm->states_card,
        sm->initial,
        sm->finals_card
    );

    for (uint i = 0; i < sm->finals_card; ++i)
    {
        printf("%d ", sm->finals[i]);
    }

    printf(
        "\n"
        "Transitions number:    %u\n"
        "Transitions:\n",
        sm->transitions_num
    );

    for (Transition *curr = sm->transition; curr; curr = curr->next)
    {
        printf("\t%u %c %u\n", curr->from, curr->a, curr->to);
    }
}


int acceptable_word_exists(const char *subword, const StateMachine *sm);
int acceptable_word_exists_given_state(const char *subword, const StateMachine *sm, const uint state);
int is_finish_reachable_from_state(const StateMachine *sm, const uint state);
int _is_finish_reachable_from_state_helper(const StateMachine *sm, const uint state, int *state_visited);
int is_state_reachable_from_start(const StateMachine *sm, const uint state);
int _is_state_reachable_from_start_helper(const StateMachine *sm, const uint state, const uint current_state, int *state_visited);

// Givent subword w0 check if there exists w = w1+w0+w2
// so that w is acceptable by state machine sm
int 
acceptable_word_exists(const char *subword, const StateMachine *sm)
{
    // Algorithm:
    //  if subword is empty:
    //  check if any final state is reachable from initial state;
    //  else:
    //  for all sequence of states S0...SN which produce word sm
    //      check if S0 is reachable from s0 — initial state
    //      if not — continue
    //      else check if any final state is reachable form SN
    //      if not — continue
    //      else — return 1

    if (strlen(subword) == 0)
    {
        return is_finish_reachable_from_state(sm, sm->initial);
    }

    for (Transition *t = sm->transition; t; t = t->next)
    {
        if (t->a != subword[0])
            continue;
        
        uint S0 = t->from;

        // check if there exists sequence S0...SN which produces subword
        // such that exists final state reachable from SN
        if (!acceptable_word_exists_given_state(subword, sm, S0)) 
            continue;
        
        // check if S0 is reachable
        if (is_state_reachable_from_start(sm, S0))
            return 1;
    }
    return 0;
}

int 
acceptable_word_exists_given_state(const char *subword, const StateMachine *sm, const uint state)
{
    if (subword[0] == 0)
    {
        // no subword guidence anymore, 
        // check if there is a path to final state from state 
        return is_finish_reachable_from_state(sm, state);
    }

    // try all transitions from state
    for (Transition *t = sm->transition; t; t = t->next)
    {
        if (t->from != state || t->a != subword[0])
            continue;
        if (acceptable_word_exists_given_state(subword + 1, sm, t->to))
            return 1;
    }
    return 0;
}

int 
is_finish_reachable_from_state(const StateMachine *sm, const uint state)
{
    int state_visited[sm->states_card];
    memset(state_visited, 0, sizeof(state_visited));

    return _is_finish_reachable_from_state_helper(sm, state, state_visited);
}


int 
_is_finish_reachable_from_state_helper(const StateMachine *sm, const uint state, int *state_visited)
{
    state_visited[state] = 1;   // mark current state as visited

    // check if state is final
    for (int i = 0; i < sm->finals_card; ++i)
    {
        if (sm->finals[i] == state)
            return 1;
    }

    // try all transitions from this state
    for (Transition *t = sm->transition; t; t = t->next)
    {
        if (t->from != state || state_visited[t->to])
            continue;
        if (_is_finish_reachable_from_state_helper(sm, t->to, state_visited))
            return 1;
    }

    return 0;
}


int 
is_state_reachable_from_start(const StateMachine *sm, const uint state)
{
    int state_visited[sm->states_card];
    memset(state_visited, 0, sizeof(state_visited));

    return _is_state_reachable_from_start_helper(sm, state, sm->initial, state_visited);
}


int 
_is_state_reachable_from_start_helper(const StateMachine *sm, const uint state, const uint current_state, int *state_visited)
{
    state_visited[current_state] = 1;   // mark current state as visited

    // check if current_state is state
    if (current_state == state)
        return 1;

    // try all transitions from this state
    for (Transition *t = sm->transition; t; t = t->next)
    {
        if (t->from != current_state || state_visited[t->to])
            continue;
        if (_is_state_reachable_from_start_helper(sm, state, t->to, state_visited))
            return 1;
    }

    return 0;
}


void print_usage(const char *prog_name)
{
    printf("Usage: %s state_machine_file_path word_to_check [--verbose]\n", prog_name);
}


void print_help(const char *prog_name)
{
    print_usage(prog_name);
    printf("\nParameters:\n");
    printf("\tword_to_check   empty word '-' or any word in A.\n");
    printf("\tstate_machine_file_path     path to state machine file\n");

    printf("\nFalgs:\n");
    printf("\t--verbose       print additional execution information\n");
    
    printf("\nStructure of state machine file:\n");
    printf("\t||A||           cardinality of alphabet\n");
    printf("\t||S||           number of states\n");
    printf("\ts0              initial state\n");
    printf("\t||F|| {Fs}      number of final states and final states\n");
    printf("\t{ s a s' }      define transition from state s to s' by character a\n\n");
}


int 
main(int argc, const char *argv[])
{
    if (argc == 2 && strcmp(argv[1], "-h") == 0)
    {
        print_help(argv[0]);
        return 0;
    }

    // parse arguments
    if (argc != 3 && (argc != 4 || strcmp(argv[3], "--verbose")))
    {
        print_usage(argv[0]);
        printf("Use -h to get help.\n");
        return -1;
    }
    const char* file_path = argv[1];
    const char* word_to_check = strcmp(argv[2], "-") ? argv[2] : "";
    const int verbose = (argc == 4);


    FILE* file = fopen(file_path, "r");

    if (!file)
    {
        printf("Cannot open the file\n");
        return -1;
    }

    // load state machine
    StateMachine sm;
    int r = load_sm_from_file(file, &sm);
    fclose(file);
    if (r)
    {
        return -1;
    }

    if (verbose)
        show_sm(&sm);
        printf("\nChecking if there exists w = w1+w0+w2, where w0 = \"%s\", such that w is acceptable...\n", word_to_check);

    printf("Answer: ");
    if (acceptable_word_exists(word_to_check, &sm))
    {
        printf("yes.\n");
    } else {
        printf("no.\n");
    }

    free_sm(sm);

    return 0;
}
