#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>

#include "tophat/api/client.h"
#include "tophat/devices/digital_switch.h"
#include "tophat/devices/neopixel.h"

#define HEADLAMP_DEVICE_NAME "headlamp"
#define NEOPIXEL_DEVICE_NAME "neopixels"
#define NUM_MINERS 8

static FILE* socket_fp;
PyGILState_STATE gil_state;
static PyObject *tophat_client;

struct merchant {
    unsigned long num_deposits;
    unsigned long total_silver;
    unsigned long total_gold;
    unsigned long total_diamonds;
};

struct miner {
    char name[25];
    unsigned char depth;
    unsigned short silver;
    unsigned short gold;
    unsigned short diamonds;
};

struct miner *miners[NUM_MINERS] = {NULL};
size_t miner_index = 0;

struct merchant *merchants[NUM_MINERS] = {NULL};

char *read_line() {
    unsigned long line_size = 1024;
    char *line = malloc(line_size);
    memset(line, 0x0, line_size);
    getline(&line, &line_size, socket_fp);
    if (line[strlen(line)-1] == '\n') {
        line[strlen(line)-1] = 0x0;
    }
    return line;
}

int create_miner() {
    struct miner *new_miner = malloc(sizeof(struct miner));
    memset(new_miner, 0x0, sizeof(struct miner));

    fprintf(socket_fp, "Gold-Tooth Mike: Now, what'd you say y'er name was again?\n");
    char *line = read_line();
    strncpy(new_miner->name, line, sizeof(new_miner->name) - 1);
    free(line);

    int new_miner_index = miner_index;
    miners[new_miner_index] = new_miner;
    miner_index = (miner_index + 1) % NUM_MINERS;
    return new_miner_index;
}

int select_miner() {
    fprintf(socket_fp, "Gold-Tooth Mike: Ho, you there! You certainly seem a familiar fella...\n");
    fprintf(socket_fp, "Gold-Tooth Mike: S'pose my memry ain't what it used to be though, which miner are you?\n");
    for (size_t i = 0; i < NUM_MINERS; i++) {
        if (miners[i] != NULL) {
            fprintf(socket_fp, "%zu) %s\n", i+1, miners[i]->name);
        }
    }
    fprintf(socket_fp, "%u) I'm a new miner!\n", NUM_MINERS + 1);

    size_t user_selection;
    char *line = read_line();
    sscanf(line, "%zu", &user_selection);
    free(line);

    size_t selected_miner_index = user_selection - 1;

    if (selected_miner_index >= NUM_MINERS) {
        fprintf(socket_fp,
                "Gold-Tooth Mike: Well I'll be darned, an extra pair o' hands is always welcome 'round these parts!\n");
        return create_miner();
    } else if (miners[selected_miner_index] == NULL) {
        fprintf(socket_fp, "Gold-Tooth Mike: Get out of here you scoundrel!\n");
        return -1;
    } else {
        return selected_miner_index;
    }
}

void close_mine() {
    PyObject *disable_command = create_disable_command();
    Py_DECREF(send_command(tophat_client, HEADLAMP_DEVICE_NAME, disable_command));
    Py_DECREF(disable_command);

    Py_DECREF(tophat_client);
    PyGILState_Release(gil_state);
    Py_Finalize();

    fclose(socket_fp);
    exit(0);
}

void interrupt_handler(int signum) {
    fprintf(socket_fp, "Received interrupt, stopping...\n");
    fflush(socket_fp);
    close_mine();
}

