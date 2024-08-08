#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#define NUM_MINERS 32

bool stop = false;

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

int create_miner() {
    struct miner *new_miner = malloc(sizeof(struct miner));
    new_miner->silver = 0;
    new_miner->gold = 0;
    new_miner->diamonds = 0;

    printf("Gold-Tooth Mike: Now, what'd you say y'er name was again?\n");
    fflush(stdout);
    fgets(new_miner->name, sizeof(new_miner->name), stdin);
    new_miner->name[strcspn(new_miner->name, "\n")] = 0;

    int new_miner_index = miner_index;
    miners[new_miner_index] = new_miner;
    miner_index = (miner_index + 1) % NUM_MINERS;
    return new_miner_index;
}

int select_miner() {
    printf("Gold-Tooth Mike: Ho, you there! You certainly seem a familiar fella...\n");
    printf("Gold-Tooth Mike: S'pose my memry ain't what it used to be though, which miner are you?\n");
    for (size_t i = 0; i < NUM_MINERS; i++) {
        if (miners[i] != NULL) {
            printf("%zu) %s\n", i+1, miners[i]->name);
        }
    }
    printf("%u) I'm a new miner!\n", NUM_MINERS + 1);

    size_t user_selection;
    scanf("%zu", &user_selection);
    fgetc(stdin);

    size_t selected_miner_index = user_selection - 1;

    if (selected_miner_index >= NUM_MINERS) {
        printf("Gold-Tooth Mike: Well I'll be darned, an extra pair o' hands is always welcome 'round these parts!\n");
        return create_miner();
    } else if (miners[selected_miner_index] == NULL) {
        printf("Gold-Tooth Mike: Get out of here you scoundrel!\n");
        return -1;
    } else {
        return selected_miner_index;
    }
}

void deposit(struct miner *miner, struct merchant *merchant) {
    printf("[You hand over %u silver, %u gold, and %u diamonds to the merchant.]\n",
           miner->silver, miner->gold, miner->diamonds);
    if (miner->silver == 0 && miner->gold == 0 && miner->diamonds == 0) {
        printf("Merchant: You came back empty handed!?!?\n");
        printf("Merchant: That's it, I'm outta here!\n");
        free(merchant);
    } else {
        printf("Merchant: Ho there fella! I'll take them there riches off y'er hands!\n");
        merchant->total_silver += miner->silver;
        merchant->total_gold += miner->gold;
        merchant->total_diamonds += miner->diamonds;
        merchant->num_deposits += 1;
        miner->silver = 0;
        miner->gold = 0;
        miner->diamonds = 0;

        printf("[The merchant now has %lx silver, %lx gold, and %lx diamonds.]\n",
               merchant->total_silver, merchant->total_gold, merchant->total_diamonds);
        if (merchant->total_silver == 0x7265746661657375 && merchant->total_gold == 0x6461627365657266) {
            printf("Merchant: Well if it ain't ye'r lucky day, you's got enough riches to get somethin' real nice!\n");
            printf("[The merchant hands you an ancient CODEX, once filled with power long lost to the ages.]\n");
            printf("===================================================\n");
            printf("If you made it this far, reach out to us directly at hackmyhat@proton.me!\n")
            printf("===================================================\n");
            // TODO: Do cool stuff!

            merchant->total_silver = 0;
            merchant->total_gold = 0;
        }
    }
}