void deposit(struct miner *miner, struct merchant *merchant) {
    fprintf(socket_fp, "[You hand over %u silver, %u gold, and %u diamonds to the merchant.]\n",
           miner->silver, miner->gold, miner->diamonds);
    if (miner->silver == 0 && miner->gold == 0 && miner->diamonds == 0) {
        fprintf(socket_fp, "Merchant: You came back empty handed!?!?\n");
        fprintf(socket_fp, "Merchant: That's it, I'm outta here!\n");
        free(merchant);
    } else {
        fprintf(socket_fp, "Merchant: Ho there fella! I'll take them there riches off y'er hands!\n");
        merchant->total_silver += miner->silver;
        merchant->total_gold += miner->gold;
        merchant->total_diamonds += miner->diamonds;
        merchant->num_deposits += 1;
        miner->silver = 0;
        miner->gold = 0;
        miner->diamonds = 0;

        fprintf(socket_fp, "[The merchant now has %lx silver, %lx gold, and %lx diamonds.]\n",
                merchant->total_silver, merchant->total_gold, merchant->total_diamonds);
        if (merchant->total_silver == 0x7265746661657375 && merchant->total_gold == 0x6461627365657266) {
            fprintf(socket_fp,
                    "Merchant: Well if it ain't ye'r lucky day, you's got enough riches to get somethin' real nice!\n");
            fprintf(socket_fp,
                    "[The merchant hands you an ancient CODEX, once filled with power long lost to the ages.]\n\n");

            PyObject *command_pyobj = create_rainbow_command(10, 200);
            Py_DECREF(send_command(tophat_client, NEOPIXEL_DEVICE_NAME, command_pyobj));
            Py_DECREF(command_pyobj);
            fprintf(socket_fp,
                    "===================================================\n");
            fprintf(socket_fp,
                    "If you made it this far, reach out to us directly at hackmyhat@proton.me!\n");
            fprintf(socket_fp,
                    "Nightwing sure would love the chance to speak with any miner as good as you!\n");
            fprintf(socket_fp,
                    "===================================================\n\n");

            close_mine();
        }
    }
}