bool mine(struct miner *miner, struct merchant *merchant) {
    if (miner->depth == 0) {
        printf("\n[You affix a mining helmet, firmly grasp your pickaxe, and descend into the mine...]\n");
        miner->depth += 1;
    }

    printf("\n[You arrive at level %u of the mine.]\n", miner->depth);

    const char *demise;
    unsigned level_type = rand() % 4;
    switch (level_type) {
        case 0:
            printf("[You gaze out across a vast, dimly lit cavern.]\n");
            break;
        case 1:
            printf("[You stare into a dark abyss that seems to suffocate the faint glow of your headlamp.]\n");
            demise = "strike your head on a stalactite in the darkness and plummet";
            break;
        case 2:
            printf("[You peer into a grotto filled with the sound of water dripping, rushing, and crashing below.]\n");
            demise = "stumble into fast-moving water and are rapidly swept down a waterfall";
            break;
        default:
            printf("[You look out across a wide chamber dotted with crags, crevices, and pools of boiling lava.]\n");
            demise = "misstep and slowly sink into a pool of boiling lava";
            break;
    }

    unsigned mined_silver = 0;
    unsigned mined_gold = 0;
    unsigned mined_diamonds = 0;
    unsigned riches_type = rand() % 4;
    switch (riches_type) {
        case 0:
            printf("[The walls of this level are devoid of any glimmer of riches.]\n");
            break;
        case 1:
            printf("[The walls of this level shimmer from bright slivers of silver veins.]\n");
            mined_silver = rand() % 10;
            break;
        case 2:
            printf("[The walls of this level glow softly from their numerous gold deposits.]\n");
            mined_gold = rand() % 10;
            break;
        default:
            printf("[The walls of this level reflect sparkling beams from thousands of diamonds.]\n");
            mined_diamonds = rand() % 10;
            break;
    }

    printf("How will you proceed?\n");
    printf("1) Avoid hazards and descend to the next level.\n");
    printf("2) Return to the surface with your newfound wealth.\n");
    printf("3) Put steel to stone and retrieve some riches!\n");

    unsigned proceed_choice;
    scanf("%u", &proceed_choice);
    fgetc(stdin);
    switch (proceed_choice) {
        case 1:
            printf("[You proceed cautiously...]\n");
            printf("[and safely descend to the next level.]\n");
            miner->depth += 1;
            return true;
        case 2:
            printf("[You climb back to the surface and head straight for the merchant.]\n");
            miner->depth = 0;
            deposit(miner, merchant);
            return true;
        case 3:
            printf("[You proceed to swing your pickaxe at everything that shines before you.]\n");
            unsigned outcome_type = rand() % 2;
            if (outcome_type == 0 || level_type == 0) {
                if (riches_type != 0) {
                    printf("[You extract all of the riches from this level.]\n");
                    miner->silver += mined_silver;
                    miner->gold += mined_gold;
                    miner->diamonds += mined_diamonds;
                    printf("[You now have %u silver, %u gold, and %u diamonds.]\n",
                           miner->silver, miner->gold, miner->diamonds);
                } else {
                    printf("[You are denser than the rocks around you, there is nothing to mine!]\n");
                }
                return true;
            } else {
                printf("[You %s to your demise!]\n", demise);
                return false;
            }
        default:
            printf("[You dawdle too long and are crushed by a falling boulder. Unlucky.]\n");
            return false;
    }
}

int main() {
    srand(time(NULL));

    for (unsigned i = 0; i < NUM_MINERS; i++) {
        merchants[i] = malloc(sizeof(struct merchant));
        memset(merchants[i], 0, sizeof(struct merchant));
    }

    int current_miner_index;
    struct miner *current_miner;
    struct merchant *current_merchant;
    while (!stop) {
        current_miner_index = select_miner();
        if (current_miner_index == -1) {
            continue;
        }
        current_miner = miners[current_miner_index];
        current_merchant = merchants[current_miner_index];
        while (true) {
            if (current_miner->depth == 0) {
                unsigned int user_selection = 0;
                printf("\nGold-Toothed Mike: Howdy %s, lookin' to head into the mine or take a break?\n",
                       current_miner->name);
                while (user_selection == 0) {
                    printf("1) Head into the mine.\n");
                    printf("2) Take a break.\n");

                    scanf("%du", &user_selection);
                    fgetc(stdin);
                    if (user_selection == 1 || user_selection == 2) {
                        break;
                    } else {
                        printf("Gold-Toothed Mike: My ears ain't what they used ta be, what'd ya say?\n");
                        user_selection = 0;
                    }
                }

                if (user_selection == 2) {
                    printf("Gold-Toothed Mike: Have a good rest, I'm sure I'll see ya again soon!\n\n");
                    break;
                }
            }
            if (!mine(current_miner, current_merchant)) {
                printf("[%s has perished.]\n\n", current_miner->name);
                free(current_miner);
                miners[current_miner_index] = NULL;
                break;
            }
        }
    }
}