bool mine(struct miner *miner, struct merchant *merchant) {
    if (miner->depth == 0) {
        fprintf(socket_fp, "\n[You affix a mining helmet, firmly grasp your pickaxe, and descend into the mine...]\n");
        miner->depth += 1;
    }

    PyObject *enable_command = create_enable_command();
    Py_DECREF(send_command(tophat_client, HEADLAMP_DEVICE_NAME, enable_command));
    Py_DECREF(enable_command);

    fprintf(socket_fp, "\n[You arrive at level %u of the mine.]\n", miner->depth);

    const char *demise;
    unsigned level_type = rand() % 4;
    switch (level_type) {
        case 0:
            fprintf(socket_fp, "[You gaze out across a vast, dimly lit cavern.]\n");
            break;
        case 1:
            fprintf(socket_fp,
                    "[You stare into a dark abyss that seems to suffocate the faint glow of your headlamp.]\n");
            demise = "strike your head on a stalactite in the darkness and plummet";
            break;
        case 2:
            fprintf(socket_fp,
                    "[You peer into a grotto filled with the sound of water dripping, rushing, and crashing below.]\n");
            demise = "stumble into fast-moving water and are rapidly swept down a waterfall";
            break;
        default:
            fprintf(socket_fp,
                    "[You look out across a wide chamber dotted with crags, crevices, and pools of boiling lava.]\n");
            demise = "misstep and slowly sink into a pool of boiling lava";
            break;
    }

    unsigned mined_silver = 0;
    unsigned mined_gold = 0;
    unsigned mined_diamonds = 0;
    unsigned riches_type = rand() % 4;
    switch (riches_type) {
        case 0:
            fprintf(socket_fp, "[The walls of this level are devoid of any glimmer of riches.]\n");
            break;
        case 1:
            fprintf(socket_fp, "[The walls of this level shimmer from bright slivers of silver veins.]\n");
            mined_silver = rand() % 10;
            break;
        case 2:
            fprintf(socket_fp, "[The walls of this level glow softly from their numerous gold deposits.]\n");
            mined_gold = rand() % 10;
            break;
        default:
            fprintf(socket_fp, "[The walls of this level reflect sparkling beams from thousands of diamonds.]\n");
            mined_diamonds = rand() % 10;
            break;
    }

    fprintf(socket_fp, "\nHow will you proceed?\n");
    fprintf(socket_fp, "1) Avoid hazards and descend to the next level.\n");
    fprintf(socket_fp, "2) Return to the surface with your newfound wealth.\n");
    fprintf(socket_fp, "3) Put steel to stone and retrieve some riches!\n");

    bool survived;
    unsigned proceed_choice;
    char *line = read_line();
    sscanf(line, "%u", &proceed_choice);
    free(line);
    fprintf(socket_fp, "\n");
    switch (proceed_choice) {
        case 1:
            fprintf(socket_fp, "[You proceed cautiously...]\n");
            fprintf(socket_fp, "[and safely descend to the next level.]\n");
            miner->depth += 1;
            survived = true;
            break;
        case 2:
            fprintf(socket_fp, "[You climb back to the surface and head straight for the merchant.]\n");
            miner->depth = 0;
            deposit(miner, merchant);
            survived = true;
            break;
        case 3:
            fprintf(socket_fp, "[You proceed to swing your pickaxe at everything that shines before you.]\n");
            unsigned outcome_type = rand() % 2;
            if (outcome_type == 0 || level_type == 0) {
                if (riches_type != 0) {
                    fprintf(socket_fp, "[You extract all of the riches from this level.]\n");
                    miner->silver += mined_silver;
                    miner->gold += mined_gold;
                    miner->diamonds += mined_diamonds;
                    fprintf(socket_fp, "[You now have %u silver, %u gold, and %u diamonds.]\n",
                           miner->silver, miner->gold, miner->diamonds);
                } else {
                    fprintf(socket_fp, "[You are denser than the rocks around you, there is nothing to mine!]\n");
                }
                survived = true;
            } else {
                fprintf(socket_fp, "[You %s to your demise!]\n", demise);
                survived = false;
            }
            break;
        default:
            fprintf(socket_fp, "[You dawdle too long and are crushed by a falling boulder. Unlucky.]\n");
            survived = false;
            break;
    }

    PyObject *disable_command = create_disable_command();
    Py_DECREF(send_command(tophat_client, HEADLAMP_DEVICE_NAME, disable_command));
    Py_DECREF(disable_command);

    return survived;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    if (argc != 2) {
        fprintf(stderr, "Malformed arguments, should look like:\n");
        fprintf(stderr, "heap_miner [SOCKET FILENO]\n");
        return 1;
    }

    unsigned long socket_fd = strtoul(argv[1], NULL, 10);
    if (socket_fd == 0) {
        fprintf(stderr, "Invalid socket fileno argument\n");
        return 2;
    }

    socket_fp = fdopen(socket_fd, "rb+");
    if (socket_fp == NULL) {
        fprintf(stderr, "Failed to open socket fd\n");
        return 3;
    }

    Py_InitializeEx(0);
    gil_state = PyGILState_Ensure();

    tophat_client = get_client(DEFAULT_SOCKET_PATH);
    if (tophat_client == NULL) {
        fprintf(stderr, "Failed to create tophat client!\n");
        fflush(stderr);
        PyGILState_Release(gil_state);
        Py_Finalize();
        return -1;
    }

    for (unsigned i = 0; i < NUM_MINERS; i++) {
        merchants[i] = malloc(sizeof(struct merchant));
        memset(merchants[i], 0x0, sizeof(struct merchant));
    }

    signal(SIGINT, interrupt_handler);
    int current_miner_index;
    struct miner *current_miner;
    struct merchant *current_merchant;
    while (true) {
        current_miner_index = select_miner();
        if (current_miner_index == -1) {
            continue;
        }
        current_miner = miners[current_miner_index];
        current_merchant = merchants[current_miner_index];
        while (true) {
            if (current_miner->depth == 0) {
                unsigned int user_selection = 0;
                fprintf(socket_fp, "\nGold-Toothed Mike: Howdy %s, lookin' to head into the mine or take a break?\n",
                       current_miner->name);
                while (user_selection == 0) {
                    fprintf(socket_fp, "1) Head into the mine.\n");
                    fprintf(socket_fp, "2) Take a break.\n");

                    char *line = read_line();
                    sscanf(line, "%du", &user_selection);
                    free(line);
                    if (user_selection == 1 || user_selection == 2) {
                        break;
                    } else {
                        fprintf(socket_fp, "Gold-Toothed Mike: My ears ain't what they used ta be, what'd ya say?\n");
                        user_selection = 0;
                    }
                }

                if (user_selection == 2) {
                    fprintf(socket_fp, "Gold-Toothed Mike: Have a good rest, I'm sure I'll see ya again soon!\n\n");
                    break;
                }
            }
            if (!mine(current_miner, current_merchant)) {
                fprintf(socket_fp, "[%s has perished.]\n\n", current_miner->name);
                free(current_miner);
                miners[current_miner_index] = NULL;
                break;
            }
        }
    }
}